# 🤖 Cuca Bot — Dual ESP32 Interactive System

Projeto robótico interativo baseado em uma arquitetura **Mestre–Escravo**, utilizando dois microcontroladores **ESP32**, comunicação serial UART, detecção de cores por sensor óptico, controle de motores e transmissão de áudio via Bluetooth A2DP.

O projeto também possui uma **versão de locomoção utilizada na Olimpíada Brasileira de Robótica (OBR)**, equipada com **8 motores DC e 2 módulos de ponte H**, permitindo a movimentação da base do robô.

---

## 📌 Arquitetura do Sistema

O sistema principal é dividido em dois blocos de processamento independentes para distribuir as tarefas e minimizar possíveis atrasos na reprodução de áudio.

### 🧠 ESP32 Mestre

Responsável pelo processamento dos sensores e controle dos movimentos do robô.

- Leitura do sensor de cor **TCS230/TCS3200** ou sensor ultrassônico, dependendo da versão.
- Controle do motor de passo **28BYJ-48**, utilizando o driver **ULN2003**.
- Controle da base motorizada com **8 motores DC e 2 pontes H**, na versão OBR.
- Envio de comandos de sincronização via UART (`TOCAR:X`), nas versões com áudio.
- Gerenciamento de atualizações remotas via Wi-Fi (**ArduinoOTA**), nas versões compatíveis.

### 🔊 ESP32 Escravo

Responsável pelo processamento e transmissão do áudio.

- Recepção de comandos via UART.
- Áudios WAV convertidos para arrays C++.
- Leitura dos dados de áudio armazenados na memória Flash (`PROGMEM`).
- Transmissão de áudio estéreo em tempo real via **Bluetooth A2DP** para caixas de som externas.
- Envio do sinal de conclusão (`FIM`) para o ESP32 Mestre.

### 🚗 Sistema de Locomoção — Versão OBR

A versão utilizada na **Olimpíada Brasileira de Robótica (OBR)** possui uma base motorizada composta por **8 motores DC**, acionados por **2 módulos de ponte H**.

Essa configuração permite que a Cuca execute movimentos de deslocamento, recuo e rotação, proporcionando maior interação durante as apresentações.

O ESP32 é responsável por enviar sinais digitais às pontes H, determinando o sentido de rotação dos grupos de motores.

**Componentes da base:**

| Componente | Quantidade | Função |
|---|---|---|
| Motores DC | 8 | Movimentação da base |
| Módulos de ponte H | 2 | Acionamento e controle dos motores |
| ESP32 | 1 | Processamento dos sensores e comandos de movimento |
| Sensor TCS230/TCS3200 | 1 | Identificação da cor que inicia os movimentos |

Os motores são comandados por quatro canais de acionamento, identificados no código como:

- Traseira esquerda.
- Dianteira esquerda.
- Traseira direita.
- Dianteira direita.

O acionamento desses grupos permite executar movimentos para frente, para trás e giros para ambos os lados.

---

## 🔌 Pinagem e Conexões

### 1. ESP32 Mestre (Versão Principal — Sensor de Cor)

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

### 2. Pinagem da Base Motorizada — Versão OBR

Na versão utilizada na OBR, o ESP32 controla **8 motores DC por meio de duas pontes H**, sendo uma responsável pelos grupos do lado esquerdo e outra pelos grupos do lado direito.

**Ponte H 1 — Lado Esquerdo**

| Pino da ponte H | GPIO do ESP32 | Função |
|---|---|---|
| IN1 | GPIO 13 | Traseira esquerda |
| IN2 | GPIO 12 | Traseira esquerda |
| IN3 | GPIO 14 | Dianteira esquerda |
| IN4 | GPIO 27 | Dianteira esquerda |

**Ponte H 2 — Lado Direito**

| Pino da ponte H | GPIO do ESP32 | Função |
|---|---|---|
| IN1 (IN5 no código) | GPIO 33 | Traseira direita |
| IN2 (IN6 no código) | GPIO 32 | Traseira direita |
| IN3 (IN7 no código) | GPIO 21 | Dianteira direita |
| IN4 (IN8 no código) | GPIO 4 | Dianteira direita |

**Sensor de Cor — TCS230/TCS3200 (Versão OBR)**

| Pino do sensor | GPIO do ESP32 |
|---|---|
| S0 | GPIO 25 |
| S1 | GPIO 26 |
| S2 | GPIO 18 |
| S3 | GPIO 19 |
| OUT | GPIO 35 |

**Observação:** esta pinagem corresponde ao código da base utilizado na OBR. Ela é diferente da pinagem do sistema Mestre–Escravo com motor de passo, portanto as duas configurações não devem ser utilizadas simultaneamente sem reorganizar as conexões.

### 3. Funcionamento dos Motores DC

A movimentação da base é realizada por funções específicas implementadas no firmware do ESP32.

| Função | Descrição |
|---|---|
| `moverFrente()` | Aciona os motores para deslocar a Cuca para frente |
| `moverTras()` | Inverte o sentido dos motores para deslocamento para trás |
| `girarEsquerda()` | Aciona os lados em sentidos opostos para girar à esquerda |
| `girarDireita()` | Aciona os lados em sentidos opostos para girar à direita |
| `pararMotores()` | Desativa os sinais de acionamento dos motores |

O firmware utilizado na OBR possui uma sequência de **14 ações**, executadas conforme a identificação da cor verde pelo sensor TCS230/TCS3200.

