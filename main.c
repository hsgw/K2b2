/********************************************************************
 Dm9Records-k2b2/k4b4 midi controller firmware
 Copyright (c) 2012, hsgw(Takuya Urakawa), http://dm9records.com

 These files are distributed under MIT licence.
 http://opensource.org/licenses/mit-license.php
********************************************************************/

#ifndef MAIN_C
#define MAIN_C

/** INCLUDES *******************************************************/
#include "main.h"

/** USB method check************************************************/
#if defined(USB_INTERRUPT)
    #warning "USB Interrupt ON"
#elif defined(USB_POLLING)
    #warning "USB Polling ON"
#else
    #error "NOT defined USB method(usb_descriptors.c)"
#endif

/** VECTOR REMAPPING ***********************************************/
#if defined(__18CXX)

#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
#warning "USB HID BOOTLOADER"
#define REMAPPED_RESET_VECTOR_ADDRESS			0x1000
#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1008
#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1018
#elif defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
#define REMAPPED_RESET_VECTOR_ADDRESS			0x800
#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x808
#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x818
#else
#warning "NOT BOOTLOADER"
#define REMAPPED_RESET_VECTOR_ADDRESS			0x00
#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x08
#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x18
#endif

#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
extern void _startup(void); // See c018i.c in your C18 compiler dir
#pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS

void _reset(void) {
    _asm goto _startup _endasm
}
#endif
#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS

void Remapped_High_ISR(void) {
    _asm goto YourHighPriorityISRCode _endasm
}
#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS

void Remapped_Low_ISR(void) {
    _asm goto YourLowPriorityISRCode _endasm
}

#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)

#pragma code HIGH_INTERRUPT_VECTOR = 0x08

void High_ISR(void) {
    _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
}
#pragma code LOW_INTERRUPT_VECTOR = 0x18

void Low_ISR(void) {
    _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
}
#endif	//end of "#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_LEGACY_CUSTOM_CLASS_BOOTLOADER)"

#pragma code

//These are your actual interrupt handling routines.
#pragma interrupt YourHighPriorityISRCode

void YourHighPriorityISRCode() {
#if defined(USB_INTERRUPT)
    USBDeviceTasks();
#endif

}
#pragma interruptlow YourLowPriorityISRCode

void YourLowPriorityISRCode() {

}
    
/** DECLARATIONS ***************************************************/
#pragma code

/** main **********************************************************/
void main(void) {
    init();

#if defined(USB_INTERRUPT)
    USBDeviceAttach();
#endif

    //main loop
    while (1) {
#if defined(USB_POLLING)
        USBDeviceTasks();
#endif
        if((USBDeviceState >= CONFIGURED_STATE) && (USBSuspendControl == 0)){
            updateSw();
            sendMidiNote();
            updatePot();
            sendMidiCC();
        }
    }//end while
}//end main


 /********************************************************************
  * initialize
 ********************************************************************/

void init(void) {
    unsigned char i;
    
    ADCON1 = 0x00; // REF VSS,VDD
    ADCON2 = 0b00101110;
    ANSEL  = 0b01110000;
    ANSELH = 0b00000100;
    ADCON0bits.ADON = 1;
    
    TRISB = (TRISB_SW | TRISB_POT) & TRISB_LED;
    TRISC = (TRISC_SW | TRISC_POT) & TRISC_LED;

    for(i=0;i<4;i++){
        swDebounceCount[i] = 0;
        lastSwState[i] = 0;
        swState[i] = 0;
        swChanged[i] = 0;
    }

#if defined(USE_USB_BUS_SENSE_IO)
    tris_usb_bus_sense = INPUT_PIN; // See HardwareProfile.h
#endif
#if defined(USE_SELF_POWER_SENSE_IO)
    tris_self_power = INPUT_PIN; // See HardwareProfile.h
#endif

    USBTxHandle = NULL;
    USBRxHandle = NULL;

    USBDeviceInit(); //usb_device.c.  Initializes USB module SFRs and firmware
    //variables to known states.
}//end init


 /********************************************************************
  *
  * Switch functions
  *
 ********************************************************************/

void updateSw(void){
    unsigned char i;
    unsigned char currentSwState[4];

    currentSwState[0] = PIN_SW1;
    currentSwState[1] = PIN_SW2;
    currentSwState[2] = PIN_SW3;
    currentSwState[3] = PIN_SW4;

    for(i=0;i<SW_NUM;i++){
        if(currentSwState[i] != swState[i]){
            if(currentSwState[i] != lastSwState[i]){
                swDebounceCount[i] = 0;
            }else{
                if(swDebounceCount[i] < SW_DEBOUNCE && ++swDebounceCount[i] == SW_DEBOUNCE){
                    swChanged[i] = 1;
                    swState[i] = currentSwState[i];
                }
            }
        }
        lastSwState[i] = currentSwState[i];
    }
}

