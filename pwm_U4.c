#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define SERVO_PIN 22  // GPIO para o sinal PWM do servo
#define LED_RGB 12    // GPIO do LED RGB
#define FREQ_PWM 50   // Frequência do PWM (50Hz)
#define PERIOD 20000  // Período do PWM (20ms)
#define STEP 5        // Passo de incremento para suavizar movimento
#define DELAY_MS 10   // Atraso entre incrementos

// Configura o PWM para um determinado duty cycle (em microssegundos)
void set_pwm_duty(uint slice_num, uint channel, float duty_us) {
    uint16_t wrap_value = clock_get_hz(clk_sys) / (FREQ_PWM * 64); // Cálculo do valor de wrap
    uint16_t level = (duty_us / PERIOD) * wrap_value; // Conversão do duty cycle para o valor PWM
    pwm_set_gpio_level(SERVO_PIN, level); // Aplica o nível PWM no pino do servo
}

// Configuração inicial do PWM
void setup_pwm() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM); // Define a GPIO como saída PWM
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN); // Obtém o slice correspondente
    pwm_config config = pwm_get_default_config(); // Obtém a configuração padrão do PWM
    
    pwm_config_set_clkdiv(&config, 64.0f); // Configura o divisor de clock
    pwm_config_set_wrap(&config, clock_get_hz(clk_sys) / (FREQ_PWM * 64)); // Define o valor de wrap
    
    pwm_init(slice_num, &config, true); // Inicializa o PWM com as configurações definidas
}

// Movimenta o servo suavemente entre dois ângulos
void move_servo_smoothly(uint slice_num, uint channel, float start_us, float end_us) {
    if (start_us < end_us) {
        for (float duty = start_us; duty <= end_us; duty += STEP) {
            set_pwm_duty(slice_num, channel, duty); // Aplica o novo duty cycle
            sleep_ms(DELAY_MS); // Aguarda um curto período para suavizar o movimento
        }
    } else {
        for (float duty = start_us; duty >= end_us; duty -= STEP) {
            set_pwm_duty(slice_num, channel, duty);
            sleep_ms(DELAY_MS);
        }
    }
}

// Configura GPIO do LED RGB
void setup_led() {
    gpio_init(LED_RGB); // Inicializa a GPIO do LED
    gpio_set_dir(LED_RGB, GPIO_OUT); // Define a GPIO como saída
}

int main() {
    stdio_init_all(); // Inicializa a comunicação serial (opcional)
    setup_pwm(); // Configuração inicial do PWM
    setup_led(); // Configuração inicial do LED
    
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN); // Obtém o slice do PWM

    while (true) {
        gpio_put(LED_RGB, 1); // Liga o LED RGB durante a movimentação do servo
        
        // Posição 180° (2.400µs)
        printf("Posição do servo motor em 180°.\n");
        set_pwm_duty(slice_num, 0, 2400);
        sleep_ms(5000);
        
        // Posição 90° (1.470µs)
        printf("Posição do servo motor em 90°.\n");
        set_pwm_duty(slice_num, 0, 1470);
        sleep_ms(5000);
        
        // Posição 0° (500µs)
        printf("Posição do servo motor em 0°.\n");
        set_pwm_duty(slice_num, 0, 500);
        sleep_ms(5000);
        
        // Movimento suave entre 0° e 180°
        printf("Posição do servo motor de 0 a 180°.\n");
        move_servo_smoothly(slice_num, 0, 500, 2400);
        move_servo_smoothly(slice_num, 0, 2400, 500);
        
        gpio_put(LED_RGB, 0); // Desliga o LED RGB após a movimentação
        sleep_ms(1000);
    }
}
