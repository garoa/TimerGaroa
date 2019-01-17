GCCFLAGS=-g -Os -Wall -mcall-prologues -mmcu=atmega8
LINKFLAGS=-Wl,-Map,${PROGNAME}.map
AVRDUDEFLAGS=-c usbtiny -p atmega8
LINKOBJECTS=ht1632c.o

PROGNAME=Timer

all:	${PROGNAME}.hex

program: ${PROGNAME}-upload

${PROGNAME}.hex:	${PROGNAME}.c ht1632c.c
	avr-gcc ${GCCFLAGS} -o ht1632c.o -c ht1632c.c
	avr-gcc ${GCCFLAGS} ${LINKFLAGS} -o ${PROGNAME}.o ${PROGNAME}.c ${LINKOBJECTS}
	avr-objcopy -O ihex ${PROGNAME}.o ${PROGNAME}.hex
	
${PROGNAME}-upload:	${PROGNAME}.hex
	avrdude ${AVRDUDEFLAGS} -U flash:w:${PROGNAME}.hex:a