// LED function
void led(unsigned char num, unsigned char state){

#ifdef k4b4
    switch(num){
        case 0:
            PIN_LED1 = state;
            break;
        case 1:
            PIN_LED2 = state;
            break;
        case 2:
            PIN_LED3 = state;
            break;
        case 3:
            PIN_LED4 = state;
            break;
        default:
            break;
    }
#else
    switch(num){
        case 0:
            PIN_LED1 = state;
            PIN_LED3 = state;
            break;
        case 1:
            PIN_LED2 = state;
            PIN_LED4 = state;
            break;
        default:
            break;
    }
#endif
}

// sendMidiNote ***********************************************************
void sendMidiNote(void){
    unsigned char i;
    for(i=0;i<SW_NUM;i++){
        if(swChanged[i]==1){
            if (!USBHandleBusy(USBTxHandle)) {
                swChanged[i] = 0;
                midiData.Val = 0;   //midiData init
                midiData.CableNumber = 0;

                if(swState[i]==0){
                    midiData.CodeIndexNumber = MIDI_CIN_NOTE_ON;
                    midiData.DATA_0 = 0x90; //Note on
                    led(i,1);
                }else{
                    midiData.CodeIndexNumber = MIDI_CIN_NOTE_OFF;
                    midiData.DATA_0 = 0x80; //Note off
                    led(i,0);
                }
                midiData.DATA_1 = noteNo[i]; //pitch
                midiData.DATA_2 = 0x7F; //velocity

                USBTxHandle = USBTxOnePacket(MIDI_EP, (BYTE*) & midiData, 4);
                //delayUs(20);
            }
        }
    }
}

 /********************************************************************
  * 
  * ADC functions
  * 
 ********************************************************************/

unsigned char readAdc(unsigned char num){
    ADCON0bits.CHS  = num;
    delayUs(20);
   //ADCON0bits.ADON = 1;
    ADCON0bits.GO   = 1;

    while(ADCON0bits.NOT_DONE);
    return ADRESH;
}

void updatePot(){
    char i;
    int tempValue = 0;
    for(i=0;i<32;i++){
        tempValue += (int)readAdc(NO_POT1);
    }
    //tempValue = tempValue >> 6;
    tempValue = tempValue >> 6;
    potValue[0] = (tempValue  + potValue[0]) >> 1;

    tempValue = 0;
    for(i=0;i<32;i++){
        tempValue += (int)readAdc(NO_POT2);
    }
    tempValue = tempValue >> 6;
    //potValue[1] = (lastPotValue[1] + tempValue) >> 1;
    potValue[1] = (tempValue  + potValue[1]) >> 1;

#ifdef k4b4
    tempValue = 0;
    for(i=0;i<32;i++){
        tempValue += (int)readAdc(NO_POT3);
    }
    potValue[2] = tempValue >> 6;

    tempValue = 0;
    for(i=0;i<32;i++){
        tempValue += (int)readAdc(NO_POT4);
    }
    potValue[3] = tempValue >> 6;
#endif
}

void sendMidiCC(void){
    char i;
    for(i=0;i<POT_NUM;i++){
        if(potValue[i]!=lastPotValue[i]){
            if (!USBHandleBusy(USBTxHandle)) {
                midiData.Val = 0;   //midiData init
                midiData.CableNumber = 0;
                midiData.CodeIndexNumber = MIDI_CIN_CONTROL_CHANGE;
                midiData.DATA_0 = 0xB0; //CC
                midiData.DATA_1 = ccNo[i]; //CC No
                midiData.DATA_2 = potValue[i]; //velocity

                USBTxHandle = USBTxOnePacket(MIDI_EP, (BYTE*) & midiData, 4);
                lastPotValue[i] = potValue[i];
                //delayUs(20);
            }
        }
    }
}

/************************************************************
 *
 * delay functions
 *
 ************************************************************/
void delayUs(long us){
    while(us>0){
        _asm
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
            nop
        _endasm
        us--;
    }
}

