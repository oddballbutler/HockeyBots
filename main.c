// PIC16F1455 Configuration Bit Settings

#include <xc.h>

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT enabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = ON   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover Mode (Internal/External Switchover Mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config CPUDIV = CLKDIV2 // CPU System Clock Selection Bit (CPU system clock divided by 2)
#pragma config USBLSCLK = 48MHz // USB Low SPeed Clock Selection bit (System clock expects 48 MHz, FS/LS USB CLKENs divide-by is set to 8.)
#pragma config PLLMULT = 3x     // PLL Multipler Selection Bit (3x Output Frequency Selected)
#pragma config PLLEN = DISABLED // PLL Enable Bit (3x or 4x PLL Disabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP = OFF         // Low-Voltage Programming Enable (Low-voltage programming enabled)

#include <stdio.h>
#include "usart.h"

// Function Prototypes

// Global Variables
unsigned char packetBuffer[6];
unsigned char packetReceived = 0;
unsigned char packetCorrupt = 0;


unsigned char rxchar, i = 0;

void interrupt ISR()
{
    if (RCIF) //Was the interrupt the USART RX interrupt?
    {
        //PORTC = 0b00000001;
        rxchar = getch();
        if (packetCorrupt)
        {
            //We need to wait for packet start character
            if (rxchar == 0x02)
            {
                //Start character received
                packetCorrupt = 0;
            }
            else
            {
                return;
            }
        }
        packetBuffer[i] = rxchar;
        i++;
        if (i >= 6)
        {
            //PORTC = 0b00000011;
            packetReceived = 1;
            i = 0;
        }
    }
}

void setup()
{
    //Setup clock
    OSCCONbits.IRCF = 0b1111;

    //Setup USART
    init_comms(); // set up the USART - settings defined in usart.h

    //Setup I/O directions
    TRISCbits.TRISC0 = 0;
    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC3 = 0;

    PORTC = 0x00;

    //Setup interrupts
    RCIE = 1; //Enable USART RX interrupt
    PEIE = 1; //Enable perifferal interrupts
    GIE = 1; //Enable global interrupts
}

void main()
{
    setup();

    while (1)
    {
        if (packetReceived)
        {
            if (packetBuffer[0] == 0x02 && packetBuffer[5] == 0x03)
            {
                //Packet start and stop characters OK
                if (packetBuffer[1] == 0)
                {
                    //Joystick X left
                    if (packetBuffer[3] == 0)
                    {
                        //Joystick Y down
                        PORTC = 0b00000100;
                    }
                    else if (packetBuffer[3] == 1)
                    {
                        //Joystick Y center
                        PORTC = 0b00001001;
                    }
                    else if (packetBuffer[3] == 2)
                    {
                        //Joystick Y up
                        PORTC = 0b00001000;
                    }
                    else
                    {
                        //Joystick Y corrupt
                        packetCorrupt = 1;
                    }
                }
                else if (packetBuffer[1] == 1)
                {
                    //Joystick X center
                    if (packetBuffer[3] == 0)
                    {
                        //Joystick Y down
                        PORTC = 0b00000101;
                    }
                    else if (packetBuffer[3] == 1)
                    {
                        //Joystick Y center
                        PORTC = 0b00000000;
                    }
                    else if (packetBuffer[3] == 2)
                    {
                        //Joystick Y up
                        PORTC = 0b00001010;
                    }
                    else
                    {
                        //Joystick Y corrupt
                        packetCorrupt = 1;
                    }
                }
                else if (packetBuffer[1] == 2)
                {
                    //Joystick X right
                    if (packetBuffer[3] == 0)
                    {
                        //Joystick Y down
                        PORTC = 0b00000010;
                    }
                    else if (packetBuffer[3] == 1)
                    {
                        //Joystick Y center
                        PORTC = 0b00000110;
                    }
                    else if (packetBuffer[3] == 2)
                    {
                        //Joystick Y up
                        PORTC = 0b00000001;
                    }
                    else
                    {
                        //Joystick Y corrupt
                        packetCorrupt = 1;
                    }
                }
                else
                {
                    //Joystick X corrupt
                    packetCorrupt = 1;
                }

            }
            else
            {
                //Packet start and stop characters corrupt
                packetCorrupt = 1;
                //PORTC = 0x01;
            }
        }
    }
}
