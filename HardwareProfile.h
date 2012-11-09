/********************************************************************
 FileName:      HardwareProfile.h
 Dependencies:  See INCLUDES section
 Processor:     PIC18, PIC24, dsPIC33, or PIC32 USB Microcontrollers
 Hardware:      This demo is natively intended to be used on Microchip USB demo
                boards supported by the MCHPFSUSB stack.  See release notes for
                support matrix.  The firmware may be modified for use on
                other USB platforms by editing this file (HardwareProfile.h)
                and adding a new hardware specific 
                HardwareProfile - [platform name].h file.
 Compiler:      Microchip C18 (for PIC18), C30 (for PIC24/dsPIC33), or C32 (for PIC32)
 Company:       Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PIC® Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

 ********************************************************************/

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

/*******************************************************************/
/******** USB stack hardware selection options *********************/
/*******************************************************************/
//This section is the set of definitions required by the MCHPFSUSB
//  framework.  These definitions tell the firmware what mode it is
//  running in, and where it can find the results to some information
//  that the stack needs.
//These definitions are required by every application developed with
//  this revision of the MCHPFSUSB framework.  Please review each
//  option carefully and determine which options are desired/required
//  for your application.

//#define USE_SELF_POWER_SENSE_IO
#define tris_self_power     TRISAbits.TRISA2    // Input
#if defined(USE_SELF_POWER_SENSE_IO)
#define self_power          PORTAbits.RA2
#else
#define self_power          1
#endif

//#define USE_USB_BUS_SENSE_IO
#define tris_usb_bus_sense  TRISAbits.TRISA1    // Input
#if defined(USE_USB_BUS_SENSE_IO)
#define USB_BUS_SENSE       PORTAbits.RA1
#else
#define USB_BUS_SENSE       1
#endif

/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/******** Application specific definitions *************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/

//Uncomment the following line to make the output HEX of this
//  project work with the HID Bootloader
#define PROGRAMMABLE_WITH_USB_HID_BOOTLOADER

#define CLOCK_FREQ 48000000
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

#endif  //HARDWARE_PROFILE_H
