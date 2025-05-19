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

// Criando as filas que seram utilizadas
QueueHandle_t xQueueSensorsData;
QueueHandle_t xQueueAlerts;


// FUNÇÕES AUXILIARES =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Função para configurar o PWM e iniciar com 0% de DC
void set_pwm(uint gpio, uint wrap){
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice_num, clkdiv);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); 
    pwm_set_gpio_level(gpio, 0);
}

// Função para fazer pontos
void make_point(){
    pwm_set_gpio_level(LED_GREEN, wrap*0.05);
    pwm_set_gpio_level(LED_RED, wrap*0.05);
    vTaskDelay(pdMS_TO_TICKS(200));
    pwm_set_gpio_level(LED_GREEN, 0);
    pwm_set_gpio_level(LED_RED, 0);
    vTaskDelay(pdMS_TO_TICKS(200));
}

// Função para fazer pontos
void make_line(){
    pwm_set_gpio_level(LED_GREEN, wrap*0.05);
    pwm_set_gpio_level(LED_RED, wrap*0.05);
    vTaskDelay(pdMS_TO_TICKS(600));
    pwm_set_gpio_level(LED_GREEN, 0);
    pwm_set_gpio_level(LED_RED, 0);
    vTaskDelay(pdMS_TO_TICKS(200));
}


// TASKS UTILIZADAS NO CÓDIGO =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Task para leitura do ADC
void vTaskReadSensors(void *params){
    // Configurando o ADC
    adc_init();
    adc_gpio_init(JOYSTICK_X); // Canal 1
    adc_gpio_init(JOYSTICK_Y); // Canal 0

    // Structs que serão utilizadas para armazenar os valores
    Joy joystick;
    Sensors sensors;
    Alerts alerts;


    while(true){
        // Leitura do Eixo X (Canal 1)
        adc_select_input(1);
        joystick.vrx_value = adc_read();
        // Leitura do Eixo Y (Canal 0)
        adc_select_input(0);
        joystick.vry_value = adc_read();

        // Conversão para porcentagem
        // Nível de água
        sensors.water_level = (float)(joystick.vrx_value*100)/4095;
        // Volume de chuva
        sensors.rain_volume = (float)(joystick.vry_value*100)/4095;

        // Manipulação das flags de alerta
        // Modo modo normal
        if(sensors.water_level<70.0f && sensors.rain_volume<80.0f){
            alerts.normal_mode = 1;
            alerts.alert_rain_volume = 0;
            alerts.alert_water_level = 0;
        }
        // Modos de alerta
        else{
            alerts.normal_mode = 0;
            alerts.alert_rain_volume = 0;
            alerts.alert_water_level = 0;
            // Nivel de agua
            if(sensors.water_level>=70.0f){
                alerts.alert_water_level = 1;
            }
            // Volume de chuva
            if(sensors.rain_volume>=80.0f){
                alerts.alert_rain_volume = 1;
            }
        }

        // Enviando os dados para a fila
        xQueueSend(xQueueSensorsData, &sensors, portMAX_DELAY);
        xQueueSend(xQueueAlerts, &alerts, portMAX_DELAY);

        //printf("[Tarefa: %s]\tJoy_x: %u | Joy_y: %u\n", pcTaskGetName(NULL), joystick.vrx_value, joystick.vry_value);
        printf("[Tarefa: %s]\tWater_Level: %.2f | Rain_Volume: %.2f\n", pcTaskGetName(NULL), sensors.water_level, sensors.rain_volume);
        printf("[Tarefa: %s]\tNormal: %d | Water: %d | Rain: %d\n", pcTaskGetName(NULL), alerts.normal_mode, alerts.alert_water_level, alerts.alert_rain_volume);
        vTaskDelay(pdMS_TO_TICKS(16)); // 20 Hz de leitura
    }
}


