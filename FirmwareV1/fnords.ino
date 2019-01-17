// Codigo de Relogio Regressivo feito para o Fnords & Furiosos 
// 
// Alterei algumas coisas do codigo original do DQ para fazer a funcao de contador 
// para as Lightning Talks.
//
// AleVecchio
//
//
//
// (C) 2012, Daniel Quadros
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

#define Buzzer 16 //definir pino do Buzzer


// Variaveis

const int fimdotempo = 0;


uint8_t tempo, controle_tempo = 0, run = 0;
uint16_t temporelogio, tempototal = 300;

uint8_t buttonState[2]; // estado atual do botao
uint8_t lastbuttonState[2]; // ultimo estado do botao
const int buttonPin[] = {7, 6, 5}; // PINS dos botoes

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers



// Rotinas
static void display (uint8_t x, uint8_t d);
static void tempo_init (void);

// rotina inicial, executada apenas uma vez no arduino
void setup() {
  temporelogio = tempototal;

  relogio(); // rotina que exibe o relogio
  pontos(1); // liga os dois pontos entre minutos e segundos

  ht1632c_init();             // inicia o controlador do display
  ht1632c_send_screen ();     // limpa o display
  tempo_init();               // inicia a contagem de tempo

  pinMode (buttonPin[0], INPUT_PULLUP); // botao superior
  pinMode (buttonPin[1], INPUT_PULLUP); // botao do meio
  pinMode (buttonPin[2], INPUT_PULLUP); // botao inferior
  pinMode (Buzzer, OUTPUT); // beep
}

// rotina executada em loop infinito
void loop() {

for (uint8_t i = 0; i < 3; i++) // for para escanear os 3 botoes
{
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin[i]);
  
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastbuttonState[i]) 
  {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) 
  {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState[i]) 
    {
      buttonState[i] = reading;

      // executa caso algum botao seja pressionado - esta em LOW porque usei PULLUP interno do atmega8
      if (buttonState[i] == LOW) 
      {
        switch(i)
        {
          case 0: // Botao superior = RESET
            if (run == 0 && temporelogio != tempototal)             
            {temporelogio = tempototal; boom(0); relogio();} // boom(0) desliga beep e reseta display
            else 
              if (run == 0 && tempototal == 900) // Alterna tempo para 300 segundos (5 min)
              { tempototal = 300; temporelogio = tempototal; relogio();}
              else
                if (run == 0 && tempototal == 300)  // Alterna tempo para 900 segundos (15 minutos)
                { tempototal = 900; temporelogio = tempototal; relogio();} 
            pontos(1);
            break;
            
          case 1: // Botao do meio = STOP
            if (run == 1) 
              { run = 0; relogio();}
            break; 
          
          case 2: // Botao inferior = START
            if (run == 0) 
              {run = 1; }
            break;
        }
      }
    }
  }

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastbuttonState[i] = reading;
} // fim do for
  
 
} // fim do void



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



void relogio (void)
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


void pontos (uint8_t estadoponto) // exibe os dois pontos entre minutos e segundos
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
}

void boom (uint8_t leds)
{
  for (uint8_t y = 0; y < 8; y++)
    for (uint8_t x = 0; x < 32; x++)
      ht1632c_setLED (x, y, leds);
  digitalWrite (Buzzer, leds);
}

