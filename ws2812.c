#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
  // Biblioteca para LEDs WS2812


#define LED_R 13
#define LED_G 11
#define LED_B 12
#define BUTTON_A 5
#define BUTTON_B 6
#define LED_PIN 7  // Pino do WS2812
#define NUMPIXELS 25
#define DEBOUNCE_DELAY 200  // Tempo menor para debounce
#define BLINK_INTERVAL 100 // Pisca o LED 5 vezes por segundo
#define IS_RGBW false

volatile int current_number = 0;
volatile uint32_t last_interrupt_time_a = 0;
volatile uint32_t last_interrupt_time_b = 0;

// Definição dos números no formato 5x5 (1 = LED aceso, 0 = LED apagado)
const char* numbers[] = {
    "0111001010010100101001110", // 0
    "0111000100001000110000100", // 1
    "0111001000011100001001110", // 2
    "0111000010011100001001110", // 3
    "1000000001111111000110001", // 4
    "1111100001111111000011111", // 5
    "0111001010011100100001110", // 6
    "1000000001100000000111111", // 7
    "0111001010011100101001110", // 8
   "0111000010011100101001110"  // 9
};

// Inicializa a matriz WS2812
void init_ws2812() {
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_PIN, 800000, IS_RGBW);
}

// Envia cor para um único LED
void set_pixel_color(int pixel, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b);
    pio_sm_put_blocking(pio0, 0, color << 8);
}

// Atualiza a matriz com base no número atual
void display_number(int num) {
    printf("Número exibido: %d\n", num);
    for (int i = 0; i < NUMPIXELS; i++) {
        if (numbers[num][i] == '1') {
            set_pixel_color(i, 0, 255, 0);  // Verde para LEDs acesos
        } else {
            set_pixel_color(i, 0, 0, 0);    // LED apagado
        }
    }
}

// Manipuladores de botão com debounce
void button_a_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (gpio_get(BUTTON_A)==0 && (current_time - last_interrupt_time_a) > DEBOUNCE_DELAY) {
        
        if (current_number < 9) {
            current_number++;

        }
        display_number(current_number);  // Atualiza o display após incremento
        last_interrupt_time_a = current_time;
    }
    if (gpio_get(BUTTON_B)==0 &&(current_time - last_interrupt_time_b) > DEBOUNCE_DELAY) {
        if (current_number > 0) {
            current_number--;
        }
        display_number(current_number);  // Atualiza o display após decremento
        last_interrupt_time_b = current_time;
    }
}


        
    
int main() {
    stdio_init_all();
    
    // Inicializa LED RGB
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);

    // Inicializa Botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_a_handler);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_a_handler);

    // Inicializa WS2812
    init_ws2812();
    
    // Exibe o primeiro número (0)
    display_number(0);

    // Loop principal - Piscar LED vermelho
    while (1) {
        gpio_put(LED_R, 1);
        sleep_ms(BLINK_INTERVAL);
        gpio_put(LED_R, 0);
        sleep_ms(BLINK_INTERVAL);
    }
}