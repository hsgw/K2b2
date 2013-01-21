/********************************************************************
 Dm9Records-k2b2/k4b4 midi controller firmware
 Copyright (c) 2012, hsgw(Takuya Urakawa), http://dm9records.com

 These files are distributed under MIT licence.
 http://opensource.org/licenses/mit-license.php
********************************************************************/

#ifndef MAIN_H
#define	MAIN_H

/** DEFINES ********************************************************/
//if use k4b4 uncomment following line
//#define k4b4

#define SW_DEBOUNCE 1

#ifdef k4b4
#define POT_NUM 4
#define SW_NUM 4
#else
#define POT_NUM 2
#define SW_NUM 2
#endif

/** INCLUDES *******************************************************/
#include "USB/usb.h"
#include "USB/usb_function_midi.h"
#include "HardwareProfile.h"

/** CONFIGURATION **************************************************/
#pragma config CPUDIV = NOCLKDIV
#pragma config USBDIV = OFF
#pragma config FOSC   = HS
#pragma config PLLEN  = ON
#pragma config FCMEN  = OFF
#pragma config IESO   = OFF
#pragma config PWRTEN = OFF
#pragma config BOREN  = OFF
#pragma config BORV   = 30
#pragma config WDTEN  = OFF
#pragma config WDTPS  = 32768
#pragma config MCLRE  = OFF
#pragma config HFOFST = OFF
#pragma config STVREN = ON
#pragma config LVP    = OFF
#pragma config XINST  = OFF
#pragma config BBSIZ  = OFF
#pragma config CP0    = OFF
#pragma config CP1    = OFF
#pragma config CPB    = OFF
#pragma config WRT0   = OFF
#pragma config WRT1   = OFF
#pragma config WRTB   = OFF
#pragma config WRTC   = OFF
#pragma config EBTR0  = OFF
#pragma config EBTR1  = OFF
#pragma config EBTRB  = OFF



/** VARIABLES ******************************************************/
#pragma udata
#pragma udata usbram2

#define RX_BUFFER_ADDRESS_TAG
#define TX_BUFFER_ADDRESS_TAG
#define MIDI_EVENT_ADDRESS_TAG

#ifdef k4b4
#define POT_NUM 4
#else
#define POT_NUM 2
#endif

unsigned char ReceivedDataBuffer[64] RX_BUFFER_ADDRESS_TAG;
unsigned char ToSendDataBuffer[64] TX_BUFFER_ADDRESS_TAG;
USB_AUDIO_MIDI_EVENT_PACKET midiData MIDI_EVENT_ADDRESS_TAG;

USB_HANDLE USBTxHandle = 0;
USB_HANDLE USBRxHandle = 0;

char swDebounceCount[4];
unsigned char lastSwState[4];
unsigned char swState[4];
unsigned char swChanged[4];

unsigned char potValue[4];
unsigned char lastPotValue[4];

const char no_pot[4] = {NO_POT1, NO_POT2, NO_POT3, NO_POT4};

const unsigned char ccNo[4] = {10,11,12,13};
const unsigned char noteNo[4] = {10,11,12,13};

/** PRIVATE PROTOTYPES *********************************************/
void init(void);

//switch function
void updateSw(void);
void sendMidiNote(void);

//LED function
void led(unsigned char num, unsigned char state);

//adc function
unsigned char readAdc(unsigned char num);
void updatePot(void);
void sendMidiCC(void);

void delayUs(long us);

//interrupt function
void YourHighPriorityISRCode();
void YourLowPriorityISRCode();

//USB function
void USBCBSendResume(void);

#endif	/* MAIN_H */