// ******************************************************************************************************
// ************** USB Callback Functions ****************************************************************
// ******************************************************************************************************
// The USB firmware stack will call the callback functions USBCBxxx() in response to certain USB related
// events.  For example, if the host PC is powering down, it will stop sending out Start of Frame (SOF)
// packets to your device.  In response to this, all USB devices are supposed to decrease their power
// consumption from the USB Vbus to <2.5mA* each.  The USB module detects this condition (which according
// to the USB specifications is 3+ms of no bus activity/SOF packets) and then calls the USBCBSuspend()
// function.  You should modify these callback functions to take appropriate actions for each of these
// conditions.  For example, in the USBCBSuspend(), you may wish to add code that will decrease power
// consumption from Vbus to <2.5mA (such as by clock switching, turning off LEDs, putting the
// microcontroller to sleep, etc.).  Then, in the USBCBWakeFromSuspend() function, you may then wish to
// add code that undoes the power saving things done in the USBCBSuspend() function.

// The USBCBSendResume() function is special, in that the USB stack will not automatically call this
// function.  This function is meant to be called from the application firmware instead.  See the
// additional comments near the function.

// Note *: The "usb_20.pdf" specs indicate 500uA or 2.5mA, depending upon device classification. However,
// the USB-IF has officially issued an ECN (engineering change notice) changing this to 2.5mA for all 
// devices.  Make sure to re-download the latest specifications to get all of the newest ECNs.

/******************************************************************************
 * Function:        void USBCBSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Call back that is invoked when a USB suspend is detected
 *
 * Note:            None
 *****************************************************************************/
void USBCBSuspend(void) {
    //Example power saving code.  Insert appropriate code here for the desired
    //application behavior.  If the microcontroller will be put to sleep, a
    //process similar to that shown below may be used:

    //ConfigureIOPinsForLowPower();
    //SaveStateOfAllInterruptEnableBits();
    //DisableAllInterruptEnableBits();
    //EnableOnlyTheInterruptsWhichWillBeUsedToWakeTheMicro();	//should enable at least USBActivityIF as a wake source
    //Sleep();
    //RestoreStateOfAllPreviouslySavedInterruptEnableBits();	//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.
    //RestoreIOPinsToNormal();									//Preferrably, this should be done in the USBCBWakeFromSuspend() function instead.

    //IMPORTANT NOTE: Do not clear the USBActivityIF (ACTVIF) bit here.  This bit is
    //cleared inside the usb_device.c file.  Clearing USBActivityIF here will cause
    //things to not work as intended.



}

/******************************************************************************
 * Function:        void USBCBWakeFromSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The host may put USB peripheral devices in low power
 *					suspend mode (by "sending" 3+ms of idle).  Once in suspend
 *					mode, the host may wake the device back up by sending non-
 *					idle state signalling.
 *
 *					This call back is invoked when a wakeup from USB suspend
 *					is detected.
 *
 * Note:            None
 *****************************************************************************/
void USBCBWakeFromSuspend(void) {
    // If clock switching or other power savings measures were taken when
    // executing the USBCBSuspend() function, now would be a good time to
    // switch back to normal full power run mode conditions.  The host allows
    // 10+ milliseconds of wakeup time, after which the device must be
    // fully back to normal, and capable of receiving and processing USB
    // packets.  In order to do this, the USB module must receive proper
    // clocking (IE: 48MHz clock must be available to SIE for full speed USB
    // operation).
    // Make sure the selected oscillator settings are consistent with USB
    // operation before returning from this function.
}

/********************************************************************
 * Function:        void USBCB_SOF_Handler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB host sends out a SOF packet to full-speed
 *                  devices every 1 ms. This interrupt may be useful
 *                  for isochronous pipes. End designers should
 *                  implement callback routine as necessary.
 *
 * Note:            None
 *******************************************************************/
void USBCB_SOF_Handler(void) {
    // No need to clear UIRbits.SOFIF to 0 here.
    // Callback caller is already doing that.
}

/*******************************************************************
 * Function:        void USBCBErrorHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The purpose of this callback is mainly for
 *                  debugging during development. Check UEIR to see
 *                  which error causes the interrupt.
 *
 * Note:            None
 *******************************************************************/
void USBCBErrorHandler(void) {
    // No need to clear UEIR to 0 here.
    // Callback caller is already doing that.

    // Typically, user firmware does not need to do anything special
    // if a USB error occurs.  For example, if the host sends an OUT
    // packet to your device, but the packet gets corrupted (ex:
    // because of a bad connection, or the user unplugs the
    // USB cable during the transmission) this will typically set
    // one or more USB error interrupt flags.  Nothing specific
    // needs to be done however, since the SIE will automatically
    // send a "NAK" packet to the host.  In response to this, the
    // host will normally retry to send the packet again, and no
    // data loss occurs.  The system will typically recover
    // automatically, without the need for application firmware
    // intervention.

    // Nevertheless, this callback function is provided, such as
    // for debugging purposes.
}

