#include "led_matrix.h"

#define MATRIX_PIN 7
// Quantidade de pixels
#define NUM_PIXELS 25

// Buffer que armazena o frame atual, incluindo cores
Led_frame led_buffer= {{
    {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, 
    {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, 
    {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, 
    {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, 
    {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, 
}};


// Struct para armazenar as cores dos nívels
typedef struct {
    Led_color led[5];
} Color_levels;

// Cores para exibir na matriz
Color_levels level_colors = {{
    {0, 255, 0},   // Verde
    {255, 255, 0}, // Amarelo
    {255, 0, 0},   // Vermelho
    {255, 0, 0},   // Vermelho
    {255, 0, 0},   // Vermelho
}};


// FRAME DE ALERTA =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Led_frame yellow_frame = {{
    {0,0,0}, {0,0,0}, {255,255,0}, {0,0,0}, {0,0,0}, 
    {0,0,0}, {0,0,0}, {255,255,0}, {0,0,0}, {0,0,0}, 
    {0,0,0}, {0,0,0}, {255,255,0}, {0,0,0}, {0,0,0}, 
    {0,0,0}, {0,0,0},   {0,0,0},   {0,0,0}, {0,0,0}, 
    {0,0,0}, {0,0,0}, {255,255,0}, {0,0,0}, {0,0,0}, 
}};

// FRAME DE NIVEIS
Led_frame frame_levels;

static inline void put_pixel(uint32_t pixel_grb){
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Função que vai transformar valores correspondentes ao padrão RGB em dados binários
uint32_t urgb_u32(double r, double g, double b){
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Função que atualiza os Leds do vetor
void set_leds(float intensidade){
    uint32_t color; // Armazena os valores das cores

    // Define todos os LEDs com a cor especificada
    // Faz o processo de virar de cabeça para baixo o arranjo
    for (int i = NUM_PIXELS-1; i >= 0; i--){
        color = urgb_u32(intensidade*led_buffer.led[i].red, intensidade*led_buffer.led[i].green, intensidade*led_buffer.led[i].blue); // Converte as cores para o padrão aceito pela matriz
        put_pixel(color); // Liga o LED com um no buffer
    }
}

// Função genérica para atualiza matriz
void matrix_update_leds(Led_frame *frame, float intensidade){
    // Ordenando corretamente o vetor recebido no buffer

    // Organiza melhor essa lógica, talvez refatorando como uma matriz [5][5] ?
    int j = 0; // Variável para controle do index espelhado
    for(int i=0; i<25; i++){
        if(i>4 && i<10){
            led_buffer.led[i] = frame->led[9-j];
            j++;
        }
        else if(i>14 && i<20){
            led_buffer.led[i] = frame->led[19-j];
            j++;
        }
        else{
            j=0;
            led_buffer.led[i] = frame->led[i];
        }
    }
    set_leds(intensidade);
}

// Cor amarela
void yellow_animation(float intensidade){
    // Ordenando corretamente o vetor recebido no buffer
    int j = 0; // Variável para controle do index espelhado
    for(int i=0; i<25; i++){
        if(i>4 && i<10){
            led_buffer.led[i] = yellow_frame.led[9-j];
            j++;
        }
        else if(i>14 && i<20){
            led_buffer.led[i] = yellow_frame.led[19-j];
            j++;
        }
        else{
            j=0;
            led_buffer.led[i] = yellow_frame.led[i];
        }
    }
    set_leds(intensidade);
}

// Apagar leds
void off_leds(){
    set_leds(0);
}

// Atualiza os níveis na matriz de leds
void update_levels(float water_level, float rain_volume){
    float step = 20.0;

    // Limpa o led_buffer
    for(int i=0; i<25; i++){
        frame_levels.led[i] = (Led_color){0.0, 0.0, 0.0};
    }

    // Interpreta os valores de nível de água
    for(int i=0; i<5; i++){
        if(step*(i+1)<=water_level){
            frame_levels.led[21-(5*i)] = level_colors.led[i];
            frame_levels.led[20-((5*i))] = level_colors.led[i];
        }
    }

    // Interpreta os valores de volume de chuva
    for(int i=0; i<5; i++){
        if(step*(i+1)<=rain_volume){
            frame_levels.led[24-((5*i))] = level_colors.led[i];
            frame_levels.led[23-((5*i))] = level_colors.led[i];
        }
    }


    // Ordenando corretamente o vetor recebido no buffer
    int j = 0; // Variável para controle do index espelhado
    for(int i=0; i<25; i++){
        if(i>4 && i<10){
            led_buffer.led[i] = frame_levels.led[9-j];
            j++;
        }
        else if(i>14 && i<20){
            led_buffer.led[i] = frame_levels.led[19-j];
            j++;
        }
        else{
            j=0;
            led_buffer.led[i] = frame_levels.led[i];
        }
    }

    set_leds(0.01);
}