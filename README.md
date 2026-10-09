# 🤖 Cuca Bot — Dual ESP32 Interactive System

Projeto robótico interativo baseado em uma arquitetura **Mestre–Escravo**, utilizando dois microcontroladores **ESP32**, comunicação serial UART, detecção de cores por sensor óptico, controle de motor de passo e transmissão de áudio via Bluetooth A2DP.

---

## 📌 Arquitetura do Sistema

O sistema é dividido em dois blocos de processamento independentes para distribuir as tarefas e minimizar possíveis atrasos na reprodução de áudio.

### 🧠 ESP32 Mestre

Responsável pelo processamento dos sensores e controle dos movimentos do robô.

- Leitura do sensor de cor **TCS230/TCS3200** ou sensor ultrassônico, dependendo da versão.
- Controle do motor de passo **28BYJ-48**, utilizando o driver **ULN2003**.
- Envio de comandos de sincronização via UART2 (`TOCAR:X`).
- Gerenciamento de atualizações remotas via Wi-Fi (**ArduinoOTA**).

### 🔊 ESP32 Escravo

Responsável pelo processamento e transmissão do áudio.

- Recepção de comandos via UART2.
- Leitura do buffer de áudio armazenado na memória Flash (`PROGMEM`).
- Transmissão de áudio estéreo em tempo real via **Bluetooth A2DP** para caixas de som externas.
- Envio do sinal de conclusão (`FIM`) para o ESP32 Mestre.

---

## 🔌 Pinagem e Conexões

### 1. ESP32 Mestre

**Sensor de Cor — TCS230/TCS3200**

| Pino do sensor | GPIO do ESP32 |
|---|---|
| S0 | GPIO 25 |
| S1 | GPIO 26 |
| S2 | GPIO 27 |
| S3 | GPIO 14 |
| OUT | GPIO 35 |

**Driver do Motor de Passo — ULN2003**

| Pino do driver | GPIO do ESP32 |
|---|---|
| IN1 | GPIO 13 |
| IN2 | GPIO 4 |
| IN3 | GPIO 12 |
| IN4 | GPIO 5 |

### 2. Comunicação UART — Mestre e Escravo

A comunicação entre os dois ESP32 utiliza a interface serial UART2.

| ESP32 Mestre | Direção | ESP32 Escravo |
|---|:---:|---|
| GPIO 17 (TX2) | → | GPIO 16 (RX2) |
| GPIO 16 (RX2) | ← | GPIO 17 (TX2) |
| GND | ↔ | GND |

**Observação:** ambos os microcontroladores devem compartilhar uma referência comum de GND para garantir a comunicação serial.

---

## 🛠️ Requisitos de Hardware e Alimentação

### 1. Alimentação

- Recomenda-se utilizar uma fonte externa de **5 V / 2 A** para alimentar o driver do motor de passo e o ESP32 Escravo, desde que a capacidade da fonte seja suficiente para o consumo total do circuito.
- Adicionar um capacitor eletrolítico de **470 µF a 1000 µF** próximo à alimentação do ESP32 Escravo para ajudar a mitigar quedas momentâneas de tensão causadas pelo consumo do rádio Bluetooth e reduzir possíveis *brownouts*.
- Garantir que todos os componentes estejam conectados corretamente e recebam as tensões adequadas.

### 2. Dispositivo de Áudio

O firmware foi projetado para transmitir áudio a caixas de som Bluetooth compatíveis com o perfil **A2DP Sink**.

---

## 💻 Bibliotecas Utilizadas

| Biblioteca | Finalidade |
|---|---|
| [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) | Transmissão de áudio Bluetooth A2DP |
| `Stepper` | Controle do motor de passo |
| `ArduinoOTA` | Atualização remota do firmware |
| `WiFi` | Conectividade com a rede sem fio |

---

## 📖 Nossa Trajetória com a Cuca

O projeto **Cuca Bot** nasceu da união entre eletrônica, programação, robótica e cultura popular.

Ao longo do tempo, o robô passou por diversas etapas de desenvolvimento, testes, melhorias e conquistas que marcaram nossa trajetória.

### 🛠️ 1. Processo de Desenvolvimento e Montagem

Tudo começou na bancada de testes, com a soldagem dos componentes, ajustes eletrônicos, organização das conexões e construção da estrutura física do robô.

A integração dos dois microcontroladores ESP32 foi uma etapa fundamental para o funcionamento do sistema.

<p align="center">
  <img src="fotos/etapa(1).jpeg" width="350" alt="Primeira etapa de desenvolvimento do Cuca Bot">
  <img src="fotos/etapa%20(5).jpeg" width="350" alt="Segunda etapa de montagem do Cuca Bot">
</p>

### 🥉 2. Conquista na Olimpíada Brasileira de Robótica (OBR)

Levamos a Cuca para competir na **Olimpíada Brasileira de Robótica (OBR)** e conquistamos o **3º lugar**!

Essa conquista representou o reconhecimento do nosso esforço, dedicação e trabalho em equipe durante o desenvolvimento do projeto.

![Participação do Cuca Bot na OBR — Foto 1](fotos/obr(1).jpeg)

![Participação do Cuca Bot na OBR — Foto 2](fotos/obr(2).jpeg)

### 🎪 3. Destaque e Abertura na Expoema — Estande do IEMA

Graças ao sucesso do projeto, fomos convidados para participar da **abertura da Expoema, no estande do IEMA**.

Durante o evento, a Cuca teve a oportunidade de interagir com o público, demonstrando a aplicação prática da robótica e representando nosso trabalho.

![Cuca Bot na Expoema](fotos/expoema.jpg)

---

## 👥 Autores

Projeto desenvolvido por:

- **Ana Beatriz** — [@anabeatriz26920](https://github.com/anabeatriz26920)
- **Mosaniel** — [@mosanielbr-hub](https://github.com/mosanielbr-hub)

---

## 📄 Licença

Este projeto é proprietário e confidencial. Desenvolvido por **Ana Beatriz** e **Mosaniel**. Todos os direitos reservados. É proibida a utilização, cópia ou distribuição sem autorização expressa.
