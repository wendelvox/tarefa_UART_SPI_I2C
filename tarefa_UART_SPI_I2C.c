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
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C


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
bool led_green_state = false;  // Estado do LED Verde
bool led_blue_state = false;  // Estado do LED Verde




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

//Função para exibir informação no Display
void display_char(ssd1306_t *ssd, char c) {
    bool cor = true;
    char buffer[2];  // Buffer para armazenar o caractere como string
    sprintf(buffer, "%c", c);  // Converte o caractere para string    
    ssd1306_fill(ssd, false);  // Limpa o display
    ssd1306_rect(ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_draw_string(ssd, buffer, 55, 20);  // Desenha o caractere no centro do display
    ssd1306_send_data(ssd);  // Atualiza o display
}

// Função para exibir o estado do LED no display
void display_led_green_state(ssd1306_t *ssd, bool led_state) {
    bool cor = !cor;
    char buffer[20];  // Buffer para armazenar o texto
    if (led_state) {
        
        sprintf(buffer, "LED VERDE ON");  // Se o LED estiver aceso
    } else {
      
        sprintf(buffer, "LED VERDE OFF");  // Se o LED estiver apagado
    }
    
    ssd1306_fill(ssd, false);  // Limpa o display
    ssd1306_rect(ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_draw_string(ssd, buffer, 10, 20);  // Desenha o texto no display
    ssd1306_send_data(ssd);  // Atualiza o display
}

// Função para exibir o estado do LED no display
void display_led_blue_state(ssd1306_t *ssd, bool led_state) {
    bool cor = !cor;
    char buffer[20];  // Buffer para armazenar o texto
    if (led_state) {
      
        sprintf(buffer, "LED AZUL ON");  // Se o LED estiver aceso
    } else {
        
        sprintf(buffer, "LED AZUL OFF");  // Se o LED estiver apagado
    }
    
    ssd1306_fill(ssd, false);  // Limpa o display
    ssd1306_rect(ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_draw_string(ssd, buffer, 10, 20);  // Desenha o texto no display
    ssd1306_send_data(ssd);  // Atualiza o display
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
    

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA); // Pull up the data line
  gpio_pull_up(I2C_SCL); // Pull up the clock line  

    // Mensagem inicial
    uart_puts(UART_ID, "UART - Digite um número (0-9) para exibir na matriz:\r\n");
}



int main() {
    PIO pio;
    uint sm;
    setup(&pio, &sm);

    
    ssd1306_t ssd; // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    bool cor = true;

    while (1) {       

           // Verifica entrada UART
    if (uart_is_readable(UART_ID)) {
    char c = uart_getc(UART_ID);

    if (c >= '0' && c <= '9') { // Se for um número válido
        current_number = c - '0'; // Converte char para int
        matriz_ws2812(current_number, pio, sm); // Exibe na matriz de LEDs
    } else {  // Para qualquer outro caractere
        display_char(&ssd, c); // Exibe o caractere no display OLED
    }
    
    uart_putc(UART_ID, c);  // Envia o caractere de volta para a UART
    uart_puts(UART_ID, " <- Caractere exibido\r\n");
    }
         // Verifica botão A (LED Verde)
        if (button_a_pressed) {
            button_a_pressed = false;  // Reseta a variável
            led_green_state = !led_green_state;  // Alterna o estado do LED verde
            gpio_put(LED_PIN_G, led_green_state); // Atualiza o LED verde
            uart_puts(UART_ID, "Botão A pressionado - LED Verde alternado\r\n");
            display_led_green_state(&ssd, led_green_state);  // Atualiza o display com o estado do LED
        }

        // Verifica botão B (LED Azul)
        if (button_b_pressed) {
            button_b_pressed = false;  // Reseta a variável
            led_blue_state = !led_blue_state;  
            gpio_put(LED_PIN_B, led_blue_state); // Atualiza o LED azul
            uart_puts(UART_ID, "Botão B pressionado - LED Azul alternado\r\n");
            display_led_blue_state(&ssd, led_blue_state);  // Atualiza o display com o estado do LED
        }
        sleep_ms(40); // Pequeno atraso
    }

    return 0;
}