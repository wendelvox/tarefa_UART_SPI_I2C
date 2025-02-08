#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "matriz_led.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "ws2812.pio.h"

// ---------------- Variáveis - Início ----------------
static volatile uint32_t last_time = 0; // Tempo do último callback
static ssd1306_t *ssd_pointer; // Ponteiro para o display
static char string_a[] = "LED VERDE: OFF"; // Estado do LED verde
static char string_b[] = "LED AZUL: OFF";  // Estado do LED azul

bool led_green_state = false;  // Estado do LED Verde
bool led_blue_state = false;   // Estado do LED Azul

// ---------------- Configurações - Início ----------------
#define UART_ID uart0 // Porta UART
#define I2C_PORT i2c1 // Porta I2C
#define I2C_SDA 14 // Pino SDA
#define I2C_SCL 15 // Pino SCL
#define ENDERECO 0x3C // Endereço do display
#define BUTTON_A 5 // Botão verde
#define BUTTON_B 6 // Botão vermelho
#define RED_RGB 13 // LED vermelho
#define GREEN_RGB 11 // LED verde
#define BLUE_RGB 12 // LED azul
#define LUZ 1 // Intensidade da luz

// ---------------- WS2812 - Início ----------------
#define LED_COUNT 25
#define LED_PIN 7

// ---------------- Funções de Inicialização - Início ----------------
void start_display(ssd1306_t *ssd) {
  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);
  ssd1306_init(ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
  ssd1306_config(ssd);
  ssd1306_send_data(ssd);
  ssd1306_fill(ssd, false);
  ssd1306_send_data(ssd);
}

void init_buttons() {
  gpio_init(BUTTON_A);
  gpio_init(BUTTON_B);
  gpio_set_dir(BUTTON_A, GPIO_IN);
  gpio_set_dir(BUTTON_B, GPIO_IN);
  gpio_pull_up(BUTTON_A);
  gpio_pull_up(BUTTON_B);
}

void init_RGB() {
  gpio_init(RED_RGB);
  gpio_init(GREEN_RGB);
  gpio_init(BLUE_RGB);
  gpio_set_dir(RED_RGB, GPIO_OUT);
  gpio_set_dir(GREEN_RGB, GPIO_OUT);
  gpio_set_dir(BLUE_RGB, GPIO_OUT);
  gpio_put(RED_RGB, 0);
  gpio_put(GREEN_RGB, 0);
  gpio_put(BLUE_RGB, 0);
}
// ---------------- Funções de Inicialização - Fim ----------------

// ---------------- Interrupções - Início ----------------
void gpio_irq_callback(uint gpio, uint32_t events) {
  uint32_t current_time = to_ms_since_boot(get_absolute_time());
  if (current_time - last_time > 200) {
    last_time = current_time;
    if (gpio == BUTTON_A) {
      led_green_state = !led_green_state;
      gpio_put(GREEN_RGB, led_green_state);
      snprintf(string_a, sizeof(string_a), "LED VERDE %s", led_green_state ? "ON" : "OFF");
      printf("-------- A pressionado! LED verde: %s --------\n", led_green_state ? "ON" : "OFF");
    } else if (gpio == BUTTON_B) {
      led_blue_state = !led_blue_state;
      gpio_put(BLUE_RGB, led_blue_state);
      snprintf(string_b, sizeof(string_b), "LED AZUL %s", led_blue_state ? "ON" : "OFF");
      printf("-------- B pressionado! LED azul: %s --------\n", led_blue_state ? "ON" : "OFF");
    }
    // Atualiza a tela após o pressionamento
    ssd1306_fill(ssd_pointer, false);
    ssd1306_draw_string(ssd_pointer, string_a, 8, 40);
    ssd1306_draw_string(ssd_pointer, string_b, 8, 48);
    ssd1306_send_data(ssd_pointer);
  }
}
// ---------------- Interrupções - Fim ----------------

// ---------------- Main - Início ----------------
int main() {
  ssd1306_t ssd;
  ssd_pointer = &ssd;
  char c[] = "?\0"; // String do caractere pressionado

  stdio_init_all();
  uart_init(UART_ID, 115200);
  gpio_set_function(0, GPIO_FUNC_UART);
  gpio_set_function(1, GPIO_FUNC_UART);

  start_display(&ssd);
  npInit(LED_PIN);
  npClear();
  npWrite();
  init_buttons();
  init_RGB();

  gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback);
  gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback);

  // Desenha a interface inicial
  ssd1306_rect(&ssd, 0, 0, 128, 64, true, false);
  ssd1306_rect(&ssd, 2, 2, 124, 60, true, false);
  ssd1306_draw_string(&ssd, "Digitou ", 8, 16);
  ssd1306_draw_string(&ssd, c, 80, 16); // Desenha o caractere digitado
  ssd1306_draw_string(&ssd, string_a, 8, 40);
  ssd1306_draw_string(&ssd, string_b, 8, 48);
  ssd1306_send_data(&ssd);

while(true) {
    sleep_ms(20);

    // Código para a comunicação serial com a BitDogLab pelo terminal do VScode 
    if(stdio_usb_connected) {

      c[0] = getc(stdin); // Recebe o caractere digitado

      handle_numbers(c[0]); // Aciona a matriz
      ssd1306_draw_string(&ssd, "Digitou ", 8, 16);
      ssd1306_draw_string(&ssd, c, 80, 16); // Desenha o caractere digitado
      ssd1306_send_data(&ssd); // Atualiza o display
  }
}
}
// ---------------- Main - Fim ----------------
