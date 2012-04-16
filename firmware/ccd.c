#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "uart.h"

//Pin mapping
#define PHI_OUT     PORTD
#define PHI_DDR     DDRD
#define PHI1_PIN    5
#define PHI2_PIN    4
#define TG1_OUT     PORTB
#define TG1_DDR     DDRB
#define TG1_PIN     1
#define TG23_OUT    PORTB
#define TG23_DDR    DDRB
#define TG23_PIN    2
#define RB_OUT      PORTD
#define RB_DDR      DDRD
#define RB_PIN      7
#define CLB_OUT     PORTD
#define CLB_DDR     DDRD
#define CLB_PIN     6
#define ADDR_OUT    PORTD
#define ADDR_DDR    DDRD
#define ADDRA_PIN   2
#define ADDRB_PIN   3

void setup(void);
void loop(void);

int main(void){
    setup();
    for(;;) loop();
}

void setup(){
    uart_init(UART_BAUD_SELECT_DOUBLE_SPEED(115201, F_CPU));
    //PORTB |= 0x38;
    PHI_DDR |= _BV(PHI1_PIN) | _BV(PHI2_PIN);
    TG1_DDR |= _BV(TG1_PIN);
    TG23_DDR |= _BV(TG23_PIN);
    RB_DDR |= _BV(RB_PIN);
    CLB_DDR |= _BV(CLB_PIN);
    ADDR_DDR |= _BV(ADDRA_PIN) | _BV(ADDRB_PIN);
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
            uart_putc(c);
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
