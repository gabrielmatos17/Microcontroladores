#include <msp430g2553.h>
#include <legacymsp430.h>
#define BAUD_9600   0
#define BAUD_19200  1
#define BAUD_38400  2
#define BAUD_56000  3
#define BAUD_115200 4
#define BAUD_128000 5
#define BAUD_256000 6
#define NUM_BAUDS   7

#define BTN  BIT3

#define RX BIT1
#define TX BIT2

#define lcd_port        P2OUT
#define lcd_port_dir    P2DIR

#define LCD_EN      BIT4
#define LCD_RS      BIT5

void lcd_reset()
{
    lcd_port_dir = 0xff;
    lcd_port = 0xff;
    __delay_cycles(20000);
    lcd_port = 0x03+LCD_EN;
    lcd_port = 0x03;
    __delay_cycles(10000);
    lcd_port = 0x03+LCD_EN;
    lcd_port = 0x03;
    __delay_cycles(1000);
    lcd_port = 0x03+LCD_EN;
    lcd_port = 0x03;
    __delay_cycles(1000);
    lcd_port = 0x02+LCD_EN;
    lcd_port = 0x02;
    __delay_cycles(1000);
}

void lcd_cmd (char cmd)
{
    // Send upper nibble
    lcd_port = ((cmd >> 4) & 0x0F)|LCD_EN;
    lcd_port = ((cmd >> 4) & 0x0F);

    // Send lower nibble
    lcd_port = (cmd & 0x0F)|LCD_EN;
    lcd_port = (cmd & 0x0F);

    __delay_cycles(4000);
}

void lcd_init ()
{
    lcd_reset();         // Call LCD reset
    lcd_cmd(0x28);       // 4-bit mode - 2 line - 5x7 font.
    lcd_cmd(0x0C);       // Display no cursor - no blink.
    lcd_cmd(0x06);       // Automatic Increment - No Display shift.
    lcd_cmd(0x80);       // Address DDRAM with 0 offset 80h.
    lcd_cmd(0x01);       // Clear screen
}

void lcd_data (unsigned char dat)
{
    // Send upper nibble
    lcd_port = (((dat >> 4) & 0x0F)|LCD_EN|LCD_RS);
    lcd_port = (((dat >> 4) & 0x0F)|LCD_RS);

    // Send lower nibble
    lcd_port = ((dat & 0x0F)|LCD_EN|LCD_RS);
    lcd_port = ((dat & 0x0F)|LCD_RS);

    __delay_cycles(4000); // a small delay may result in missing char display
}


void display_line(char *line)
{
    while (*line)
        lcd_data(*line++);
}


void atraso(volatile unsigned int i)
{
    while((i--)>0);
}



configura_adc(void)
{
    ADC10CTL1  = SHS_1 + CONSEQ_2 + INCH_0+ ADC10DIV_1;                           // timerAout1 dispara - leitura consecutiva - canal 1 leitura p1.1
    ADC10CTL0  = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;    // tensao referencia - clks para aproximçao - referencia tensao interna on - adcon - interrupt on
    ADC10CTL0 &= ~ENC;
    ADC10AE0 |=0x0;

}
configurar_timerA(void)
{
    TACCR0 = 62000-1;                                      //conta ate este valor e zera
    TACCTL1 = OUTMOD_3;                                   // leva a saida para quato contar 2047
    TACCR1 = TACCR0-1;                                   //
    TACTL = TASSEL_2 | ID_3 | MC_1;                     //clk aux - contar up
}

void Init_UART(unsigned int baud_rate_choice)
{
    unsigned char BRs[NUM_BAUDS] = {104, 52, 26, 17, 8, 7, 3};
    unsigned char MCTLs[NUM_BAUDS] = {UCBRF_0+UCBRS_1,
                                        UCBRF_0+UCBRS_0,
                                        UCBRF_0+UCBRS_0,
                                        UCBRF_0+UCBRS_7,
                                        UCBRF_0+UCBRS_6,
                                        UCBRF_0+UCBRS_7,
                                        UCBRF_0+UCBRS_7};
    if(baud_rate_choice<NUM_BAUDS)
    {
        // Habilita os pinos para transmissao serial UART
        P1SEL2 = P1SEL = RX+TX;
        // Configura a transmissao serial UART com 8 bits de dados,
        // sem paridade, comecando pelo bit menos significativo,
        // e com um bit de STOP
        UCA0CTL0 = 0;
        // Escolhe o SMCLK como clock para a UART
        UCA0CTL1 = UCSSEL_2;
        // Define a baud rate
        UCA0BR0 = BRs[baud_rate_choice];
        UCA0BR1 = 0;
        UCA0MCTL = MCTLs[baud_rate_choice];
    }
}




void Send_Data(unsigned int t,unsigned int v)
{
    while((IFG2&UCA0TXIFG)==0);
    UCA0TXBUF = v;
    while((IFG2&UCA0TXIFG)==0);
    UCA0TXBUF = t;
}

short int uperbyte=0,lowerbyte=0;

void lower_byte(unsigned int A)
{
   lowerbyte = A & 255;
}

void uper_byte(int A)
{
    uperbyte = A >> 8;
}



int adc=0,j=1;

int main(void)
{

    WDTCTL = WDTPW | WDTHOLD;                   // stop watchdog timer
    BCSCTL1= CALBC1_1MHZ;
    DCOCTL=  CALDCO_1MHZ;
    P1DIR |=  BIT6;                          //definindo o led3
    P1OUT &= ~BIT6;                         //estado inicial desligado

    P1DIR &= ~BTN;                         //para o botão
    P1REN |= BTN;
    P1OUT |= BTN;
    P1IES |= BTN;
    P1IE |= BTN;

    lcd_init();
    lcd_cmd(0x80); // select 1st line (0x80 + addr) - here addr = 0x00
    display_line("Pressione Botao");
    lcd_cmd(0xc0); // select 2nd line (0x80 + addr) - here addr = 0x40
    display_line("  Para iniciar");

    configura_adc();
    configurar_timerA();
    Init_UART(BAUD_9600);

    _BIS_SR(LPM0_bits+GIE);
    return 0;
}

//a interrupçao é chamada sempre uma nova converção é realizada
interrupt(ADC10_VECTOR) ADC10_ISR(void)
{

    lower_byte(ADC10MEM);
    uper_byte(ADC10MEM);

    Send_Data(lowerbyte,uperbyte);
    P1OUT ^= BIT6;  //a cada converçao o led troca o estado
    //UCA0TXBUF=0;

}

interrupt(PORT1_VECTOR) Interrupcao_P1(void)
{
    while((P1IN&BTN)==0);
    ADC10CTL0 ^= ENC;
            j ^=BIT0;

    if (j==0)
    {
        lcd_init();
        display_line("Sistema Online");
        lcd_cmd(0xc0); // select 2nd line (0x80 + addr) - here addr = 0x40
        display_line("       :)");
    }
    else if (j==1)
    {
        lcd_init();
        display_line("Sistema Offline");
        lcd_cmd(0xc0); // select 2nd line (0x80 + addr) - here addr = 0x40
        display_line("       :|");

    }
    P1IFG &= ~BTN;
}