Para evitar acionamentos indevidos, o sistema implementa uma **verificação dupla da cor verde**, com intervalo de confirmação de 60 milissegundos.

Após executar cada ação, o ESP32 aguarda a retirada do objeto verde antes de permitir uma nova movimentação.

Algumas ações utilizam movimentos alternados entre frente e trás, enquanto outras realizam giros para esquerda e direita durante intervalos definidos no código.

O sistema encerra a execução após completar as 14 ações programadas.

### 4. Pinagem das Versões Alternativas (Sensor Ultrassônico)

Nas versões adaptadas para feiras e apresentações, como a **Expoema** e a **UEMA**, localizadas na pasta `adaptacoes/`, utilizamos um **sensor ultrassônico HC-SR04** no Mestre para garantir maior estabilidade contra a variação de luz do ambiente.

**Sensor Ultrassônico — HC-SR04**

- `Trig` → GPIO 5
- `Echo` → GPIO 18

**Driver do Motor de Passo — ULN2003**

- `IN1` → GPIO 13
- `IN2` → GPIO 4
- `IN3` → GPIO 12
- `IN4` → GPIO 5

### 5. Comunicação UART — Mestre e Escravo

A comunicação entre os dois ESP32 utiliza uma interface serial UART.

| ESP32 Mestre | Direção | ESP32 Escravo |
|---|:---:|---|
| GPIO 17 (TX2) | → | GPIO 16 (RX2) |
| GPIO 16 (RX2) | ← | GPIO 17 (TX2) |
| GND | ↔ | GND |

**Observação:** ambos os microcontroladores devem compartilhar uma referência comum de GND para garantir a comunicação serial.

---

## 🛠️ Requisitos de Hardware e Alimentação

### 1. Alimentação

- Recomenda-se utilizar uma fonte externa de **5 V / 2 A** para alimentar o driver do motor de passo e o ESP32 Escravo, desde que sua capacidade seja suficiente para o consumo total desses componentes.
- Adicionar um capacitor eletrolítico de **470 µF a 1000 µF** próximo à alimentação do ESP32 Escravo para ajudar a mitigar quedas momentâneas de tensão causadas pelo consumo do rádio Bluetooth e reduzir possíveis *brownouts*.
- Garantir que todos os componentes estejam conectados corretamente e recebam as tensões adequadas.

### 2. Alimentação dos 8 Motores DC

A base motorizada utilizada na OBR necessita de alimentação dimensionada para os **8 motores DC** e os **2 módulos de ponte H**.

Recomendações:

- Utilizar uma fonte ou bateria compatível com a tensão nominal dos motores.
- Dimensionar cada ponte H considerando a corrente dos motores conectados, especialmente durante a partida ou travamento.
- Garantir que os motores sejam alimentados pelos drivers, e não diretamente pelos GPIOs do ESP32.
- Compartilhar uma referência comum de GND entre o ESP32 e os módulos de ponte H.
- Utilizar uma alimentação estável para evitar quedas de tensão e reinicializações inesperadas do microcontrolador.

**Importante:** a capacidade necessária da alimentação dos motores depende das especificações dos motores DC e dos módulos de ponte H utilizados.

### 3. Dispositivo de Áudio

O firmware das versões com Bluetooth foi projetado para transmitir áudio a caixas de som compatíveis com o perfil **A2DP Sink**.

---

## 💻 Bibliotecas Utilizadas

| Biblioteca | Finalidade |
|---|---|
| [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) | Transmissão de áudio Bluetooth A2DP |
| `Stepper` | Controle do motor de passo |
| `ArduinoOTA` | Atualização remota do firmware |
| `WiFi` | Conectividade com a rede sem fio |
| `Arduino.h` | Controle dos GPIOs, leitura de sensores e acionamento das pontes H |

As bibliotecas utilizadas dependem da versão do firmware. O código da base motorizada da OBR realiza o controle dos motores DC diretamente pelos GPIOs do ESP32.

---

## 📖 Nossa Trajetória com a Cuca

O projeto **Cuca Bot** nasceu da união entre eletrônica, programação, robótica e cultura popular.

Ao longo do tempo, o robô passou por diversas etapas de desenvolvimento, testes, melhorias e conquistas que marcaram nossa trajetória.

### 🛠️ 1. Processo de Desenvolvimento e Montagem

Tudo começou na bancada de testes, com a soldagem dos componentes, ajustes eletrônicos, organização das conexões e construção da estrutura física do robô.

A integração dos dois microcontroladores ESP32 foi uma etapa fundamental para o funcionamento do sistema de áudio e controle.

A base motorizada, composta por **8 motores DC e 2 pontes H**, também representou uma etapa importante no desenvolvimento, permitindo que a Cuca realizasse movimentos durante as interações com o público.

<p align="center">
  <img src="fotos/etapa(1).jpeg" width="350" alt="Primeira etapa de desenvolvimento do Cuca Bot">
  <img src="fotos/etapa%20(5).jpeg" width="350" alt="Segunda etapa de montagem do Cuca Bot">
</p>

### 🥉 2. Conquista na Olimpíada Brasileira de Robótica (OBR)

Levamos a Cuca para competir na **Olimpíada Brasileira de Robótica (OBR)** e conquistamos o **3º lugar**!

Durante a competição, a Cuca utilizou sua base com **8 motores DC e 2 pontes H**, executando movimentos programados a partir da detecção de cores.

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
