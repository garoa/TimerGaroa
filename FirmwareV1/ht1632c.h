// DefiniÔøΩÔøΩes para rotinas de acesso ao controlador HT1632C usado no 
// display JY-MCU 3208.
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

#ifndef _HT1632C_H
#define _HT1632C_H

#define HT_ROWS 8
#define HT_COLS 32

// Copia da Ram do controlador
extern volatile uint8_t ht1632c_shadowram [(HT_COLS*HT_ROWS)/4];

// Rotinas
void ht1632c_init (void);
void ht1632c_setLED (uint8_t x, uint8_t y, uint8_t valor);
void ht1632c_send_screen (void);

#endif