// Task para controlar a matriz de LEDs
void vLedMatrixTask(void *params){
    // Inicializando a PIO
    pio = pio0;
    sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_MATRIX_PIN, 800000, IS_RGBW);

    // Variáveis para controlar a animação da matriz
    float matrix_intensity;
    int matrix_intensity_step=0;
    bool matrix_intensity_rising=true;

    // Variável para armazenar os dados recebidos da fila
    Alerts alerts;
    Sensors sensors;

    while(true){
        // Leitura da fila com dados de sensores
        xQueueReceive(xQueueSensorsData, &sensors, portMAX_DELAY);

        // Verificação dos alertas
        if(xQueueReceive(xQueueAlerts, &alerts, portMAX_DELAY)){

            // Detecta os alertas de chuva e agua
            if(alerts.alert_rain_volume || alerts.alert_water_level){
                matrix_intensity = 0.01*matrix_intensity_step;
                yellow_animation(matrix_intensity);
                // Animação de pulsar o desenho na matriz de leds
                if(matrix_intensity_rising){ 
                    matrix_intensity_step++;
                    if(matrix_intensity_step==10){
                        matrix_intensity_rising=false;
                    }
                }
                else{
                    matrix_intensity_step--;
                    if(matrix_intensity_step==0){
                        matrix_intensity_rising=true;
                    }
                }
            }

            // Se estiver no modo normal
            else{
                // Exibe o nivel na matriz de leds
                update_levels(sensors.water_level, sensors.rain_volume);
            }


        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Task para controlar o Led RGB
void vRGBLedTask(void *params){
    // Iniciando os pinos do Led RGB como PWM
    set_pwm(LED_RED, wrap);
    set_pwm(LED_GREEN, wrap);
    set_pwm(LED_BLUE, wrap);

    // Intensidade dos LEDs
    float alert_intensity;
    bool led_intensity_rising = true;
    int alert_intensity_step=0;

    // Variável para armazenar os dados recebidos da fila
    Alerts alerts;
    Sensors sensors;

    while(true){
        // Leitura da fila com dados de sensores
        xQueueReceive(xQueueSensorsData, &sensors, portMAX_DELAY);

        // Verificação dos alertas
        if(xQueueReceive(xQueueAlerts, &alerts, portMAX_DELAY)){
            // Modo alerta (Emite SOS)
            if(alerts.alert_rain_volume || alerts.alert_water_level){
                // S
                make_point(); make_point(); make_point(); 
                vTaskDelay(pdMS_TO_TICKS(300)); // Espaço entre letras

                // O
                make_line(); make_line(); make_line(); 
                vTaskDelay(pdMS_TO_TICKS(300)); // Espaço entre letras

                // S
                make_point(); make_point(); make_point(); 
                vTaskDelay(pdMS_TO_TICKS(1000)); // Espaço entre palavras
                }

            // Modo normal
            else{
                pwm_set_gpio_level(LED_GREEN, 0);
                pwm_set_gpio_level(LED_RED, 0);
            }
        }
            
    }

    vTaskDelay(pdMS_TO_TICKS(50));
}


// Task para controlar o Buzzer
void vBuzzerTask(void *params){
    set_pwm(BUZZER_A, wrap);
    set_pwm(BUZZER_B, wrap);

    // Variável para armazenar os dados recebidos da fila
    Alerts alerts;
    Sensors sensors;

    // Booleano para on/off do alerta CRÍTICO
    bool critical_buzzer = true;
    // Contador para o som do VOLUME DE CHUVA
    int rain_volume_count = 0;
    bool rain_volume_active = true;
    // Contador para o som do VOLUME DE CHUVA
    int water_level_count = 0;
    bool water_level_active = true;

    while(true){
        // Leitura da fila com dados de sensores
        xQueueReceive(xQueueSensorsData, &sensors, portMAX_DELAY);

        // Verificação dos alertas
        if(xQueueReceive(xQueueAlerts, &alerts, portMAX_DELAY)){
            // Alerta CRÍTICO (ambos estouram)
            if(alerts.alert_rain_volume && alerts.alert_water_level){
                if(critical_buzzer){
                    pwm_set_gpio_level(BUZZER_A, wrap*0.05);
                    pwm_set_gpio_level(BUZZER_B, wrap*0.05);
                    critical_buzzer = !critical_buzzer;
                }
                else{
                    pwm_set_gpio_level(BUZZER_A, 0);
                    pwm_set_gpio_level(BUZZER_B, 0);
                    critical_buzzer = !critical_buzzer;
                }
            }

            // Alerta volume de chuva
            else if(alerts.alert_rain_volume){
                if(rain_volume_count<5){
                    if(rain_volume_active){
                        pwm_set_gpio_level(BUZZER_A, wrap*0.05);
                        pwm_set_gpio_level(BUZZER_B, wrap*0.05);
                        rain_volume_active = !rain_volume_active;
                    }
                    else{
                        pwm_set_gpio_level(BUZZER_A, 0);
                        pwm_set_gpio_level(BUZZER_B, 0);
                        rain_volume_active = !rain_volume_active;
                    }
                }
                rain_volume_count++;

                if(rain_volume_count==10){
                    rain_volume_count=0;
                }
            }

            // Alerta nível de água
            else if(alerts.alert_water_level){
                if(water_level_count<4){
                    if(water_level_active){
                        pwm_set_gpio_level(BUZZER_A, wrap*0.05);
                        pwm_set_gpio_level(BUZZER_B, wrap*0.05);
                        water_level_active = !water_level_active;
                    }
                    else{
                        pwm_set_gpio_level(BUZZER_A, 0);
                        pwm_set_gpio_level(BUZZER_B, 0);
                        water_level_active = !water_level_active;
                    }
                }
                water_level_count++;

                if(water_level_count==10){
                    water_level_count=0;
                }
            }

            // Modo normal (buzzer off)
            else{
                pwm_set_gpio_level(BUZZER_A, 0);
                pwm_set_gpio_level(BUZZER_B, 0);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


// FUNÇÃO MAIN =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int main(){
    stdio_init_all();

    // Cria a fila para compartilhamento de valor simulado dos sensores
    xQueueSensorsData = xQueueCreate(10, sizeof(Sensors));
    xQueueAlerts = xQueueCreate(10, sizeof(Alerts));

    // Criação das Tasks
    xTaskCreate(vTaskReadSensors, "Read Sensors", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vLedMatrixTask, "Led Matrix", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vRGBLedTask, "Led RGB", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBuzzerTask, "Buzzer Alert", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    
    // Inicia o agendador
    vTaskStartScheduler();
    panic_unsupported();
}
