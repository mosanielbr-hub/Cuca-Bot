
#include <Arduino.h>
#include <Stepper.h>

// --- PINOS DO SENSOR DE COR (TCS230/320) ---
const int pinoS0  = 25;
const int pinoS1  = 26;
const int pinoS2  = 18;
const int pinoS3  = 19;
const int pinoOUT = 35;

// --- PINOS PONTE H 1 (LADO ESQUERDO) ---
const int IN1 = 13; // Trás Esquerda
const int IN2 = 12;
const int IN3 = 14; // Frente Esquerda
const int IN4 = 27;

// --- PINOS PONTE H 2 (LADO DIREITO) ---
const int IN5 = 33; // Trás Direita
const int IN6 = 32;
const int IN7 = 21; // Frente Direita
const int IN8 = 4;

// --- MOTOR DE PASSO (28BYJ-48 + ULN2003) ---
const int passoIN1 = 16;
const int passoIN2 = 17;
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

// --- CONTROLE DOS MOVIMENTOS ---
const int MAX_MOVIMENTOS = 14;
int indiceFalaAtual = 1;

void pararMotores() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  digitalWrite(IN5, LOW); digitalWrite(IN6, LOW);
  digitalWrite(IN7, LOW); digitalWrite(IN8, LOW);
}

void moverFrente() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  digitalWrite(IN5, HIGH); digitalWrite(IN6, LOW);
  digitalWrite(IN7, LOW);  digitalWrite(IN8, HIGH);
}

void moverTras() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  digitalWrite(IN5, LOW);  digitalWrite(IN6, HIGH);
  digitalWrite(IN7, HIGH); digitalWrite(IN8, LOW);
}

void girarEsquerda() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  digitalWrite(IN5, HIGH); digitalWrite(IN6, LOW);
  digitalWrite(IN7, LOW);  digitalWrite(IN8, HIGH);
}

void girarDireita() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  digitalWrite(IN5, LOW);  digitalWrite(IN6, HIGH);
  digitalWrite(IN7, HIGH); digitalWrite(IN8, LOW);
}

int lerCor(bool s2State, bool s3State) {
  digitalWrite(pinoS2, s2State);
  digitalWrite(pinoS3, s3State);
  return pulseIn(pinoOUT, LOW, 20000);
}

// Função de verificação do verde real
bool verificarVerdeReal() {
  int r = lerCor(LOW, LOW);
  int g = lerCor(HIGH, HIGH);
  int b = lerCor(LOW, HIGH);

  return (g > 15) && (g < r) && (g < b) && (g < 200);
}

void setup() {
  Serial.begin(115200);

  // Configuração dos pinos do sensor
  pinMode(pinoS0, OUTPUT);
  pinMode(pinoS1, OUTPUT);
  pinMode(pinoS2, OUTPUT);
  pinMode(pinoS3, OUTPUT);
  pinMode(pinoOUT, INPUT);

  // Frequência do sensor em 100%
  digitalWrite(pinoS0, HIGH);
  digitalWrite(pinoS1, HIGH);

  // Configuração das pontes H
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(IN5, OUTPUT); pinMode(IN6, OUTPUT);
  pinMode(IN7, OUTPUT); pinMode(IN8, OUTPUT);

  pararMotores();

  meuMotorPasso.setSpeed(17);
  desligarMotorPasso();

  Serial.println("[BASE AUTÔNOMA] Sistema inicializado com proteção anti-falso-positivo!");
  Serial.println("[MOTOR DE PASSO] Inicializado!");
}

void loop() {
  if (indiceFalaAtual > MAX_MOVIMENTOS) {
    pararMotores();
    desligarMotorPasso();

    Serial.println("[BASE] Limite de 14 movimentos atingido. Encerrado.");

    while (true) {
      delay(1000);
    }
  }

  // Checagem 1: Lê se há verde seguro
  if (verificarVerdeReal()) {
    delay(60);

    // Checagem 2: Confirmação dupla
    if (verificarVerdeReal()) {
      Serial.printf(
        "[BASE] Verde CONFIRMADO! Executando ação %d/14\n",
        indiceFalaAtual
      );

      if (indiceFalaAtual == 3) {
        unsigned long inicio = millis();

        while (millis() - inicio < 3000) {
          moverFrente(); delay(200);
          moverTras();   delay(200);
        }

        pararMotores();
      }

      else if (indiceFalaAtual == 4) {
        moverFrente();
        delay(3000);

        unsigned long inicioGiro = millis();

        while (millis() - inicioGiro < 3000) {
          girarEsquerda(); delay(150);
          girarDireita();  delay(150);
        }

        pararMotores();
      }

      else if (indiceFalaAtual == 6) {
        unsigned long inicio = millis();

        while (millis() - inicio < 4000) {
          girarEsquerda(); delay(600);
          girarDireita();  delay(600);
        }

        pararMotores();
      }

      else if (indiceFalaAtual == 7) {
        unsigned long inicio = millis();

        while (millis() - inicio < 4000) {
          girarEsquerda(); delay(250);
          girarDireita();  delay(250);
        }

        pararMotores();
      }

      else if (indiceFalaAtual == 13) {
        pararMotores();
        Serial.println("[BASE] Fala 13: Parado.");
      }

      else {
        moverFrente(); delay(150);
        moverTras();   delay(150);
        pararMotores();
      }

      Serial.println("[PASSO] Executando movimento...");
      movimentarMotorPasso();
      Serial.println("[PASSO] Movimento finalizado.");

      indiceFalaAtual++;

      // Aguarda o objeto verde ser removido
      Serial.println("[BASE] Aguardando desobstrução da cor verde...");

      while (true) {
        if (!verificarVerdeReal()) {
          delay(60);

          if (!verificarVerdeReal()) break;
        }

        delay(50);
      }

      Serial.println("[BASE] Sensor liberado para a próxima leitura.");
    }
  }

  delay(20);
}
