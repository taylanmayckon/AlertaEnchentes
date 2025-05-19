#include <stdio.h>
#include "pico/stdlib.h"

// Includes do FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

// Includes para configuração dos periféricos da BitDogLab
#include "led_matrix.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"   

// Structs
#include "structs.h"
     

// LED RGB, botão e buzzers
#define LED_RED 13
#define LED_GREEN 11
#define LED_BLUE 12
#define BUTTON_A 5
#define BUZZER_A 21 
#define BUZZER_B 10
// Analogicos
#define JOYSTICK_X 27
#define JOYSTICK_Y 26
// Constantes para a matriz de leds
#define IS_RGBW false
#define LED_MATRIX_PIN 7
// Definições da I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// Booleano para indicar se vai imprimir branco no display
bool cor = true;
// Variáveis da PIO declaradas no escopo global
PIO pio;
uint sm;
// Variáveis do PWM (setado para freq. de 312,5 Hz)
uint wrap = 2000;
uint clkdiv = 25;

// Criando a fila que será utilizada
QueueHandle_t xQueueJoystickData;


// FUNÇÕES AUXILIARES =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-



// TASKS UTILIZADAS NO CÓDIGO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Task para leitura do ADC
void vTaskReadAnalogs(void *params){
    // Configurando o ADC
    adc_init();
    adc_gpio_init(JOYSTICK_X); // Canal 1
    adc_gpio_init(JOYSTICK_Y); // Canal 0

    // Struct que será utilizada para armazenar os valores
    Joy joystick;

    while(true){
        // Leitura do Eixo X (Canal 1)
        adc_select_input(1);
        joystick.vrx_value = adc_read();
        // Leitura do Eixo Y (Canal 0)
        adc_select_input(0);
        joystick.vry_value = adc_read();

        // Enviando os dados para a fila
        xQueueSend(xQueueJoystickData, &joystick, portMAX_DELAY);

        printf("[Tarefa: %s]\tJoy_x: %u | Joy_y: %u\n", pcTaskGetName(NULL), joystick.vrx_value, joystick.vry_value);
        vTaskDelay(pdMS_TO_TICKS(200)); // 10 Hz de leitura
    }
}

// Task para consumir a fila
void vTaskReadQueue(void *params){

    while(true){
        Joy joystick;

        if(xQueueReceive(xQueueJoystickData, &joystick, portMAX_DELAY)){
            printf("[Tarefa: %s]\tJoy_x: %u | Joy_y: %u\n", pcTaskGetName(NULL), joystick.vrx_value, joystick.vry_value);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


// FUNÇÃO MAIN =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int main(){
    stdio_init_all();

    // Cria a fila para compartilhamento de valor do joystick
    xQueueJoystickData = xQueueCreate(10, sizeof(Joy));

    // Criação das Tasks
    xTaskCreate(vTaskReadAnalogs, "Read Analogs", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vTaskReadQueue, "Read Queue", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    
    // Inicia o agendador
    vTaskStartScheduler();
    panic_unsupported();
}
