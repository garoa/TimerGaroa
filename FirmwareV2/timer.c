// Codigo de Relogio Regressivo feito para o Fnords & Furiosos 
// 
// Alterei algumas coisas do codigo original do DQ para fazer a funcao de contador 
// para as Lightning Talks. -- AlVecchio
//
// Alterado para compilar direto com o avr-gcc, fora da IDE Arduino
//
//
//
// (C) 2012-2019, Daniel Quadros
//
// ----------------------------------------------------------------------------
//   "THE BEER-WARE LICENSE" (Revision 42):
//   <dqsoft.blogspot@gmail.com> wrote this file.  As long as you retain this 
//   notice you can do whatever you want with this stuff. If we meet some day, 
//   and you think this stuff is worth it, you can buy me a beer in return.
//      Daniel Quadros
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "jymcu3208.h"
#include "ht1632c.h"
#include "fonte.h"

#define FALSE 0
#define TRUE  1

// Buzzer
#define BUZ_DDR     DDRC
#define BUZ_PORT    PORTC
#define BUZZER      _BV(PC2)

// Variaveis

static int fimdotempo = 0;

static uint8_t tempo, controle_tempo = 0, run = 0;
static uint16_t temporelogio, tempototal = 300;

static volatile uint8_t debounce = 0;

// Rotinas
void setup(void);
void loop(void);
static void relogio (void);
static void pontos (uint8_t estadoponto); // exibe os dois pontos entre minutos e segundos
static void boom (uint8_t leds);
static void display (uint8_t x, uint8_t d);
static void tempo_init (void);

// Programa principal
int main(void)
{
    setup();
    for (;;)
        loop();
}

// rotina inicial, executada apenas uma vez no arduino
void setup() {
    ht1632c_init();             // inicia o controlador do display
    ht1632c_send_screen ();     // limpa o display
    tempo_init();               // inicia a contagem de tempo

    // Liga pullup das teclas
    TEC_DDR &= ~(TEC_KEY1|TEC_KEY2|TEC_KEY3);
    TEC_PORT |= TEC_KEY1|TEC_KEY2|TEC_KEY3;

    // Inicia buzzer
    BUZ_DDR |= BUZZER;
    BUZ_PORT &= ~BUZZER;

    temporelogio = tempototal;
    relogio(); // rotina que exibe o relogio
    pontos(1); // liga os dois pontos entre minutos e segundos
}

// rotina executada em loop infinito
void loop() {
    static uint8_t tecant = 0;
    static uint8_t apertou = FALSE;
    uint8_t tec;
    
    // Tratamento das teclas
    tec = TEC_PIN & (TEC_KEY1 | TEC_KEY2 | TEC_KEY3);
    tec ^= (TEC_KEY1 | TEC_KEY2 | TEC_KEY3);
    if (tec != tecant)
    {
        debounce = 2;
        tecant = tec;
    }
    else if (debounce == 0)
    {
        // mudança estável
        if (apertou)
        {
            // esperando soltar
            if (tec == 0)
                apertou = FALSE;
        }
        else if (tec != 0)
        {
            // apertou uma tecla (ou várias)
            apertou = TRUE;
            // trata a tecla
            if (tec & TEC_KEY1)
            {
                // Reset
                if (run == 0 && temporelogio != tempototal)             
                {
                    temporelogio = tempototal; 
                    boom(0); 
                    relogio();
                }
                else if (run == 0 && tempototal == 900) 
                {
                    // Alterna tempo para 300 segundos (5 min)                    
                    tempototal = 300; 
                    temporelogio = tempototal; 
                    relogio();
                }
                else if (run == 0 && tempototal == 300) 
                { 
                     // Alterna tempo para 900 segundos (15 minutos)
                    tempototal = 900; 
                    temporelogio = tempototal; 
                    relogio();
                } 
                pontos(1);
            }
            else if (tec & TEC_KEY2)
            {
                // Stop
                if (run == 1) 
                { 
                    run = 0; 
                    relogio();
                }
            }
            else if (tec & TEC_KEY3)
            {
                // Start
                if (run == 0) 
                {
                    run = 1; 
                }
            }
       }
    }
 
}


// Mostra o digito d a partir da posicao x
static void display (uint8_t x, uint8_t d)
{
    uint8_t i, y, mask, gc;
    
    for (y = 0; y < 7; y++)
    {
        gc = pgm_read_byte(&(digitos[d][y]));
        mask = 0x10;
        for (i = 0; i < 5; i++)
        {
            ht1632c_setLED (x+i, y, gc & mask);
            mask = mask >> 1;
        }
    }
}


// Atualiza o relogio no display
static void relogio (void)
{
    if (temporelogio == fimdotempo)
    {
      run = 0; // desliga contador do tempo
      boom (1); // liga o beep e zoa o display
    }
  
  tempo = (( temporelogio / 600 ));
  display (2, tempo);  // exibe a dezena dos minutos
  tempo = (( temporelogio / 60) % 10);
  display (9, tempo); // exibe a unidade dos minutos
  
  tempo = (( temporelogio % 60 ) / 10);
  display (20, tempo); // exibe a dezena dos segundos
  tempo = ( temporelogio % 10 );
  display (27, tempo); // exibe a unidade dos segundos
}

// exibe os dois pontos entre minutos e segundos
static void pontos (uint8_t estadoponto) 
{
      ht1632c_setLED (17, 1, estadoponto);
      ht1632c_setLED (16, 1, estadoponto);
      ht1632c_setLED (17, 2, estadoponto);
      ht1632c_setLED (16, 2, estadoponto);
      ht1632c_setLED (17, 4, estadoponto);
      ht1632c_setLED (16, 4, estadoponto);
      ht1632c_setLED (17, 5, estadoponto);
      ht1632c_setLED (16, 5, estadoponto);
      
      /*
      if (temporelogio < 5) // beep para os ultimos x segundos
      {
        digitalWrite (Buzzer, estadoponto);
      }
      //*/
      
}


// Inicia a contagem de tempo
static void tempo_init (void)
{
  ASSR |= (1<<AS2);     // timer2 async from external quartz
  TCCR2 = 0b00000011;   // normal,off,/32; 32768Hz/256/32 = 4 Hz
  TIMSK |= (1<<TOIE2);  // enable timer2 overflow int
  sei();                // enable interrupts
}

// Interrupcao do Timer2
ISR(TIMER2_OVF_vect) 
{  
  ++ controle_tempo;
  if (controle_tempo > 3) // entra a cada segundo porque a frequencia eh 4Hz   
  {
    if (run == 1)
    {
      temporelogio--; // decresce a cada segundo
      relogio();
      pontos(1); // liga os 2 pontos quando o segundo muda
    }
    controle_tempo = 0;
  } 
  
  if (controle_tempo == 2 && run == 1) // desliga os 2 pontos quando esta decrescendo e na metade do segundo
  {
    pontos(0);
  }
  
  if (debounce)
  {
      debounce--;
  }
}

// "Explosão" da bomb
static void boom (uint8_t leds)
{
  uint8_t x, y;
    
  for (y = 0; y < 8; y++)
    for (x = 0; x < 32; x++)
      ht1632c_setLED (x, y, leds);
  if (leds) 
  {
    BUZ_PORT |= BUZZER;
  }
  else
  {
    BUZ_PORT &= ~BUZZER;
  }
}

