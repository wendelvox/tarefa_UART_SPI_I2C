/*
 * Por: Wendel Souza Santos

 * Objetivos
• Compreender o funcionamento e a aplicação de comunicação serial em microcontroladores.
• Aplicar os conhecimentos adquiridos sobre UART e I2C na prática.
• Manipular e controlar LEDs comuns e LEDs endereçáveis WS2812.
• Fixar o estudo do uso botões de acionamento, interrupções e Debounce.
• Desenvolver um projeto funcional que combine hardware e software.
 * 
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "matriz_led.h"
#include "ws2812.pio.h" 
#include "hardware/uart.h"


#define UART_ID uart0 // Seleciona a UART0
#define BAUD_RATE 115200 // Define a taxa de transmissão
#define UART_TX_PIN 0 // Pino GPIO usado para TX
#define UART_RX_PIN 1 // Pino GPIO usado para RX

// Definição dos pinos do LED RGB
#define LED_PIN_G 11
#define LED_PIN_B 12

// Definição dos pinos dos botões
#define BUTTON_A 5
#define BUTTON_B 6

//MATRIZ 5X5 WS2812
#define WS2812_PIN 7

// Variáveis globais
int current_number = 0;  // Número a ser exibido na matriz
bool button_a_pressed = false;
bool button_b_pressed = false;
uint64_t last_button_a_time = 0;
uint64_t last_button_b_time = 0;
const uint64_t DEBOUNCE_DELAY = 200 * 1000; // 200ms debounce




// Função para verificar se um botão foi pressionado
bool is_button_pressed(uint button) {
    static bool button_a_last_state = true;
    static bool button_b_last_state = true;
    
    bool current_state = gpio_get(button);
        
    if (button == BUTTON_A) {
        if (!current_state && button_a_last_state) { // Detecta transição de HIGH para LOW
            button_a_last_state = current_state;
            return true;
        }
        button_a_last_state = current_state;
    } 
    else if (button == BUTTON_B) {
        if (!current_state && button_b_last_state) { // Detecta transição de HIGH para LOW
            button_b_last_state = current_state;
            return true;
        }
        button_b_last_state = current_state;
    }

    return false;
}
//Função MATRIZ de LED
void matriz_ws2812(int number, PIO pio, uint sm) {   
    const bool *pattern = matriz_numeros[number];      
    uint32_t color = 0; // Inicializa a variável para armazenar a cor
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            int index = row * 5 + col;
            
            // Se o LED da matriz estiver "aceso", aplica a cor com brilho
            if (pattern[index]) {
                color = (128 << 8) | 0 | 0 ; // Forma a cor no formato RGB  (G<<24)|(R<<16)|(B<<8);
            } else {
                color = 0; // Apaga o LED (sem brilho)
            }
            // Envia a cor para o PIO para o controle do LED
            pio_sm_put_blocking(pio, sm, color);
        }
    }
}

// Função de interrupção 
void button_irq_handler(uint gpio, uint32_t events) {
    uint64_t current_time = time_us_32();

    if (gpio == BUTTON_A && current_time - last_button_a_time > DEBOUNCE_DELAY) {
        button_a_pressed = true;
        last_button_a_time = current_time;
    } 
    else if (gpio == BUTTON_B && current_time - last_button_b_time > DEBOUNCE_DELAY) {
        button_b_pressed = true;
        last_button_b_time = current_time;
    }
}


// Função para inicializar os LEDs, BOTÕES, MATRIZ E DISPLAY
void setup(PIO *pio, uint *sm) {
    stdio_init_all();
    uart_init(UART_ID, BAUD_RATE);
    
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Inicializa LEDs como saída
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);
    gpio_init(LED_PIN_B);
    gpio_set_dir(LED_PIN_B, GPIO_OUT);
    
    // Inicializa botões como entrada com pull-up
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, button_irq_handler);


    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, button_irq_handler);  
    
 // Inicializa o PIO para a matriz WS2812
    *pio = pio0;
    *sm = 0;
    uint offset = pio_add_program(*pio, &tarefa_UART_SPI_I2C_program);
    tarefa_UART_SPI_I2C_program_init(*pio, *sm, offset, WS2812_PIN, 800000, false);
    
    // Mensagem inicial
    uart_puts(UART_ID, "UART - Digite um número (0-9) para exibir na matriz:\r\n");
}


int main() {
    PIO pio;
    uint sm;
    setup(&pio, &sm);



    while (1) {
        // Verifica entrada UART
           // Verifica entrada UART
        if (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);
            if (c >= '0' && c <= '9') { // Se for um número válido
                current_number = c - '0'; // Converte char para int
                matriz_ws2812(current_number, pio, sm);
            }
            uart_putc(UART_ID, c);
            uart_puts(UART_ID, " <- Número exibido\r\n");
        }

         // Verifica botão A (LED Verde)
        if (is_button_pressed(BUTTON_A)) {
            gpio_put(LED_PIN_G, !gpio_get(LED_PIN_G));
            uart_puts(UART_ID, "Botão A pressionado - LED Verde alternado\r\n");
         
        }

        // Verifica botão B (LED Azul)
        if (is_button_pressed(BUTTON_B)) {
            gpio_put(LED_PIN_B, !gpio_get(LED_PIN_B));
            uart_puts(UART_ID, "Botão B pressionado - LED Azul alternado\r\n");
        }
        sleep_ms(40); // Pequeno atraso
    }

    return 0;
}