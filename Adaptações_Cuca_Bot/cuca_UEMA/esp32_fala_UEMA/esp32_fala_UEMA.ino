#include <Arduino.h>
#include "BluetoothA2DPSource.h"
#include "audio.h"

// --- CONFIGURAÇÕES BLUETOOTH ---
const char *nomeCaixinhaBT = "KA-790";
BluetoothA2DPSource a2dp_source;

volatile bool conectouBT = false;
volatile bool tocarAudio = false;
volatile bool flagNotificarFim = false;

// --- ESTRUTURA PARA CONTROLAR OS TEMPOS EM SEGUNDOS ---
struct TrechoFala {
    float tempoInicioSeg; // Tempo em segundos de início
    float tempoFimSeg;    // Tempo em segundos de término
};

// =========================================================================
// CONFIGURAÇÃO DOS TEMPOS DAS 7 FALAS (SINCRONIZADO COM O ÁUDIO)
// =========================================================================
const int TOTAL_FALAS = 7;

TrechoFala listaTrechos[TOTAL_FALAS] = {
    { 0.0,   5.5 },  // Fala 1: "Então vocês vieram..."
    { 5.5,   10.5 },  // Fala 2: "Vocês acham que..."
    { 10.5,   17.5 },  // Fala 3: "Cuidado! Porque por trás..."
    { 17.5,  22.5 },  // Fala 4: "A tecnologia está crescendo..."
    { 22.5,  26.2 },  // Fala 5: "...os robôs estão ficando..."
    { 26.0,  32.0 },  // Fala 6: "...e talvez, um dia..."
    { 32.0,  37.5 }   // Fala 7: "Agora, quero ver se..."
};

// --- CONTROLE DE AMOSTRAGEM (16-bit Mono = 16.000 bytes por segundo) ---
const uint32_t TAXA_AMOSTRAGEM = 8000; 
const uint32_t BYTES_POR_SEGUNDO = TAXA_AMOSTRAGEM * 2; 

volatile uint32_t limiteFimByte = 0;
volatile float acumuladorAmostra = 0.0;

// Callback de status do Bluetooth
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

// Callback de streaming de áudio (Executado em alta prioridade pela pilha BT)
int32_t get_sound_data(Frame *frame, int32_t frame_count) {
    size_t bytes_para_enviar = frame_count * sizeof(Frame);
    uint8_t *buffer_dest = (uint8_t*)frame;

    // Se estiver desconectado ou sem sinal para tocar, envia silêncio rápido
    if (!conectouBT || !tocarAudio) {
        memset(frame, 0, bytes_para_enviar);
        return frame_count;
    }

    const float passo = (float)TAXA_AMOSTRAGEM / 44100.0f; 

    for (size_t i = 0; i < bytes_para_enviar; i += 4) {
        uint32_t indiceAmostra = (uint32_t)acumuladorAmostra;
        uint32_t indiceByte = indiceAmostra * 2;

        if (indiceByte < limiteFimByte) {
            uint8_t lowByte  = pgm_read_byte(&audios_unificados[indiceByte]);
            uint8_t highByte = pgm_read_byte(&audios_unificados[indiceByte + 1]);

            // Canal Esquerdo e Direito
            buffer_dest[i]     = lowByte;  
            buffer_dest[i + 1] = highByte; 
            buffer_dest[i + 2] = lowByte;  
            buffer_dest[i + 3] = highByte; 

            acumuladorAmostra += passo;
        } else {
            // Fim da minutagem da fala
            memset(&buffer_dest[i], 0, bytes_para_enviar - i);
            tocarAudio = false;
            flagNotificarFim = true; // Sinaliza para enviar a resposta pela Serial no loop()
            break;
        }
    }

    return frame_count;
}

void setup() {
    Serial.begin(115200);
    
    // Comunicação Serial2 com ESP32 Mestre (RX: GPIO 16 / TX: GPIO 17)
    Serial2.begin(115200, SERIAL_8N1, 16, 17);

    Serial.println("\n[ESCRAVO ÁUDIO] Inicializando Bluetooth...");
    a2dp_source.set_volume(128);
    a2dp_source.set_on_connection_state_changed(status_bluetooth);
    a2dp_source.start(nomeCaixinhaBT, get_sound_data);
}

void loop() {
    // Processa a notificação "FIM" fora do callback de áudio para evitar crash do Bluetooth
    if (flagNotificarFim) {
        flagNotificarFim = false;
        Serial.println("[⏹] Minutagem concluída. Enviando 'FIM' para o Mestre...");
        Serial2.println("FIM");
    }

    // Processa o comando vindo do Mestre (ex: "TOCAR:1" até "TOCAR:7")
    if (Serial2.available()) {
        String comando = Serial2.readStringUntil('\n');
        comando.trim();

        if (comando.startsWith("TOCAR:")) {
            int numFala = comando.substring(6).toInt();

            if (numFala >= 1 && numFala <= TOTAL_FALAS) {
                if (!conectouBT) {
                    Serial.println("[!] Caixa Bluetooth desconectada!");
                    Serial2.println("FIM"); // Avisa o Mestre para não travar
                    return;
                }

                int idx = numFala - 1;

                // Configura os ponteiros de amostragem
                tocarAudio = false;
                acumuladorAmostra = listaTrechos[idx].tempoInicioSeg * TAXA_AMOSTRAGEM;
                limiteFimByte     = (uint32_t)(listaTrechos[idx].tempoFimSeg * BYTES_POR_SEGUNDO);
                tocarAudio = true;

                Serial.printf("[►] Tocando Fala %d de %d (Tempo: %.2fs até %.2fs)\n", 
                              numFala, TOTAL_FALAS, listaTrechos[idx].tempoInicioSeg, listaTrechos[idx].tempoFimSeg);
            }
        }
    }
    delay(10);
}