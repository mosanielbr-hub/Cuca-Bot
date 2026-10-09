#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "BluetoothA2DPSource.h"
#include "audio.h"

// --- CONFIGURAÇÕES WI-FI & OTA ---
const char *ssid = "HADASSA";
const char *password = "Hadassa123#";
bool wifiConectado = false;

// --- CONFIGURAÇÕES BLUETOOTH ---
const char *nomeCaixinhaBT = "JBL Go 4 de Adiane";
BluetoothA2DPSource a2dp_source;
bool conectouBT = false;
bool tocarAudio = false;

// --- ESTRUTURA PARA CONTROLAR OS TEMPOS EM SEGUNDOS ---
struct TrechoFala {
    float tempoInicioSeg; // Tempo em segundos de início
    float tempoFimSeg;    // Tempo em segundos de término
};

// =========================================================================
// CONFIGURAÇÃO DOS TEMPOS DE CADA FALA (Altere os segundos aqui!)
// Exemplo: {inicio, fim} -> {0.0, 2.5} toca de 0s até 2,5s
// =========================================================================
TrechoFala listaTrechos[15] = {
    { 0.0,  8.0 },    // Fala 1 
    { 8.0,  17.0 },   // Fala 2  
    { 17.0, 22.0 },   // Fala 3 
    { 22.0, 25.0 },   // Fala 4 
    { 25.0, 30.0 },   // Fala 5
    { 29.5, 32.0 },   // Fala 6
    { 32.0, 35.0 },   // Fala 7
    { 35.0, 39.0 },   // Fala 8
    { 39.0, 42.0 },   // Fala 9
    { 42.0, 46.0 },   // Fala 10
    { 47.0, 49.0 },   // Fala 11
    { 50.0, 51.0 },   // Fala 12
    { 51.0, 54.0 },   // Fala 13
    { 54.0, 56.0 }    // Fala 14
};

// Váriáveis de controle interno
const uint32_t TAXA_AMOSTRAGEM = 8000; // 8000 Hz (8000 bytes por segundo)
uint32_t limiteFimByte = 0;
static float acumuladorAmostra = 0.0;

void status_bluetooth(esp_a2d_connection_state_t state, void *ptr) {
    if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
        Serial.println("\n[✓] Bluetooth Conectado na Caixinha!");
        conectouBT = true;
    } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
        Serial.println("\n[X] Bluetooth Desconectado.");
        conectouBT = false;
        tocarAudio = false;
    }
}

int32_t get_sound_data(Frame *frame, int32_t frame_count) {
    size_t bytes_para_enviar = frame_count * sizeof(Frame);
    uint8_t *buffer_dest = (uint8_t*)frame;

    if (!conectouBT || !tocarAudio) {
        memset(frame, 0, bytes_para_enviar);
        return frame_count;
    }

    const float passo = (float)TAXA_AMOSTRAGEM / 44100.0; 

    for (size_t i = 0; i < bytes_para_enviar; i += 4) {
        uint32_t indiceAtual = (uint32_t)acumuladorAmostra;

        // Toca apenas até o limite calculado do final da fala
        if (indiceAtual < limiteFimByte) {
            uint8_t amostra8bit = pgm_read_byte(&audios_unificados[indiceAtual]);
            int16_t amostra16bit = ((int16_t)amostra8bit - 128) << 8;

            uint8_t lowByte  = amostra16bit & 0xFF;
            uint8_t highByte = (amostra16bit >> 8) & 0xFF;

            buffer_dest[i]     = lowByte;
            buffer_dest[i + 1] = highByte;
            buffer_dest[i + 2] = lowByte;
            buffer_dest[i + 3] = highByte;

            acumuladorAmostra += passo;
        } else {
            // Tempo da minutagem encerrado
            memset(&buffer_dest[i], 0, bytes_para_enviar - i);
            tocarAudio = false;

            Serial.println("[⏹] Minutagem concluída. Enviando 'FIM' para o Mestre...");
            Serial2.println("FIM");
            break;
        }
    }

    return frame_count;
}

void setupOTA() {
    ArduinoOTA.setHostname("CUCA-BOT-ESCRAVO");
    ArduinoOTA.begin();
}

void setup() {
    Serial.begin(115200);
    // Comunicação Serial2 com ESP32 Mestre (RX16 / TX17)
    Serial2.begin(115200, SERIAL_8N1, 16, 17);

    // --- CONEXÃO WI-FI PARA OTA ---
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    unsigned long tInicio = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - tInicio < 5000) {
        delay(200);
    }

    if (WiFi.status() == WL_CONNECTED) {
        wifiConectado = true;
        setupOTA();
        Serial.println("[Wi-Fi] Conectado! OTA Ativo.");
    } else {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        Serial.println("[Wi-Fi] Modo offline.");
    }

    // --- CONEXÃO BLUETOOTH ---
    Serial.println("\n[ESCRAVO ÁUDIO] Inicializando Bluetooth...");
    a2dp_source.set_volume(128);
    a2dp_source.set_on_connection_state_changed(status_bluetooth);
    a2dp_source.start(nomeCaixinhaBT, get_sound_data);
}

void loop() {
    if (wifiConectado && WiFi.status() == WL_CONNECTED) {
        ArduinoOTA.handle();
    }

    // Processa o comando vindo do Mestre (ex: "TOCAR:1" até "TOCAR:14")
    if (Serial2.available()) {
        String comando = Serial2.readStringUntil('\n');
        comando.trim();

        if (comando.startsWith("TOCAR:")) {
            int numFala = comando.substring(6).toInt();

            if (numFala >= 1 && numFala <= 14) {
                if (!conectouBT) {
                    Serial.println("[!] Caixa Bluetooth desconectada!");
                    Serial2.println("FIM"); // Notifica o mestre para não travar
                    return;
                }

                int idx = numFala - 1;

                // Converte a minutagem em segundos para os bytes equivalentes (8000 Hz)
                acumuladorAmostra = listaTrechos[idx].tempoInicioSeg * TAXA_AMOSTRAGEM;
                limiteFimByte     = (uint32_t)(listaTrechos[idx].tempoFimSeg * TAXA_AMOSTRAGEM);

                tocarAudio = true;
                Serial.printf("[►] Tocando Fala %d (Tempo: %.2fs até %.2fs)\n", 
                              numFala, listaTrechos[idx].tempoInicioSeg, listaTrechos[idx].tempoFimSeg);
            }
        }
    }
    delay(10);
}