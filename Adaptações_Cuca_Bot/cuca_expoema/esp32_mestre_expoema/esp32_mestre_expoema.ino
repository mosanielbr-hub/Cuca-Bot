#include <Arduino.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Stepper.h>

// --- CONFIGURAÇÕES WI-FI & OTA ---
const char *ssid = "HADASSA";
const char *password = "Hadassa123#";
bool wifiConectado = false;

// --- PINOS SENSOR ULTRASSÓNICO (HC-SR04) ---
const int trigPin = 5;
const int echoPin = 18;

// --- MOTOR DE PASSO (28BYJ-48 + ULN2003) ---
const int passosPorVolta = 2048;
Stepper meuMotorPasso(passosPorVolta, 13, 4, 12, 5);

// Passos curtos por ciclo para manter a resposta rápida
const int passosMovimento = 100; 

// --- COMUNICAÇÃO SERIAL COM O ESCRAVO DE ÁUDIO ---
HardwareSerial SerialAudio(2); // Utilizando a UART2 nativa do ESP32

// --- CONTROLE DAS FALAS ---
const int TOTAL_FALAS = 7; 
int indiceAudioAtual = 1;

// Função para medir a distância com o sensor ultrassónico (retorna em centímetros)
int lerDistanciaCm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000); // Timeout de 30ms para evitar travamentos
  if (duration == 0) return 999; // Se não houver retorno, retorna uma distância grande
  
  return duration * 0.034 / 2;
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

  // Configuração dos pinos do Ultrassónico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Configuração da UART2 (RX=16, TX=17)
  SerialAudio.begin(115200, SERIAL_8N1, 16, 17);

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

  meuMotorPasso.setSpeed(17); 
  desligarMotor();

  Serial.println("[MESTRE] Sistema pronto! Aguardando aproximação no ultrassónico...");
}

void loop() {
  if (wifiConectado && WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
  }

  // Lê a distância atual do sensor ultrassónico
  int distancia = lerDistanciaCm();

  // Condição de gatilho: objeto/pessoa a menos de 5 cm
  bool objetoProximo = (distancia > 0 && distancia < 5);

  if (objetoProximo) {
    // 1. Envia o comando de fala ao escravo
    String comando = "TOCAR:" + String(indiceAudioAtual);
    SerialAudio.println(comando);
    Serial.printf("[MESTRE] Gatilho ativado! Distância: %d cm | Enviado: %s\n", distancia, comando.c_str());

    bool tocando = true;
    int sentido = 1;

    // Limpa a serial de dados residuais antes da leitura
    while (SerialAudio.available() > 0) SerialAudio.read();

    // 2. Loop de movimento do motor enquanto o áudio é executado
    while (tocando) {
      if (wifiConectado && WiFi.status() == WL_CONNECTED) {
        ArduinoOTA.handle();
      }

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

    // 3. Finalização do movimento do motor
    meuMotorPasso.step(passosMovimento);
    meuMotorPasso.step(-passosMovimento);
    desligarMotor();

    // 4. LÓGICA DE LOOPING AUTOMÁTICO DAS FALAS (1 ao 7 e recomeça)
    if (indiceAudioAtual >= TOTAL_FALAS) {
      Serial.println("[MESTRE] Última fala executada. Reiniciando sequência para a Fala 1 em loop.");
      indiceAudioAtual = 1; // Volta para o início do ciclo
    } else {
      indiceAudioAtual++;  // Avança para a próxima fala
    }

    // 5. Aguarda a remoção do objeto/pessoa da frente do sensor antes de permitir novo gatilho
    Serial.println("[MESTRE] Aguardando liberação do sensor ultrassónico...");
    while (true) {
      if (wifiConectado && WiFi.status() == WL_CONNECTED) {
        ArduinoOTA.handle();
      }
      int checkDist = lerDistanciaCm();
      if (checkDist >= 25) { // Só sai quando a pessoa se afastar (mais de 25 cm)
        break;
      }
      delay(100);
    }
    Serial.println("[MESTRE] Sensor liberado. Pronto para o próximo acionamento!");
  }

  delay(50);
}