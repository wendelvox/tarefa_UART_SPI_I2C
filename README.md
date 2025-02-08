# Projeto: Controle de LEDs e Display com Raspberry Pi Pico e BitDogLab

## Autor: Wendel
## Video: https://youtu.be/cBBmoyHNMHo
## 📌 Objetivos

Este projeto tem os seguintes objetivos:

- Compreender o funcionamento e a aplicação de comunicação serial em microcontroladores.
- Aplicar os conhecimentos adquiridos sobre **UART** e **I2C** na prática.
- Manipular e controlar **LEDs comuns e LEDs endereçáveis WS2812**.
- Fixar o estudo do uso de **botões de acionamento, interrupções e debounce**.
- Desenvolver um **projeto funcional** que combine hardware e software.

## 📝 Descrição do Projeto

O projeto utiliza a **placa BitDogLab** e foi desenvolvido para controlar LEDs, um display OLED e interagir via botões físicos e comunicação serial.

### 🔧 **Componentes Utilizados**

- **Matriz 5x5 de LEDs WS2812** (endereçáveis), conectada à **GPIO 7**.
- **LED RGB**, com os pinos conectados às **GPIOs 11, 12 e 13**.
- **Botão A** conectado à **GPIO 5**.
- **Botão B** conectado à **GPIO 6**.
- **Display SSD1306** conectado via **I2C** (GPIO 14 e GPIO 15).

## 📜 Código Principal

O código foi escrito em **C** utilizando a biblioteca `pico/stdlib.h` para comunicação e controle do hardware. Ele realiza as seguintes funções:

- Inicializa os periféricos (display, LEDs, botões).
- Configura interrupções para os botões.
- Atualiza o estado dos LEDs ao pressionar os botões.
- Exibe informações no display **SSD1306**.
- Permite interação via **UART**.

## 🚀 Como Executar

### 1️⃣ **Configurar o Ambiente de Desenvolvimento**
Certifique-se de ter instalado:
- **SDK do Raspberry Pi Pico**
- **CMake** e **GNU Make**
- **Ferramenta de compilação arm-none-eabi**

### 2️⃣ **Compilar e Gravar no Raspberry Pi Pico**

Clone este repositório:
```bash
git clone https://github.com/wendelvox/tarefa_UART_SPI_I2C.git
cd seu-repositorio
```

Compile o código:
```bash
mkdir build && cd build
cmake ..
make
```

Conecte o **Raspberry Pi Pico** no modo **BOOTSEL** e copie o arquivo `.uf2` gerado para ele.

### 3️⃣ **Executar e Testar**
Após a gravação, utilize o **PuTTY** ou outro terminal serial para interagir:
```bash
sudo minicom -b 115200 -o -D /dev/ttyUSB0
```
Pressione os botões **A e B** para ligar/desligar os LEDs.

## 📷 Imagens (Opcional)
![image](https://github.com/user-attachments/assets/0e81d643-72ad-4562-bcc5-e0622b92a482)


## 🔗 Referências
- [Documentação oficial do Raspberry Pi Pico](https://www.raspberrypi.com/documentation/microcontrollers/)
- [BitDogLab no GitHub](https://github.com/bitdoglab)
