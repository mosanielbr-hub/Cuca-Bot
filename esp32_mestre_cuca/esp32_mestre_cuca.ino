
#include <Arduino.h>
#include <Stepper.h>
#include <HardwareSerial.h>

// ===========================================
// COMUNICACAO UART COM ESP32 ESCRAVO
// ===========================================

const int UART_RX = 16;
const int UART_TX = 17;

HardwareSerial SerialAudio(2);

const unsigned long TIMEOUT_AUDIO = 20000;

// ===========================================
// SENSOR DE COR TCS230 / TCS3200
// ===========================================

const int pinoS0  = 25;
const int pinoS1  = 26;
const int pinoS2  = 18;
const int pinoS3  = 19;
const int pinoOUT = 35;

// ===========================================
// PONTE H 1 - LADO ESQUERDO
// ===========================================

const int IN1 = 13; // Tras Esquerda
const int IN2 = 12;
const int IN3 = 14; // Frente Esquerda
const int IN4 = 27;

// ===========================================
// PONTE H 2 - LADO DIREITO
// ===========================================

const int IN5 = 33; // Tras Direita
const int IN6 = 32;
const int IN7 = 21; // Frente Direita
const int IN8 = 4;

// ===========================================
// MOTOR DE PASSO 28BYJ-48 + ULN2003
// ===========================================

// Novos pinos para liberar UART 16 e 17
const int passoIN1 = 5;
const int passoIN2 = 15;
const int passoIN3 = 22;
const int passoIN4 = 23;

const int passosPorVolta = 2048;
const int passosMovimento = 100;

Stepper meuMotorPasso(
  passosPorVolta,
  passoIN1,
  passoIN2,
  passoIN3,
  passoIN4
);

void desligarMotorPasso() {
  digitalWrite(passoIN1, LOW);
  digitalWrite(passoIN2, LOW);
  digitalWrite(passoIN3, LOW);
  digitalWrite(passoIN4, LOW);
}

void movimentarMotorPasso() {
  meuMotorPasso.step(passosMovimento);
  meuMotorPasso.step(-passosMovimento);
  desligarMotorPasso();
}

// ===========================================
// CONTROLE DAS 14 FALAS
// ===========================================

const int MAX_MOVIMENTOS = 14;
int indiceFalaAtual = 1;

// ===========================================
// COMUNICACAO COM ESP32 ESCRAVO
// ===========================================

void enviarComandoAudio(int numeroFala) {

  // Limpa dados antigos da UART
  while (SerialAudio.available()) {
    SerialAudio.read();
  }

  // Envia comando TOCAR:X
  SerialAudio.print("TOCAR:");
  SerialAudio.println(numeroFala);

  Serial.printf(
    "[MESTRE] Comando enviado: TOCAR:%d\n",
    numeroFala
  );
}

bool aguardarFimAudio() {

  unsigned long inicio = millis();
  String resposta = "";

  Serial.println(
    "[MESTRE] Aguardando FIM do Escravo..."
  );

  while (millis() - inicio < TIMEOUT_AUDIO) {

    while (SerialAudio.available()) {

      char caractere = SerialAudio.read();

      if (caractere == '\n') {

        resposta.trim();

        if (resposta == "FIM") {

          Serial.println(
            "[MESTRE] Audio finalizado!"
          );

          return true;
        }

        resposta = "";

      } else {

        if (resposta.length() < 32) {
          resposta += caractere;
        }
      }
    }

    delay(10);
  }

  Serial.println(
    "[MESTRE] Timeout aguardando FIM!"
  );

  return false;
}

// ===========================================
// CONTROLE DOS 8 MOTORES DC
// ===========================================

void pararMotores() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  digitalWrite(IN5, LOW);
  digitalWrite(IN6, LOW);

  digitalWrite(IN7, LOW);
  digitalWrite(IN8, LOW);
}

void moverFrente() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  digitalWrite(IN5, HIGH);
  digitalWrite(IN6, LOW);

  digitalWrite(IN7, LOW);
  digitalWrite(IN8, HIGH);
}

void moverTras() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  digitalWrite(IN5, LOW);
  digitalWrite(IN6, HIGH);

  digitalWrite(IN7, HIGH);
  digitalWrite(IN8, LOW);
}

void girarEsquerda() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  digitalWrite(IN5, HIGH);
  digitalWrite(IN6, LOW);

  digitalWrite(IN7, LOW);
  digitalWrite(IN8, HIGH);
}

void girarDireita() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  digitalWrite(IN5, LOW);
  digitalWrite(IN6, HIGH);

  digitalWrite(IN7, HIGH);
  digitalWrite(IN8, LOW);
}

// ===========================================
// LEITURA DO SENSOR DE COR
// ===========================================

int lerCor(bool s2State, bool s3State) {

  digitalWrite(pinoS2, s2State);
  digitalWrite(pinoS3, s3State);

  return pulseIn(pinoOUT, LOW, 20000);
}

