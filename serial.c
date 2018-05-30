#include <avr/io.h>

/*
 * Initialize the serial port.
 */
void serial_init() {
   uint16_t baud_setting;

   UCSR0A = _BV(U2X0);
   baud_setting = 16; //115200 baud

   // assign the baud_setting
   UBRR0H = baud_setting >> 8;
   UBRR0L = baud_setting;

   // enable transmit and receive
   UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
}

/*
 * Return 1 if a character is available else return 0.
 */
uint8_t byte_available() {
   return (UCSR0A & (1 << RXC0)) ? 1 : 0;
}

/*
 * Unbuffered read
 * Return 255 if no character is available otherwise return available character.
 */
uint8_t read_byte() {
   if (UCSR0A & (1 << RXC0)) return UDR0;
   return 255;
}

/*
 * Unbuffered write
 *
 * b byte to write.
 */
uint8_t write_byte(uint8_t b) {
   //loop until the send buffer is empty
   while (((1 << UDRIE0) & UCSR0B) || !(UCSR0A & (1 << UDRE0))) {}

   //write out the byte
   UDR0 = b;
   return 1;
}




void print_string(char *s)
{
    while (*s != 0)
    {
        write_byte(*(uint8_t *)s);
        s++;
    }
}

void print_int(uint16_t i)
{
    uint16_t divisor = 10000;
    uint8_t ans;
    char seen_z = 0;      
 
    while (divisor > 0)
    {
        ans = i / divisor;
        if (ans != 0) {seen_z = 1;}
        if (seen_z == 1) {write_byte(ans+48);}
        i = i % divisor;
        divisor = divisor/10;
    }
}

void print_int32(uint32_t i)
{
    uint32_t divisor = 1000000000;
    uint8_t ans;
    char seen_z = 0;   
 
    while (divisor > 0)
    {
        ans = i / divisor;
        if (ans != 0) {seen_z = 1;}
        if (seen_z == 1) {write_byte(ans+48);}
        i = i % divisor;
        divisor = divisor/10;
    }
}

void print_hex(uint16_t i)
{
    uint8_t ans;
    int n;
    char seen_z = 0;
    for (n = 0; n < 4; n++)
    {
        ans = (i & 0xF000) >> 12;
        if (ans != 0) {seen_z = 1;}
        if (ans > 9)
        {
            ans = ans + 55;
        }
        else
        {
            ans = ans + 48;
        }
        if (seen_z == 1) {write_byte(ans);}
        i = i << 4;
    }
}

void print_hex32(uint32_t i)
{
    uint8_t ans;
    int n;
    char seen_z = 0;
    for (n = 0; n < 8; n++)
    {
        ans = (i & 0xF0000000) >> 28;
        if (ans != 0) {seen_z = 1;}
        if (ans > 9)
        {
            ans = ans + 55;
        }
        else
        {
            ans = ans + 48;
        }
        if (seen_z == 1) {write_byte(ans);}
        i = i << 4;
    }  
}

void set_cursor(uint8_t row, uint8_t col)
{
    int i;
    //<ESC>[{ROW};{COLUMN}f
    i = write_byte(0x1B); 
    i = write_byte(0x5B);
    if (row > 99) {i = write_byte((row/100)+48);}
    if (row > 9) {i = write_byte(((row%100)/10)+48);}
    i = write_byte((row%10)+48);
    i = write_byte(0x3B);
    if (col > 99) {i = write_byte((col/100)+48);}
    if (col > 9) {i = write_byte(((col%100)/10)+48);}
    i = write_byte((col%10)+48);
    i = write_byte(0x66);
}

void set_color(uint8_t color)
{
    int i;
    //<ESC>[{attr1}m
    i = write_byte(0x1B);
    i = write_byte(0x5B);
    i = write_byte((color/10)+48);
    i = write_byte((color%10)+48);
    i = write_byte(0x6D);
}

void clear_screen()
{
    int i;
    //<ESC>[2J
    i = write_byte(0x1B);
    i = write_byte(0x5B);
    i = write_byte(0x32);
    i = write_byte(0x4A);
}








