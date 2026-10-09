# 🤖 Cuca Bot - Dual ESP32 Interactive System

Projeto robótico interativo baseado em uma arquitetura Mestre-Escravo com dois microcontroladores **ESP32**, utilizando comunicação serial UART, detecção de cor por sensor óptico, controle de motor de passo e transmissão de áudio via Bluetooth A2DP.

---

## 📌 Arquitetura do Sistema

O sistema é dividido em dois blocos de processamento independentes para garantir alta performance e evitar atrasos na reprodução de áudio:

- **ESP32 Mestre:**
  - Leitura do sensor de cor (TCS230 / TCS320).
  - Controle do motor de passo (28BYJ-48 com driver ULN2003).
  - Envio de comandos de sincronização via UART2 (`TOCAR:X`).
  - Gestão de atualizações remotas via Wi-Fi (ArduinoOTA).

- **ESP32 Escravo:**
  - Recepção de comandos via UART2.
  - Leitura do buffer de áudio na memória Flash (PROGMEM).
  - Streaming de áudio estéreo em tempo real via Bluetooth A2DP para caixas de som externas.
  - Envio de sinal de conclusão (`FIM`) para o Mestre.

---

## 🔌 Pinagem e Conexões

### 1. ESP32 Mestre
- **Sensor de Cor TCS230/320:**
  - `S0` -> GPIO 25
  - `S1` -> GPIO 26
  - `S2` -> GPIO 27
  - `S3` -> GPIO 14
  - `OUT` -> GPIO 35
- **Driver de Motor ULN2003:**
  - `IN1` -> GPIO 13
  - `IN2` -> GPIO 4
  - `IN3` -> GPIO 12
  - `IN4` -> GPIO 5

### 2. Comunicação UART (Entre ESP32 Mestre e Escravo)

| ESP32 Mestre | Direção | ESP32 Escravo |
| :--- | :---: | :--- |
| **GPIO 17 (TX2)** | $\rightarrow$ | **GPIO 16 (RX2)** |
| **GPIO 16 (RX2)** | $\leftarrow$ | **GPIO 17 (TX2)** |
| **GND** | $\leftrightarrow$ | **GND** |

---

## 🛠️ Requisitos de Hardware e Alimentação

1. **Alimentação:**
   - Recomenda-se utilizar uma fonte externa de **5V / 2A** para alimentar o driver do motor de passo e o ESP32 Escravo.
   - Adicionar um capacitor eletrolítico (470µF a 1000µF) nos pinos de alimentação do ESP32 Escravo para mitigar picos de consumo do rádio Bluetooth e evitar *brownouts*.

2. **Dispositivo de Áudio:**
   - O firmware é compatível com caixas de som Bluetooth padrão (A2DP Sink).
   - *Nota:* Fones de ouvido TWS (como G9S) podem não manter a conexão devido às exigências de gestão de energia do perfil A2DP Source.

---

## 💻 Bibliotecas Utilizadas

- [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) (A2DP Source para transmissão de áudio)
- `Stepper` (Controle de motores de passo)
- `ArduinoOTA` / `WiFi` (Atualização de firmware via rede)

---

## 👥 Autores

Desenvolvido por:

- **[Ana Beatriz]** - [@anabeatriz26920](https://github.com/anabeatriz26920)
- **[Mosaniel]** - [@mosanielbr-hub](https://github.com/mosanielbr-hub)

---

## 📄 Licença

Este projeto está licenciado sob a Licença MIT - consulte o arquivo [LICENSE](LICENSE) para mais detalhes.