// Protecao contra falsos positivos
bool verificarVerdeReal() {

  int r = lerCor(LOW, LOW);
  int g = lerCor(HIGH, HIGH);
  int b = lerCor(LOW, HIGH);

  return (g > 15) &&
         (g < r) &&
         (g < b) &&
         (g < 200);
}

// ===========================================
// SETUP
// ===========================================

void setup() {

  Serial.begin(115200);

  // UART COM ESP32 ESCRAVO
  SerialAudio.begin(
    115200,
    SERIAL_8N1,
    UART_RX,
    UART_TX
  );

  Serial.println(
    "[UART] Comunicacao com Escravo iniciada!"
  );

  // SENSOR DE COR
  pinMode(pinoS0, OUTPUT);
  pinMode(pinoS1, OUTPUT);
  pinMode(pinoS2, OUTPUT);
  pinMode(pinoS3, OUTPUT);
  pinMode(pinoOUT, INPUT);

  digitalWrite(pinoS0, HIGH);
  digitalWrite(pinoS1, HIGH);

  // PONTE H ESQUERDA
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // PONTE H DIREITA
  pinMode(IN5, OUTPUT);
  pinMode(IN6, OUTPUT);
  pinMode(IN7, OUTPUT);
  pinMode(IN8, OUTPUT);

  pararMotores();

  // MOTOR DE PASSO
  meuMotorPasso.setSpeed(17);
  desligarMotorPasso();

  Serial.println(
    "[CUCA BOT] Sistema inicializado!"
  );

  Serial.println(
    "[BASE] Aguardando cor verde..."
  );
}

// ===========================================
// LOOP PRINCIPAL
// ===========================================

void loop() {

  // Limite de 14 falas
  if (indiceFalaAtual > MAX_MOVIMENTOS) {

    pararMotores();
    desligarMotorPasso();

    Serial.println(
      "[MESTRE] Todas as 14 falas executadas!"
    );

    while (true) {
      delay(1000);
    }
  }

  // Primeira verificacao do verde
  if (verificarVerdeReal()) {

    delay(60);

    // Segunda verificacao do verde
    if (verificarVerdeReal()) {

      Serial.printf(
        "[BASE] Verde CONFIRMADO! Acao %d/14\n",
        indiceFalaAtual
      );

      // ENVIA COMANDO DE AUDIO
      enviarComandoAudio(indiceFalaAtual);

      // ===================================
      // MOVIMENTOS ORIGINAIS DA OBR
      // ===================================

      if (indiceFalaAtual == 3) {

        unsigned long inicio = millis();

        while (millis() - inicio < 3000) {

          moverFrente();
          delay(200);

          moverTras();
          delay(200);
        }

        pararMotores();
      }

      else if (indiceFalaAtual == 4) {

        moverFrente();
        delay(3000);

        unsigned long inicioGiro = millis();

        while (millis() - inicioGiro < 3000) {

          girarEsquerda();
          delay(150);

          girarDireita();
          delay(150);
        }

        pararMotores();
      }

      else if (indiceFalaAtual == 6) {

        unsigned long inicio = millis();

        while (millis() - inicio < 4000) {

          girarEsquerda();
          delay(600);

          girarDireita();
          delay(600);
        }

        pararMotores();
      }

      else if (indiceFalaAtual == 7) {

        unsigned long inicio = millis();

        while (millis() - inicio < 4000) {

          girarEsquerda();
          delay(250);

          girarDireita();
          delay(250);
        }

        pararMotores();
      }

      else if (indiceFalaAtual == 13) {

        pararMotores();

        Serial.println(
          "[BASE] Fala 13: Parado."
        );
      }

      else {

        moverFrente();
        delay(150);

        moverTras();
        delay(150);

        pararMotores();
      }

      // ===================================
      // MOTOR DE PASSO
      // ===================================

      Serial.println(
        "[PASSO] Executando movimento..."
      );

      movimentarMotorPasso();

      Serial.println(
        "[PASSO] Movimento finalizado."
      );

      // ===================================
      // AGUARDA FINALIZACAO DO AUDIO
      // ===================================

      bool audioFinalizado = aguardarFimAudio();

      if (!audioFinalizado) {
        Serial.println(
          "[MESTRE] Prosseguindo apos timeout."
        );
      }

      // PROXIMA FALA
      indiceFalaAtual++;

      // ===================================
      // AGUARDA RETIRADA DO OBJETO VERDE
      // ===================================

      Serial.println(
        "[BASE] Aguardando retirar o verde..."
      );

      while (true) {

        if (!verificarVerdeReal()) {

          delay(60);

          if (!verificarVerdeReal()) {
            break;
          }
        }

        delay(50);
      }

      Serial.println(
        "[BASE] Sensor liberado!"
      );
    }
  }

  delay(20);
}
