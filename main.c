#include <stdio.h>
#include <htc.h>
#include "usart.h"

/* A simple demonstration of serial communications which
 * incorporates the on-board hardware USART of the Microchip
 * PIC16Fxxx series of devices. */
__CONFIG(0x3ff4);

//function prototypes

//unsigned int uintDelayCount = 0;
//bit tog = 0;
//
//void main(void){
//	unsigned char input;
//	INTCON=0;	// purpose of disabling the interrupts.
//	init_comms();	// set up the USART - settings defined in usart.h
//
//        TRISCbits.TRISC1 = 0x00;
//
//	// Output a message to prompt the user for a keypress
//	//printf("\rPress a key and I will echo it back:\n");
//
//	while(1){
//		input = getch();	// read a response from the user
//
//                tog = !tog;
//                PORTCbits.RC1 = tog;
//
//		//printf("I detected [%02x]\n\r",input);	// echo it back
//	}
//}
char dataIn = 'S'; //Character/Data coming from the phone.
//#define L1 RC0;     //Pin that controls the car's Forward-Back motor.
//#define L2 RC1;       //Pin that controls the car's Left-Right motor.
//#define L3 RC2;  //Pin that enables/disables Left-Right motor.
//#define L4 RC3;  //Pin that enables/disables Forward-Back motor.
//int pinLeftRightSpeed = 3;    //Pin that sets the speed for the Left-Right motor.
//int pinForwardBackSpeed = 11;  //Pin that sets the speed for the Forward-Back motor.
//int pinfrontLights = 4;    //Pin that activates the Front lights.
//int pinbackLights = 7;    //Pin that activates the Back lights.
char determinant; //Used in the check function, stores the character received from the phone.
char det; //Used in the loop function, stores the character received from the phone.
int velocity = 0; //Stores the speed based on the character sent by the phone.
unsigned char portc_shadow = 0x00;

char rxchar, i = 0; // Variable for storing the data from UART and array counter
unsigned char commandBuffer[6];
unsigned char packetRecieved = 0;
unsigned char packetCorrupt = 0;

static void interrupt
isr()
{
    if (RCIF) // test the interrupt for uart rx
    {
        rxchar = getch(); //
        if (packetCorrupt)
        {
            //If we got a corrupt packet, we need to wait for the packet start character.
            if (rxchar != 0x02)
            {
                return;
            }
            else
            {
                packetCorrupt = 0;
            }
        }
        commandBuffer[i] = rxchar;
        i++;
        if (i == 6) //We got a packet
        {
            packetRecieved = 1;
            i = 0;
        }
    }
}

void setup()
{
    //*************NOTE: If using Bluetooth Mate Silver use 115200 btu
    //                   If using MDFLY Bluetooth Module use 9600 btu
    //INTCON = 0; // purpose of disabling the interrupts.

    //Setup interrupts
    INTCONbits.GIE = 1; //Enable global interrupt
    INTCONbits.PEIE = 1; //Enable Periferal interrupts
    PIE1bits.RCIE = 1; //enable USART receive interrupt.

    init_comms(); // set up the USART - settings defined in usart.h

    //Setup PWM
    //Set PWM pins to outputs.
    TRISCbits.TRISC5 = 0;
    //Set to dual PWM
    CCP1CON = 0b00001100;
    //Set PWM period
    //PR2 = 0xFF; (default)





    TRISCbits.TRISC0 = 0; // setting c0 to be output
    TRISCbits.TRISC1 = 0; // c1, ditto
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC3 = 0;

    //pinMode(pinForwardBack, OUTPUT);
    //pinMode(pinLeftRight, OUTPUT);
    //pinMode(pinBrakeLeftRight, OUTPUT);
    //pinMode(pinBrakeForwardBack, OUTPUT);
    //pinMode(pinLeftRightSpeed , OUTPUT);
    //pinMode(pinForwardBackSpeed , OUTPUT);
    //pinMode(pinfrontLights , OUTPUT);
    //pinMode(pinbackLights , OUTPUT);
}
//
//bit tog = 0;
//unsigned char portc_shadow = 0x00;
//
//void main()
//{
//    INTCON = 0;
//    //TRISC = 0x00;
//    TRISCbits.TRISC0 = 0;
//    TRISCbits.TRISC1 = 0;
//    TRISCbits.TRISC2 = 0;
//    TRISCbits.TRISC3 = 0;
//
//    while(1)
//    {
//        //PORTC = 0xFF;
//        PORTC = portc_shadow;
//        for(int i=0; i<10000;i++);
//        portc_shadow ^= 0b00001111;
////        for(int i=0; i<10000;i++);
//    }
//}

int leftChannelDutyCycle = 0;
int rightChannelDutyCycle = 0;

void main()
{
    setup();
    while (1)
    {
        if (packetRecieved)
        {
            //Check packet integrity
            if (commandBuffer[0] == 0x02 && commandBuffer[5] == 0x03)
            {
                //Packet is OK
                //Update pwm outputs
                if (commandBuffer[3] == 0x02)
                {
                    //Both channels should be forward
                    PORTC = 0b00000011;
                }
                else if (commandBuffer[1] == 0x02)
                {

                }
                else if (commandBuffer[1] == 0x01)
                {

                }
                else
                {
                    packetCorrupt = 1;
                }
            }
            else
            {
                packetCorrupt = 1;
            }
            packetRecieved = 0;
        }
    }
}
