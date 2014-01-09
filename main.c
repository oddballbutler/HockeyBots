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
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
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
unsigned char leftPWM = 0;
unsigned char leftDir = 1;
unsigned char rightPWM = 0;
unsigned char rightDir = 1;

unsigned char packetBuffer[6];
unsigned char packetReceived = 0;
unsigned char packetCorrupt = 0;


unsigned char rxchar, i = 0;

void interrupt ISR()
{
    if (TMR2IF) //Was the interrupt the Timer 2 comparator?
    {
        if (PR2 == 0xff)
        {
            if (leftPWM > 0 && rightPWM > 0)
            {
                PR2 = leftPWM < rightPWM ? leftPWM : rightPWM;
                LATAbits.LATA4 = 1;
                LATAbits.LATA5 = 1;

            }
            else if (leftPWM > 0)
            {
                PR2 = leftPWM;
                LATAbits.LATA4 = 1;
            }
            else if (rightPWM > 0)
            {
                PR2 = rightPWM;
                LATAbits.LATA5 = 1;
            }
        }
        else
        {
            if (PR2 == leftPWM && leftPWM == rightPWM)
            {
                LATAbits.LATA4 = 0;
                LATAbits.LATA5 = 0;
                PR2 = 0xff;
            }
            else
            {
                if (PR2 == leftPWM)
                {
                    LATAbits.LATA4 = 0;
                    if (leftPWM > rightPWM)
                    {
                        PR2 = 0xff;
                    }
                    else if (rightPWM != 0)
                    {
                        PR2 = rightPWM;
                    }
                }
                else if (PR2 == rightPWM)
                {
                    LATAbits.LATA5 = 0;
                    if (rightPWM >= leftPWM)
                    {
                        PR2 = 0xff;
                    }
                    else if (leftPWM != 0)
                    {
                        PR2 = leftPWM;
                    }
                }
            }
        }
        TMR2IF = 0;
    }
    if (RCIF) //Was the interrupt the USART RX interrupt?
    {
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
    TRISAbits.TRISA4 = 0; // Left Enable
    TRISAbits.TRISA5 = 0; // Right Enable
    TRISCbits.TRISC0 = 0; // L1
    TRISCbits.TRISC1 = 0; // L2
    TRISCbits.TRISC2 = 0; // L3
    TRISCbits.TRISC3 = 0; // L4
    TRISCbits.TRISC4 = 0; // TX
    TRISCbits.TRISC5 = 1; // RX

    //Set all pins to digital I/O
    ANSELC = 0x00;
    ANSELA = 0x00;

    //Set all outputs to 0
    LATC = 0x00;
    LATA = 0x00;

    //Setup timer for PWM
    T2CONbits.T2CKPS = 0b11;
    T2CONbits.TMR2ON = 1;

    //Setup interrupts
    RCIE = 1; //Enable USART RX interrupt
    TMR2IE = 1; //Enable Timer 2 comparator interrupt
    PEIE = 1; //Enable perifferal interrupts
    GIE = 1; //Enable global interrupts
}

void main()
{
    int x = 0;
    int y = 0;
    int leftPWMtemp = 0;
    int rightPWMtemp = 0;
    int xAbs = 0;
    int yAbs = 0;

    setup();
    while (1)
    {
        if (packetReceived)
        {
            if (packetBuffer[0] == 0x02 && packetBuffer[5] == 0x03)
            {
                if (packetBuffer[1] == 0)
                {
                    x = 0 - 72 - (128 - packetBuffer[2]);
                }
                else if (packetBuffer[1] == 1)
                {
                    x = 0 - 72 + (packetBuffer[2]);
                }
                else if (packetBuffer[1] == 2)
                {
                    x = 56 + packetBuffer[2];
                }
                else
                {
                    packetCorrupt = 1;
                }

                if (packetBuffer[3] == 0)
                {
                    y = 0 - 72 - (128 - packetBuffer[4]);
                }
                else if (packetBuffer[3] == 1)
                {
                    y = 0 - 72 + (packetBuffer[4]);
                }
                else if (packetBuffer[3] == 2)
                {
                    y = 56 + packetBuffer[4];
                }
                else
                {
                    packetCorrupt = 1;
                }

                xAbs = x > 0 ? x : -x;
                yAbs = y > 0 ? y : -y;

                if (x < (-y))
                {
                    leftDir = 0;
                    if (x < 0)
                    {
                        leftPWMtemp = xAbs - y;
                    }
                    else
                    {
                        leftPWMtemp = yAbs - x;
                    }
                }
                else
                {
                    leftDir = 1;
                    if (x < 0)
                    {
                        leftPWMtemp = yAbs - xAbs;
                    }
                    else
                    {
                        leftPWMtemp = x + y;
                    }
                }

                if (y > x)
                {
                    rightDir = 1;
                    if (y > 0)
                    {
                        rightPWMtemp = y - x;
                    }
                    else
                    {
                        rightPWMtemp = xAbs - yAbs;
                    }
                }
                else
                {
                    rightDir = 0;
                    if (y > 0)
                    {
                        rightPWMtemp = xAbs - y;
                    }
                    else
                    {
                        rightPWMtemp = yAbs + x;
                    }
                }
                if (leftPWMtemp > 100)
                {
                    leftPWMtemp = 100;
                }
                if (rightPWMtemp > 100)
                {
                    rightPWMtemp = 100;
                }

                GIE = 0;
                if (leftPWMtemp == 0)
                {
                    leftPWM = 0;
                }
                else
                {
                    leftPWM = (leftPWMtemp * 2) + 50;
                }

                if (rightPWMtemp == 0)
                {
                    rightPWM = 0;
                }
                else
                {
                    rightPWM = (rightPWMtemp * 2) + 50;
                }
                if (leftDir)
                {

                    LATCbits.LATC0 = 0;
                    LATCbits.LATC1 = 1;
                }
                else
                {
                    LATCbits.LATC0 = 1;
                    LATCbits.LATC1 = 0;
                }
                if (rightDir)
                {

                    LATCbits.LATC2 = 0;
                    LATCbits.LATC3 = 1;
                }
                else
                {
                    LATCbits.LATC2 = 1;
                    LATCbits.LATC3 = 0;
                }

                GIE = 1;



                //Packet start and stop characters OK
                //                if (packetBuffer[1] == 0)
                //                {
                //                    //Joystick X left
                //                    if (packetBuffer[3] == 0)
                //                    {
                //                        //Joystick Y down
                //                        LATC = 0b00000100;
                //                    }
                //                    else if (packetBuffer[3] == 1)
                //                    {
                //                        //Joystick Y center
                //                        LATC = 0b00001001;
                //                    }
                //                    else if (packetBuffer[3] == 2)
                //                    {
                //                        //Joystick Y up
                //                        LATC = 0b00001000;
                //                    }
                //                    else
                //                    {
                //                        //Joystick Y corrupt
                //                        packetCorrupt = 1;
                //                        //putch('a');
                //                    }
                //                }
                //                else if (packetBuffer[1] == 1)
                //                {
                //                    //Joystick X center
                //                    if (packetBuffer[3] == 0)
                //                    {
                //                        //Joystick Y down
                //                        LATC = 0b00000101;
                //                    }
                //                    else if (packetBuffer[3] == 1)
                //                    {
                //                        //Joystick Y center
                //                        LATC = 0b00000000;
                //                    }
                //                    else if (packetBuffer[3] == 2)
                //                    {
                //                        //Joystick Y up
                //                        LATC = 0b00001010;
                //                    }
                //                    else
                //                    {
                //                        //Joystick Y corrupt
                //                        packetCorrupt = 1;
                //                        //putch('b');
                //                    }
                //                }
                //                else if (packetBuffer[1] == 2)
                //                {
                //                    //Joystick X right
                //                    if (packetBuffer[3] == 0)
                //                    {
                //                        //Joystick Y down
                //                        LATC = 0b00000010;
                //                    }
                //                    else if (packetBuffer[3] == 1)
                //                    {
                //                        //Joystick Y center
                //                        LATC = 0b00000110;
                //                    }
                //                    else if (packetBuffer[3] == 2)
                //                    {
                //                        //Joystick Y up
                //                        LATC = 0b00000001;
                //                    }
                //                    else
                //                    {
                //                        //Joystick Y corrupt
                //                        packetCorrupt = 1;
                //                        //putch('c');
                //                    }
                //                }
                //                else
                //                {
                //                    //Joystick X corrupt
                //                    packetCorrupt = 1;
                //                    //putch('d');
                //                }

            }
            else
            {
                //Packet start and stop characters corrupt
                packetCorrupt = 1;
            }
            packetReceived = 0;
        }
    }
}