/*******************************************************************
 * Function:        void USBCBCheckOtherReq(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        When SETUP packets arrive from the host, some
 * 					firmware must process the request and respond
 *					appropriately to fulfill the request.  Some of
 *					the SETUP packets will be for standard
 *					USB "chapter 9" (as in, fulfilling chapter 9 of
 *					the official USB specifications) requests, while
 *					others may be specific to the USB device class
 *					that is being implemented.  For example, a HID
 *					class device needs to be able to respond to
 *					"GET REPORT" type of requests.  This
 *					is not a standard USB chapter 9 request, and
 *					therefore not handled by usb_device.c.  Instead
 *					this request should be handled by class specific
 *					firmware, such as that contained in usb_function_hid.c.
 *
 * Note:            None
 *******************************************************************/
void USBCBCheckOtherReq(void) {
}//end

/*******************************************************************
 * Function:        void USBCBStdSetDscHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USBCBStdSetDscHandler() callback function is
 *					called when a SETUP, bRequest: SET_DESCRIPTOR request
 *					arrives.  Typically SET_DESCRIPTOR requests are
 *					not used in most applications, and it is
 *					optional to support this type of request.
 *
 * Note:            None
 *******************************************************************/
void USBCBStdSetDscHandler(void) {
    // Must claim session ownership if supporting this request
}//end

/*******************************************************************
 * Function:        void USBCBInitEP(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when the device becomes
 *                  initialized, which occurs after the host sends a
 * 					SET_CONFIGURATION (wValue not = 0) request.  This
 *					callback function should initialize the endpoints
 *					for the device's usage according to the current
 *					configuration.
 *
 * Note:            None
 *******************************************************************/
void USBCBInitEP(void) {
    //enable the HID endpoint
    USBEnableEndpoint(MIDI_EP, USB_OUT_ENABLED | USB_IN_ENABLED | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);

    //Re-arm the OUT endpoint for the next packet
    USBRxHandle = USBRxOnePacket(MIDI_EP, (BYTE*) & ReceivedDataBuffer, 64);
}

/********************************************************************
 * Function:        void USBCBSendResume(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB specifications allow some types of USB
 * 					peripheral devices to wake up a host PC (such
 *					as if it is in a low power suspend to RAM state).
 *					This can be a very useful feature in some
 *					USB applications, such as an Infrared remote
 *					control	receiver.  If a user presses the "power"
 *					button on a remote control, it is nice that the
 *					IR receiver can detect this signalling, and then
 *					send a USB "command" to the PC to wake up.
 *					
 *					The USBCBSendResume() "callback" function is used
 *					to send this special USB signalling which wakes 
 *					up the PC.  This function may be called by
 *					application firmware to wake up the PC.  This
 *					function will only be able to wake up the host if
 *                  all of the below are true:
 *					
 *					1.  The USB driver used on the host PC supports
 *						the remote wakeup capability.
 *					2.  The USB configuration descriptor indicates
 *						the device is remote wakeup capable in the
 *						bmAttributes field.
 *					3.  The USB host PC is currently sleeping,
 *						and has previously sent your device a SET 
 *						FEATURE setup packet which "armed" the
 *						remote wakeup capability.   
 *
 *                  If the host has not armed the device to perform remote wakeup,
 *                  then this function will return without actually performing a
 *                  remote wakeup sequence.  This is the required behavior, 
 *                  as a USB device that has not been armed to perform remote 
 *                  wakeup must not drive remote wakeup signalling onto the bus;
 *                  doing so will cause USB compliance testing failure.
 *                  
 *					This callback should send a RESUME signal that
 *                  has the period of 1-15ms.
 *
 * Note:            This function does nothing and returns quickly, if the USB
 *                  bus and host are not in a suspended condition, or are 
 *                  otherwise not in a remote wakeup ready state.  Therefore, it
 *                  is safe to optionally call this function regularly, ex: 
 *                  anytime application stimulus occurs, as the function will
 *                  have no effect, until the bus really is in a state ready
 *                  to accept remote wakeup. 
 *
 *                  When this function executes, it may perform clock switching,
 *                  depending upon the application specific code in 
 *                  USBCBWakeFromSuspend().  This is needed, since the USB
 *                  bus will no longer be suspended by the time this function
 *                  returns.  Therefore, the USB module will need to be ready
 *                  to receive traffic from the host.
 *
 *                  The modifiable section in this routine may be changed
 *                  to meet the application needs. Current implementation
 *                  temporary blocks other functions from executing for a
 *                  period of ~3-15 ms depending on the core frequency.
 *
 *                  According to USB 2.0 specification section 7.1.7.7,
 *                  "The remote wakeup device must hold the resume signaling
 *                  for at least 1 ms but for no more than 15 ms."
 *                  The idea here is to use a delay counter loop, using a
 *                  common value that would work over a wide range of core
 *                  frequencies.
 *                  That value selected is 1800. See table below:
 *                  ==========================================================
 *                  Core Freq(MHz)      MIP         RESUME Signal Period (ms)
 *                  ==========================================================
 *                      48              12          1.05
 *                       4              1           12.6
 *                  ==========================================================
 *                  * These timing could be incorrect when using code
 *                    optimization or extended instruction mode,
 *                    or when having other interrupts enabled.
 *                    Make sure to verify using the MPLAB SIM's Stopwatch
 *                    and verify the actual signal on an oscilloscope.
 *******************************************************************/
