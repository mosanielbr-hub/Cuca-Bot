#include <Arduino.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Stepper.h>

// --- CONFIGURAÇÕES WI-FI & OTA ---
const char *ssid = "HADASSA";
const char *password = "Hadassa123#";
bool wifiConectado = false;

// --- PINOS SENSOR DE COR (TCS230/320) ---
const int pinoS0 = 25;
const int pinoS1 = 26;
const int pinoS2 = 27;
const int pinoS3 = 14;
const int pinoOUT = 35;

// --- MOTOR DE PASSO (28BYJ-48 + ULN2003) ---
const int passosPorVolta = 2048;
Stepper meuMotorPasso(passosPorVolta, 13, 4, 12, 5);

// Reduzido para passos curtos: sincroniza na hora e não "estoura" o tempo após o som parar
const int passosMovimento = 100; 

// --- COMUNICAÇÃO SERIAL (UART1) ---
HardwareSerial SerialAudio(1);

// --- CONTROLE DAS 14 FALAS ---
const int TOTAL_FALAS = 14;
int indiceAudioAtual = 1;

int lerCor(bool s2State, bool s3State) {
  digitalWrite(pinoS2, s2State);
  digitalWrite(pinoS3, s3State);
  return pulseIn(pinoOUT, LOW);
}

void desligarMotor() {
  digitalWrite(13, LOW);
  digitalWrite(4, LOW);
  digitalWrite(12, LOW);
  digitalWrite(5, LOW);
}

void setupOTA() {
  ArduinoOTA.setHostname("CUCA-BOT-MESTRE");
  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);

  pinMode(pinoS0, OUTPUT);
  pinMode(pinoS1, OUTPUT);
  pinMode(pinoS2, OUTPUT);
  pinMode(pinoS3, OUTPUT);
  pinMode(pinoOUT, INPUT);

  digitalWrite(pinoS0, HIGH);
  digitalWrite(pinoS1, LOW);

  SerialAudio.begin(115200, SERIAL_8N1, 18, 19);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long tInicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - tInicio < 5000) {
    delay(200);
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConectado = true;
    setupOTA();
    Serial.println("[Wi-Fi Mestre] Conectado! OTA Ativo.");
  } else {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("[Wi-Fi Mestre] Modo offline.");
  }

  // Aumentada a velocidade para o motor responder instantaneamente
  meuMotorPasso.setSpeed(17); 
  
  // Posição inicial limpa
  desligarMotor();

  Serial.println("[MESTRE] Sistema pronto! Aguardando gatilho verde...");
}

void loop() {
  if (wifiConectado && WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
  }

  int vVermelho = lerCor(LOW, LOW);
  int vVerde    = lerCor(HIGH, HIGH);
  int vAzul     = lerCor(LOW, HIGH);

  bool vendoVerde = (vVerde < vVermelho) && (vVerde < vAzul) && (vVerde < 200);

  if (vendoVerde) {
    // 1. Envia o comando de fala ao escravo
    String comando = "TOCAR:" + String(indiceAudioAtual);
    SerialAudio.println(comando);
    Serial.printf("[MESTRE] Gatilho ativado! Enviado: %s\n", comando.c_str());

    bool tocando = true;
    int sentido = 1;

    // Limpa a serial de dados residuais
    while (SerialAudio.available() > 0) SerialAudio.read();

    // 2. Loop de movimento durante o áudio
    while (tocando) {
      if (wifiConectado && WiFi.status() == WL_CONNECTED) {
        ArduinoOTA.handle();
      }

      // Dá passos curtos (100 passos) para poder checar a Serial constantemente
      meuMotorPasso.step(passosMovimento * sentido);
      sentido *= -1; 

      if (SerialAudio.available() > 0) {
        String resposta = SerialAudio.readStringUntil('\n');
        resposta.trim();
        if (resposta == "FIM") {
          tocando = false;
          Serial.printf("[MESTRE] Fala %d finalizada pelo Escravo.\n", indiceAudioAtual);
        }
      }
    }

    // 3. AVISO VISUAL: Executa NO MÁXIMOA 1 ciclo extra (vai e vem) para avisar que a fala acabou
    meuMotorPasso.step(passosMovimento);
    meuMotorPasso.step(-passosMovimento);

    // 4. Desliga as bobinas do motor imediatamente após a confirmação
    desligarMotor();

    // 5. TRAVA FINAL APÓS A FALA 14
    if (indiceAudioAtual >= TOTAL_FALAS) {
      Serial.println("[MESTRE] Todas as 14 falas foram executadas. Sistema finalizado!");
      while (true) {
        if (wifiConectado && WiFi.status() == WL_CONNECTED) {
          ArduinoOTA.handle();
        }
        delay(100);
      }
    }

    // Incrementa o índice para a próxima fala
    indiceAudioAtual++;

    // 6. Aguarda retirar o objeto verde da frente do sensor
    Serial.println("[MESTRE] Aguardando liberação do sensor...");
    while (true) {
      if (wifiConectado && WiFi.status() == WL_CONNECTED) {
        ArduinoOTA.handle();
      }
      int checkG = lerCor(HIGH, HIGH);
      int checkR = lerCor(LOW, LOW);
      int checkB = lerCor(LOW, HIGH);
      bool aindaVerde = (checkG < checkR) && (checkG < checkB) && (checkG < 200);
      if (!aindaVerde) break;
      delay(100);
    }
    Serial.println("[MESTRE] Sensor liberado. Pronto para o próximo gatilho!");
  }

  delay(50);
}