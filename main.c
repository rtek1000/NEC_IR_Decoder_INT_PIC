/*
 * NEC Protocol IR remote control decoder with PIC16F887 MCU.
 * C Code for MPLAB XC8 compiler.
 * Internal oscillator used @ 8MHz.
 * This is a free software with NO WARRANTY.
 * https://simple-circuit.com/
 * 
 * https://www.youtube.com/watch?v=qwRmMq5N7J4
 * https://simple-circuit.com/mplab-xc8-nec-remote-control-decoder/
*/ 

// set configuration words
#pragma config CONFIG1 = 0x2CD4
#pragma config CONFIG2 = 0x0700


//LCD module connections
#define LCD_RS       RD0
#define LCD_EN       RD1
#define LCD_D4       RD2
#define LCD_D5       RD3
#define LCD_D6       RD4
#define LCD_D7       RD5
#define LCD_RS_DIR   TRISD0
#define LCD_EN_DIR   TRISD1
#define LCD_D4_DIR   TRISD2
#define LCD_D5_DIR   TRISD3
#define LCD_D6_DIR   TRISD4
#define LCD_D7_DIR   TRISD5
//End LCD module connections

#include <xc.h>
#define _XTAL_FREQ 8000000
#include <stdio.h>         // for sprintf
#include <stdint.h>        // include stdint header
#include "LCD_Lib.c"       // include LCD driver source file


__bit nec_ok;
char text[5];
uint8_t nec_state, bit_n;
uint16_t timer_value;
uint32_t nec_code;

// interrupt ISRs
void __interrupt() EXT(void)
{
/*************** start external interrupt ISR ***************/
  if (RBIF && (RB0 || !RB0))   // PORTB change ISR (& clear mismatch condition)
  {
    RBIF = 0;   // clear PORTB interrupt flag bit
    if(nec_state != 0)
    {
      timer_value = (TMR1H << 8) | TMR1L;  // store Timer1 value
      TMR1H = TMR1L = 0;     // reset Timer1
    }

    switch(nec_state)
    {
     case 0 :              // start receiving IR data (we're at the beginning of 9ms pulse)
       TMR1H = TMR1L = 0;  // reset Timer1
       TMR1ON = 1;         // enable Timer1
       nec_state = 1;      // next state: end of 9ms pulse (start of 4.5ms space)
       bit_n = 0;
       break;

     case 1 :                                       // End of 9ms pulse
       if((timer_value > 9500) || (timer_value < 8500))
       { // invalid interval ==> stop decoding and reset
         nec_state = 0;  // reset decoding process
         TMR1ON = 0;     // disable Timer1
       }
       else
         nec_state = 2;  // next state: end of 4.5ms space (start of 562µs pulse)
       break;

     case 2 :                                       // End of 4.5ms space
       if((timer_value > 5000) || (timer_value < 4000))
       { // invalid interval ==> stop decoding and reset
         nec_state = 0;  // reset decoding process
         TMR1ON = 0;     // disable Timer1
       }
       else
         nec_state = 3; // next state: end of 562µs pulse (start of 562µs or 1687µs space)
       break;

     case 3 :    // End of 562µs pulse
       if((timer_value > 700) || (timer_value < 400))
       { // invalid interval ==> stop decoding and reset
         TMR1ON = 0;     // disable Timer1
         nec_state = 0;  // reset decoding process
       }
       else
         nec_state = 4;  // next state: end of 562µs or 1687µs space
       break;

       case 4 :
       if((timer_value > 1800) || (timer_value < 400))
       { // invalid interval ==> stop decoding and reset
         TMR1ON = 0;     // disable Timer1
         nec_state = 0;  // reset decoding process
       }

       else
       {
         if( timer_value > 1000)  // if space width > 1ms (short space)
           nec_code |=   (uint32_t)1 << (31 - bit_n);   // write 1 to bit (31 - bit_n)

         else    // if space width < 1ms (long space)
           nec_code &= ~((uint32_t)1 << (31 - bit_n));  // write 0 to bit (31 - bit_n)
         bit_n++;

         if(bit_n > 31)
         {
           nec_ok = 1;   // decoding process OK
           RBIE = 0;     // disable PORTB change interrupt 
         }

         else
           nec_state = 3;  // next state: end of 562µs pulse (start of 562µs or 1687µs space)
         
         break;
       }  // end " else "
       
    }  // end " switch(nec_state) "

  }  // end " if (RBIF && (RB0 || !RB0)) "
/*************** end external interrupt ISR ***************/

/*************** start Timer1 ISR ***************/
  if (TMR1IF)         // Timer1 ISR
  {
    TMR1IF    = 0;   // clear Timer1 overflow flag bit
    nec_state = 0;   // reset decoding process
    TMR1ON    = 0;   // disable Timer1
  }
/*************** end Timer1 ISR ***************/

}


/*************************** main function *********************/
void main(void)
{
  OSCCON = 0x70;   // set internal oscillator to 8MHz
  ANSELH = 0;      // configure all PORTB pins as digital

  TMR1IF = 0;     // clear Timer1 overflow interrupt flag bit
  RBIF   = 0;     // clear PORTB change interrupt flag bit
  TMR1IE = 1;     // enable Timer1 overflow interrupt
  T1CON  = 0x10;  // set Timer1 clock source to internal with 1:2 prescaler (Timer1 clock = 1MHz)
  INTCON = 0xC8;  // enable global, peripheral and PORTB change interrupts
  IOCB0  = 1;     // enable RB0 pin change interrupt

  nec_ok = 0;
  nec_state = 0;

  __delay_ms(1000);   // wait 1 second

  LCD_Begin();       // initialize LCD module
  LCD_Goto(1, 1);    // move cursor to column 1, row 1
  LCD_Print("Address:0x0000");
  LCD_Goto(1, 2);    // move cursor to column 1, row 2
  LCD_Print("Com:0x00 In:0x00");

  while(1)
  {
    while (!nec_ok);   // wait until NEC code receiver

    nec_ok    = 0;   // reset decoding process
    nec_state = 0;
    TMR1ON    = 0;   // disable Timer1

    uint16_t address = nec_code >> 16;
    uint8_t  command = nec_code >> 8;
    uint8_t  inv_command = nec_code;

    sprintf(text,"%04X",address);
    LCD_Goto(11, 1);   // move cursor to column 11 line 1
    LCD_Print(text);   // print address

    sprintf(text,"%02X",command);
    LCD_Goto(7, 2);    // move cursor to column 7 line 2
    LCD_Print(text);   // print command

    sprintf(text,"%02X",inv_command);
    LCD_Goto(15, 2);    // move cursor to column 15 line 2
    LCD_Print(text);    // print inverted command

    RBIE = 1;    // enable PORTB change interrupt

  }

}

/*************************** end main function ********************************/
