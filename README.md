# NEC IR Decoder INT PIC16F887

Souce: https://simple-circuit.com/mplab-xc8-nec-remote-control-decoder/

> Programming hints:
> The message of the NEC protocol is 32-bit long, address (16 bits), command (8 bits), and inverted command (8 bits). Before the 32 bits there is 9ms burst and 4.5ms space.
> A logic 1 is represented by 562.5µs burst and 562.5µs space (total of 1125µs) and a logic 0 is represented by 562.5µs burst and 1687.5µs space (total of 2250µs).
> Keep in mind that the IR receiver output is always inverted.
> 
> The interval [ 9500µs, 8500µs ] is used for the 9ms pulse and for the 4.5ms space the interval [ 5000µs, 4000µs ] is used.
> The 562.5µs pulse is checked with the interval [ 700µs, 400µs ] .
> For the 562.5µs or 1687.5µs space I used the interval [ 1800µs, 400µs ], and to know if its a short or long space I used a length of 1000µs.
> 
> The output of the IR receiver is connected to external interrupt pin (RB0) (interrupt on change) and every change in the pin status generates an interrupt and Timer1 starts ticking, Timer1 value will be used in the next interrupt, this means Timer1 measures the time between two interrupts which is pulse time or space time. Also, Timer1 interrupt is used to reset the decoding process in case of very long pulse or space.
> Timer1 time step is 1µs (Timer1 increments every 1µs). If you use mcu frequency other than 8MHz, make sure to keep Timer1 time step to 1µs, otherwise time intervals have to be changed.
> 
> The decoding results are displayed on 1602 LCD screen connected to PORTD.
> 
> The microcontroller used in this example is PIC16F887, configuration words are:
> 
> #pragma config CONFIG1 = 0x2CD4
> #pragma config CONFIG2 = 0x0700

Note: Incorrect information: NEC protocol
- Both the address and the data are 8-bit, and each has its reverse for verification.
- Ref.: https://www.researchgate.net/figure/NEC-Protocol-Transmission_fig6_328339660
