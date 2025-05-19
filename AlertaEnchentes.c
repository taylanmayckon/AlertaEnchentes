#include <stdio.h>
#include "pico/stdlib.h"

// Includes do FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

// Includes para configuração dos periféricos da BitDogLab
#include "led_matrix.h"
#include "ssd1306.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "font.h"
#include "hardware/adc.h"   
     



int main(){
    stdio_init_all();

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
