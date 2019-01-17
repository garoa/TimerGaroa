// Definições para o display JY-MCU 3208
// Baseado em exemplos de DrJones e Rick Shear
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

#ifndef _JYMCU3208_H
#define _JYMCU3208_H

// Clock da CPU (clock internio de 8MHz dividido por 8)
#define CLK_CPU 1000000 // 1.0 MHz

// Conexões do controlador HT1632C
// Três sinais: CS, RD, WR e DT(DATA)
#define HT_CS_DDR   DDRB
#define HT_WR_DDR   DDRB
#define HT_DT_DDR   DDRB

#define HT_CS_PORT  PORTB
#define HT_WR_PORT  PORTB
#define HT_DT_PORT  PORTB

#define HT_CS_BIT   _BV(PB3)
#define HT_WR_BIT   _BV(PB4)
#define HT_DT_BIT   _BV(PB5)

// Conexão das Teclas
#define TEC_DDR     DDRD
#define TEC_PIN     PIND
#define TEC_PORT    PORTD
#define TEC_KEY1    _BV(PD7)
#define TEC_KEY2    _BV(PD6)
#define TEC_KEY3    _BV(PD5)

#endif

