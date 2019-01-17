// Rotinas para o controlador HT1632C usado no display JY-MCU 3208
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

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>

#include "jymcu3208.h"
#include "ht1632c.h"

// Comandos do controlador HT1632C
// Os comandos possuem 8 bits e devem ser precedidos pelos 3 bits de ID e seguidos de 
// um bit adicional (0 ou 1, tanto faz)
#define HT_ID_CMD     0b100
#define HT_STARTSYS   0b00000001   // start system oscillator
#define HT_STOPSYS    0b00000000   // stop sytem oscillator and LED duty
#define HT_SETCLOCK   0b00011000   // set clock to master with internal RC
#define HT_SETLAYOUT  0b00100000   // N-MOS open-drain output and 32 ROW * 8 COM
#define HT_LEDON      0b00000011   // start LEDs
#define HT_LEDOFF     0b00000010   // stop LEDs
#define HT_SETBRIGHT  0b10100000   // set brightness 0b1010xxxx  xxxx = brightness
#define HT_BLINKON    0b00001001   // blinking on
#define HT_BLINKOFF   0b00001000   // blinking off

// Escrita de dado no controlador HT1632C
// Enviar 3 bits de ID, 7 bits do endereÔøΩo inicial e os 4 bits de dados 
// 101-aaaaaaa-dddd-dddd-dddd-dddd-dddd-...
#define HT_ID_WRITE   0b101

// Copia da Ram do controlador
// Usa apenas os 4 bits menos significativos de cada byte
// 
volatile uint8_t ht1632c_shadowram [(HT_COLS*HT_ROWS)/4];

// Comandos de iniciacao do controlador
static uint8_t cmd_init[] =
{
    HT_STOPSYS, HT_SETLAYOUT, HT_SETCLOCK,
    HT_STARTSYS, HT_LEDON, HT_SETBRIGHT | 3,
    HT_BLINKOFF
};

// Rotinas internas
static void ht1632c_send_commands (uint8_t *pCmds, int8_t nCmds);
static void ht1632c_send_command (uint8_t cmd);
static void ht1632c_send (uint8_t valor, int8_t nBits);

// Inicia o controlador
void ht1632c_init ()
{
    // Inicia as direÔøΩoes dos pinos de conexÔøΩo
    HT_CS_DDR |= HT_CS_BIT;
    HT_WR_DDR |= HT_WR_BIT;
    HT_DT_DDR |= HT_DT_BIT;
    
    // Default para o CS e WR ÔøΩ alto (inativo)
    HT_CS_PORT |= HT_CS_BIT;
    HT_WR_PORT |= HT_WR_BIT;
    HT_DT_PORT |= HT_DT_BIT;
    
    // Efetua a configuraÔøΩÔøΩo
    ht1632c_send_commands (cmd_init, sizeof(cmd_init));
}

// Acende ou apaga o LED na posiÔøΩÔøΩo x,y
// 0 <= x <= 31  0 <= y <= 7
void ht1632c_setLED (uint8_t x, uint8_t y, uint8_t valor)
{
    uint8_t addr, mask;
    
    mask = 1 << (3 - (x & 3));
    addr = ((x & ~7) << 1) + (y << 1);
    if (x & 4)
        addr++;
    if (valor)
        ht1632c_shadowram[addr] |= mask;
    else
        ht1632c_shadowram[addr] &= ~mask;
    
    HT_CS_PORT &= ~HT_CS_BIT;
    ht1632c_send (HT_ID_WRITE, 3);
    ht1632c_send (addr, 7);
    ht1632c_send (ht1632c_shadowram[addr], 4);
    HT_CS_PORT |= HT_CS_BIT;
}

// Atualiza a memoria do controlador com o conteudo
// da shadow mempory
void ht1632c_send_screen ()
{
    uint8_t addr;
    
    HT_CS_PORT &= ~HT_CS_BIT;       // seleciona o controlador
    ht1632c_send (HT_ID_WRITE, 3);
    ht1632c_send (0, 7);            // endereÔøΩo inicial
    for(addr = 0; addr < (HT_COLS*HT_ROWS)/4; addr++)
    {
        ht1632c_send (ht1632c_shadowram[addr], 4);
    }
    HT_CS_PORT |= HT_CS_BIT;        // libera o controlador
}

// Envia uma sÔøΩrie de comandos ao controlador
static void ht1632c_send_commands (uint8_t *pCmds, int8_t nCmds)
{
    int8_t i;
    
    HT_CS_PORT &= ~HT_CS_BIT;       // seleciona o controlador
    ht1632c_send (HT_ID_CMD, 3);    // envia ID de comando
    for (i = 0; i < nCmds; i++)
    {
        ht1632c_send (pCmds[i], 8);
        ht1632c_send (0, 1);
    }
    HT_CS_PORT |= HT_CS_BIT;        // libera o controlador
}

// Envia um comando ao controlador
static void ht1632c_send_command (uint8_t cmd)
{
    HT_CS_PORT &= ~HT_CS_BIT;       // seleciona o controlador
    ht1632c_send (HT_ID_CMD, 3);    // envia ID de comando
    ht1632c_send (cmd, 8);
    ht1632c_send (0, 1);
    HT_CS_PORT |= HT_CS_BIT;        // libera o controlador
}

// Envia uma sequencia de bits ao controlador
static void ht1632c_send (uint8_t valor, int8_t nBits)
{
    int8_t i;
    uint8_t mask = 1 << (nBits-1);    // enviar mais significativo primeiro
    
    for (i = nBits; i > 0; i--)
    {
        HT_WR_PORT &= ~HT_WR_BIT;
        if (valor & mask)
            HT_DT_PORT |= HT_DT_BIT;
        else
            HT_DT_PORT &= ~HT_DT_BIT;
        HT_WR_PORT |= HT_WR_BIT;
        mask = mask >> 1;
    }
}

