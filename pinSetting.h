/* 
 * File:   pinSetting.h
 * Author: urkwtky
 *
 * Created on 2012/11/01, 23:01
 */

#ifndef PINSETTING_H
#define	PINSETTING_H

/** Switches **************************************************
 SW1 - RC3
 SW2 - RC6
 SW3 - RC7
 SW4 - RB7
**************************************************************/
#define TRISB_SW 0B10000000
#define TRISC_SW 0B11001000

#define PIN_SW1 PORTCbits.RC3
#define PIN_SW2 PORTCbits.RC6
#define PIN_SW3 PORTCbits.RC7
#define PIN_SW4 PORTBbits.RB7

/** LEDs *****************************************************
 LED1 - RC5
 LED2 - RC4
 LED3 - RB5
 LED4 - RB6
**************************************************************/
#define TRISB_LED 0B10011111
#define TRISC_LED 0B11001111


#define PIN_LED1 PORTCbits.RC5
#define PIN_LED2 PORTCbits.RC4
#define PIN_LED3 PORTBbits.RB5
#define PIN_LED4 PORTBbits.RB6

/** POTs *****************************************************
 POT1 - RC1/AN5
 POT2 - RC0/AN4
 POT3 - RC2/AN6
 POT4 - RB4/AN10
**************************************************************/
#define TRISB_POT 0B00010000
#define TRISC_POT 0B00000111

#define NO_POT1 5
#define NO_POT2 4
#define NO_POT3 6
#define NO_POT4 10

#endif	/* PINSETTING_H */

