////+++++++++++++++++++++++++++++++++++++| LIBRARIES / HEADERS |+++++++++++++++++++++++++++++++++++++
#include "device_config.h"
#include <stdint.h>
#include <math.h>

//+++++++++++++++++++++++++++++++++++++| DIRECTIVES |+++++++++++++++++++++++++++++++++++++
#define _XTAL_FREQ 8000000
#define SWEEP_FREQ 50
#define LCD_DATA_R          PORTD
#define LCD_DATA_W          LATD
#define LCD_DATA_DIR        TRISD
#define LCD_RS              LATCbits.LATC2   
#define LCD_RS_DIR          TRISCbits.TRISC2
#define LCD_RW              LATCbits.LATC1
#define LCD_RW_DIR          TRISCbits.TRISC1
#define LCD_E               LATCbits.LATC0
#define LCD_E_DIR           TRISCbits.TRISC0           

//+++++++++++++++++++++++++++++++++++++| DATA TYPES |+++++++++++++++++++++++++++++++++++++
enum por_ACDC {digital, analog};          // digital = 0, analog = 1

////+++++++++++++++++++++++++++++++++++++| ISRs |+++++++++++++++++++++++++++++++++++++
//// ISR for high priority
//void __interrupt ( high_priority ) high_isr( void );
//// ISR for low priority
//void __interrupt ( low_priority ) low_isr( void ); 

//+++++++++++++++++++++++++++++++++++++| FUNCTION DECLARATIONS |+++++++++++++++++++++++++++++++++++++
void portsInit(void);
uint8_t char_to_seg(uint8_t);
void send_to_disp(uint32_t);
char key_scanner(void);
void LCD_init(void);
void LCD_cmd(char);
void LCD_rdy(void);
void send2LCD(char);

char myfunction(char myarr[]) {
    switch(myarr[1]) {
        case 10:
            return ((int)myarr[0] + (int)myarr[2] +'0');
            break;
        case 11:
            return ((int)myarr[0] - (int)myarr[2] +'0');
            break;
        case 12:
            return ((int)myarr[0] * (int)myarr[2] +'0');
            break;
        case 13:
            return ((int)myarr[0] / (int)myarr[2] +'0');
            break;
        default:
            break;
    }
    return 'x';
}

//+++++++++++++++++++++++++++++++++++++| MAIN |+++++++++++++++++++++++++++++++++++++
void main(void){
    portsInit();
    LCD_init();
   /* int counter = 0;
    char myarr[4];

    while(1) {
        char key = key_scanner();
        if (key != 'x') {
            myarr[counter] = key;
            counter++;
        }

        if(counter == 4 ) {
            myarr[3] =  myfunction(myarr);
            counter = 0;
        }
        for (int i = 0; i < counter+1; i++) {
            if ( i < 3){
                send2LCD(myarr[i]);
            } else {
                LCD_cmd(0x18);
                send2LCD( myarr[i]);
            }
        }
    }*/
}

//+++++++++++++++++++++++++++++++++++++| FUNCTIONS |+++++++++++++++++++++++++++++++++++++
void portsInit(void){
    ANSELA = digital;   // Set port A as Digital for keypad driving
    TRISA  = 0xF0;      // For Port A, set pins 4 to 7 as inputs (columns), and pins 0 to 3 as outputs (rows)
    OSCCON = 0x74;      // Set the internal oscillator to 8MHz and stable
}


char key_scanner(void){
    LATAbits.LA0 = 0;
    LATAbits.LA1 = 1;
    LATAbits.LA2 = 1;
    LATAbits.LA3 = 1;
    __delay_ms(SWEEP_FREQ);
    if      (PORTAbits.RA4 == 0) {__delay_ms(SWEEP_FREQ); return 1;}
    else if (PORTAbits.RA5 == 0) {__delay_ms(SWEEP_FREQ); return 2;}
    else if (PORTAbits.RA6 == 0) {__delay_ms(SWEEP_FREQ); return 3;}
    else if (PORTAbits.RA7 == 0) {__delay_ms(SWEEP_FREQ); return 10;}
    LATAbits.LA0 = 1;
    LATAbits.LA1 = 0;
    LATAbits.LA2 = 1;
    LATAbits.LA3 = 1;
    __delay_ms(SWEEP_FREQ);
    if      (PORTAbits.RA4 == 0) {__delay_ms(SWEEP_FREQ); return 4;}
    else if (PORTAbits.RA5 == 0) {__delay_ms(SWEEP_FREQ); return 5;}
    else if (PORTAbits.RA6 == 0) {__delay_ms(SWEEP_FREQ); return 6;}
    else if (PORTAbits.RA7 == 0) {__delay_ms(SWEEP_FREQ); return 11;}
    LATAbits.LA0 = 1;
    LATAbits.LA1 = 1;
    LATAbits.LA2 = 0;
    LATAbits.LA3 = 1;
    __delay_ms(SWEEP_FREQ);
    if      (PORTAbits.RA4 == 0) {__delay_ms(SWEEP_FREQ); return 7;}
    else if (PORTAbits.RA5 == 0) {__delay_ms(SWEEP_FREQ); return 8;}
    else if (PORTAbits.RA6 == 0) {__delay_ms(SWEEP_FREQ); return 9;}
    else if (PORTAbits.RA7 == 0) {__delay_ms(SWEEP_FREQ); return 12;}
    LATAbits.LA0 = 1;
    LATAbits.LA1 = 1;
    LATAbits.LA2 = 1;
    LATAbits.LA3 = 0;
    __delay_ms(SWEEP_FREQ);
    if      (PORTAbits.RA4 == 0) {__delay_ms(SWEEP_FREQ); return 14;}
    else if (PORTAbits.RA5 == 0) {__delay_ms(SWEEP_FREQ); return 0;}
    else if (PORTAbits.RA6 == 0) {__delay_ms(SWEEP_FREQ); return 15;}
    else if (PORTAbits.RA7 == 0) {__delay_ms(SWEEP_FREQ); return 13;}
    else return 'x';
}

// Send command to the LCD
void LCD_cmd(char cx) {
    //LCD_rdy();              // wait until LCD is ready
    LCD_RS = 0;             // select IR register
    LCD_RW = 0;             // set WRITE mode
    LCD_E  = 1;             // set to clock data
    Nop();
    LCD_DATA_W = cx;        // send out command
    Nop();                  // No operation (small delay to lengthen E pulse)
    LCD_E = 0;              // complete external write cycle
}

// Function to wait until the LCD is not busy
void LCD_rdy(void) {
    char test;
    // configure LCD data bus for input
    LCD_DATA_DIR = 0b11111111;
    test = 0x80;
    while(test){
        LCD_RS = 0;         // select IR register
        LCD_RW = 1;         // set READ mode
        LCD_E  = 1;         // setup to clock data
        test = LCD_DATA_R;
        Nop();
        LCD_E = 0;          // complete the READ cycle
        test &= 0x80;       // check BUSY FLAG
    }
}

// Function to display data on the LCD
void send2LCD(char xy){
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_E  = 1;
    LCD_DATA_W = xy;
    Nop();
    Nop();
    LCD_E  = 0;
}

void LCD_init(void) {
    LATC = 0;               // Make sure LCD control port is low
    LCD_E_DIR = 0;          // Set Enable as output
    LCD_RS_DIR = 0;         // Set RS as output
    LCD_RW_DIR = 0;         // Set R/W as output
    LCD_cmd(0x38);          // Display to 2x40
    LCD_cmd(0x0F);          // Display on, cursor on and blinking
    LCD_cmd(0x01);          // Clear display and move cursor home
}