#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "uart.h"

void setup(void);
void loop(void);

int main(void){
    setup();
    for(;;) loop();
}

void setup(){
    uart_init(UART_BAUD_SELECT_DOUBLE_SPEED(115201, F_CPU));
    PORTB |= 0x38;
    sei();
}

int parseHex(char* buf){
    int result = 0;
    int len = 2;
    for(int i=0; i<len; i++){
        char c = buf[len-i];
        int v = 0;
        if(c>='0' && c<='9'){
            v=(c-'0');
        }else if(c>='a' && c<= 'f'){
            v=(c-'a'+10);
        }else if(c>='A' && c<= 'F'){
            v=(c-'A'+10);
        }
        result |= v<<(4*i);
    }
    return result;
}

void loop(){ //one frame
    uint16_t receive_status = 1;
    do{ //Always empty the receive buffer since the following code might take a while. Let's hope nobody sends more than 32 bytes in the meantime.
        receive_status = uart_getc();
        char c = receive_status&0xFF;
        receive_status &= 0xFF00;
        if(!receive_status){
        }
    }while(!receive_status);
    //Transfer
    PHI_OUT &= ~_BV(PHI1_PIN);
    PHI_OUT |= ~BV(PHI2_PIN);
    _delay_us(1);
    TG1_OUT &= ~_BV(TG1_PIN);
    TG23_OUT &= ~_BV(TG23_PIN);
    _delay_us(10);
    TG1_out |= _BV(TG1_PIN);
    TG23_OUT |= _BV(TG23_PIN);
    _delay_us(1);
    PHI_OUT |= ~BV(PHI1_PIN);
    PHI_OUT &= ~_BV(PHI2_PIN);
    for(uint16_t i=5384; i; i--){
        //Pixel clock
        PHI_OUT &= ~_BV(PHI1_PIN);
        PHI_OUT |= ~BV(PHI2_PIN);
        RB_OUT |= _BV(RB_PIN);
        _delay_us(0);
        RB_OUT &= _BV(RB_PIN);
        _delay_us(0);
        CLB_OUT |= _BV(CLB_PIN);
        _delay_us(0);
        CLB_OUT &= _BV(CLB_PIN);
        _delay_us(0);
        PHI_OUT |= ~BV(PHI1_PIN);
        PHI_OUT &= ~_BV(PHI2_PIN);
        _delay_us(1); //t_d
        takeAnalogMeasurementAndRejoice();
        //repeat.
    }
}