void USBCBSendResume(void) {
    static WORD delay_count;

    //First verify that the host has armed us to perform remote wakeup.
    //It does this by sending a SET_FEATURE request to enable remote wakeup,
    //usually just before the host goes to standby mode (note: it will only
    //send this SET_FEATURE request if the configuration descriptor declares
    //the device as remote wakeup capable, AND, if the feature is enabled
    //on the host (ex: on Windows based hosts, in the device manager 
    //properties page for the USB device, power management tab, the 
    //"Allow this device to bring the computer out of standby." checkbox 
    //should be checked).
    if (USBGetRemoteWakeupStatus() == TRUE) {
        //Verify that the USB bus is in fact suspended, before we send
        //remote wakeup signalling.
        if (USBIsBusSuspended() == TRUE) {
            USBMaskInterrupts();

            //Clock switch to settings consistent with normal USB operation.
            USBCBWakeFromSuspend();
            USBSuspendControl = 0;
            USBBusIsSuspended = FALSE; //So we don't execute this code again,
            //until a new suspend condition is detected.

            //Section 7.1.7.7 of the USB 2.0 specifications indicates a USB
            //device must continuously see 5ms+ of idle on the bus, before it sends
            //remote wakeup signalling.  One way to be certain that this parameter
            //gets met, is to add a 2ms+ blocking delay here (2ms plus at 
            //least 3ms from bus idle to USBIsBusSuspended() == TRUE, yeilds
            //5ms+ total delay since start of idle).
            delay_count = 3600U;
            do {
                delay_count--;
            } while (delay_count);

            //Now drive the resume K-state signalling onto the USB bus.
            USBResumeControl = 1; // Start RESUME signaling
            delay_count = 1800U; // Set RESUME line for 1-13 ms
            do {
                delay_count--;
            } while (delay_count);
            USBResumeControl = 0; //Finished driving resume signalling

            USBUnmaskInterrupts();
        }
    }
}

/*******************************************************************
 * Function:        BOOL USER_USB_CALLBACK_EVENT_HANDLER(
 *                        int event, void *pdata, WORD size)
 *
 * PreCondition:    None
 *
 * Input:           int event - the type of event
 *                  void *pdata - pointer to the event data
 *                  WORD size - size of the event data
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called from the USB stack to
 *                  notify a user application that a USB event
 *                  occured.  This callback is in interrupt context
 *                  when the USB_INTERRUPT option is selected.
 *
 * Note:            None
 *******************************************************************/
BOOL USER_USB_CALLBACK_EVENT_HANDLER(int event, void *pdata, WORD size) {
    switch (event) {
        case EVENT_TRANSFER:
            //Add application specific callback task or callback function here if desired.
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;
        case EVENT_CONFIGURED:
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER_TERMINATED:
            //Add application specific callback task or callback function here if desired.
            //The EVENT_TRANSFER_TERMINATED event occurs when the host performs a CLEAR
            //FEATURE (endpoint halt) request on an application endpoint which was 
            //previously armed (UOWN was = 1).  Here would be a good place to:
            //1.  Determine which endpoint the transaction that just got terminated was 
            //      on, by checking the handle value in the *pdata.
            //2.  Re-arm the endpoint if desired (typically would be the case for OUT 
            //      endpoints).
            break;
        default:
            break;
    }
    return TRUE;
}

/** EOF main.c *************************************************/
#endif
