/*
 * Por: Wilton Lacerda Silva
 *    Comunicação serial com I2C
 *  
 * Uso da interface I2C para comunicação com o Display OLED
 * 
 * Estudo da biblioteca ssd1306 com PicoW na Placa BitDogLab.
 *  
 * Este programa escreve uma mensagem no display OLED.
 * 
 * 
*/


#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/timer.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define LED_GREEN 11
#define LED_BLUE  12
char c='a';
ssd1306_t ssd; // Inicializa a estrutura do display

#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define LED_PISCA 13
#define tempo 100

const uint button_0 = 5;
const uint button_1 = 6;
uint contador = 0;

// Variável global para armazenar a cor (Entre 0 e 255 para intensidade)
uint8_t led_r = 0; // Intensidade do vermelho
uint8_t led_g = 0; // Intensidade do verde
uint8_t led_b = 1; // Intensidade do azul


static volatile uint a = 1;
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)

// Prototipação da função de interrupção
static void gpio_irq_handler(uint gpio, uint32_t events);

// Buffer para desligar leds da matriz
bool desliga_led[NUM_PIXELS] = {
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0
};



static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}


void set_one_led(uint8_t r, uint8_t g, uint8_t b, bool matriz[])
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (matriz[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0);  // Desliga os LEDs com zero no buffer
        }
    }
}

// vetores correspondentes aos dígitos 0-9 a serem desenhados na matriz 
        bool num_0[NUM_PIXELS] = {
        0, 1, 1, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 1, 1, 0 };
        
      

      
        bool num_1[NUM_PIXELS] = {
        0, 0, 1, 0, 0, 
        0, 0, 1, 0, 0, 
        0, 0, 1, 0, 0, 
        0, 0, 1, 0, 0, 
        0, 0, 1, 0, 0 };
        
      

      
        bool num_2[NUM_PIXELS] = {
        0, 1, 1, 1, 0, 
        0, 1, 0, 0, 0, 
        0, 1, 1, 1, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 1, 1, 0 };
        
      

      
        bool num_3[NUM_PIXELS] = {
        0, 1, 1, 1, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 1, 1, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 1, 1, 0 };
        

       
        bool num_4[NUM_PIXELS] = {
        0, 1, 0, 0, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 1, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 0, 1, 0 };
        

      
        bool num_5[NUM_PIXELS] = {
        0, 1, 1, 1, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 1, 1, 0, 
        0, 1, 0, 0, 0, 
        0, 1, 1, 1, 0 };
        

      
        bool num_6[NUM_PIXELS] = {
        0, 1, 1, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 1, 1, 0, 
        0, 1, 0, 0, 0, 
        0, 1, 1, 1, 0 };
        

      
        bool num_7[NUM_PIXELS] = {
        0, 1, 0, 0, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 0, 0, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 1, 1, 0 };
        
      
        bool num_8[NUM_PIXELS] = {
        0, 1, 1, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 1, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 1, 1, 0 };
        

      
        bool num_9[NUM_PIXELS] = {
        0, 1, 1, 1, 0, 
        0, 0, 0, 1, 0, 
        0, 1, 1, 1, 0, 
        0, 1, 0, 1, 0, 
        0, 1, 1, 1, 0 };
        


int main()
{

  PIO pio = pio0;
  int sm = 0;
  uint offset = pio_add_program(pio, &ws2812_program);

  ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);
  stdio_init_all();
  gpio_init(LED_GREEN);              
  gpio_set_dir(LED_GREEN, GPIO_OUT); 
  gpio_init(LED_BLUE);              
  gpio_set_dir(LED_BLUE, GPIO_OUT); 

  gpio_init(button_0);
  gpio_set_dir(button_0, GPIO_IN); 
  gpio_pull_up(button_0); 

  gpio_init(button_1);
  gpio_set_dir(button_1, GPIO_IN); 
  gpio_pull_up(button_1); 

  gpio_set_irq_enabled_with_callback(button_0, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  gpio_set_irq_enabled_with_callback(button_1, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);


  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA); // Pull up the data line
  gpio_pull_up(I2C_SCL); // Pull up the clock line
  
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd); // Configura o display
  ssd1306_send_data(&ssd); // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  bool cor = true;
  while (true)
  {
    scanf(" %c",&c); //recebe o caractere desejado
    
   
    //caso seja 0-9 , desenha na matriz
    switch(c){


      case '0' :
       
        set_one_led(led_r, led_g, led_b, num_0);
      break;

      case '1' :
       
        set_one_led(led_r, led_g, led_b, num_1);
      break;

      case '2' :
       
        set_one_led(led_r, led_g, led_b, num_2);
      break;

      case '3' :
       
        set_one_led(led_r, led_g, led_b, num_3);
      break;

       case '4' :
       set_one_led(led_r, led_g, led_b, num_4);
      break;

      case '5' :
       
        set_one_led(led_r, led_g, led_b, num_5);
      break;

      case '6' :
       
        set_one_led(led_r, led_g, led_b, num_6);
      break;

      case '7' :
       
        set_one_led(led_r, led_g, led_b, num_7);
      break;

      case '8' :
       
        set_one_led(led_r, led_g, led_b, num_8);
      break;

      case '9' :
       
        set_one_led(led_r, led_g, led_b, num_9);
      break;

      default :
      set_one_led(led_r, led_g, led_b, desliga_led);
      

     
  } 


    cor = !cor;
    // Atualiza o conteúdo do display com animações
    ssd1306_fill(&ssd, !cor); // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_draw_char(&ssd, c, 8, 10);// escreve o caractere no display
    ssd1306_send_data(&ssd); // Atualiza o display

    sleep_ms(1000);
  }
}

void gpio_irq_handler(uint gpio, uint32_t events)
{
   // Obtém o tempo atual em microssegundos
   uint32_t current_time = to_us_since_boot(get_absolute_time());
   
   // Verifica se passou tempo suficiente desde o último evento
   if (current_time - last_time > 400000) // 400 ms de debouncing
   {
       last_time = current_time; // Atualiza o tempo do último evento
       
       

       if(gpio == button_0){
         gpio_put(LED_GREEN,!gpio_get(LED_GREEN));
         if(gpio_get(LED_GREEN))
         {
          
          ssd1306_draw_string(&ssd, "LED green ON ", 8, 30); //informa que o led foi ligado através de uma msg no display
          printf("LED verde ligado");                        //informa que o led foi ligado através de uma msg no serial
          
        }
         else
         {
          
          ssd1306_draw_string(&ssd, "LED green OFF", 8, 30); //informa que o led foi desligado através de uma msg no display
          printf("led verde desligado");                     //informa que o led foi desligado através de uma msg no serial
         
        }
       }

       else if (gpio == button_1){
        gpio_put(LED_BLUE,!gpio_get(LED_BLUE));
        if(gpio_get(LED_BLUE))
         {
          
          ssd1306_draw_string(&ssd, "LED blue ON ", 8, 48); //informa que o led foi ligado através de uma msg no display
          printf("led azul ligado");                        //informa que o led foi ligado através de uma msg no serial
          
        }
         else
         {
          
          ssd1306_draw_string(&ssd, "LED blue OFF", 8, 48); //informa que o led foi desligado através de uma msg no display
          printf("led azul desligado");                     //informa que o led foi desligado através de uma msg no serial
          
        }
       }
       ssd1306_send_data(&ssd);

         
                                                
      
   }



}