#include "uart.h"
#include <avr/io.h>

void
uart_init
(void)
{
  UCSR0A = 0x02; // U2X
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  UCSR0C = 0x06; // 8bit
  UBRR0H = 0;
  UBRR0L = 25; // 38400bps + 0.2%
}

void
uart_putchar
(unsigned char c)
{
  while (0 == (UCSR0A & (1 << UDRE0)));
  UDR0 = c;
}

static void
put_halfhex
(unsigned char c)
{
  if (c < 10) uart_putchar('0' + c);
  else uart_putchar('A' - 10 + c);
}

void
uart_puthex
(unsigned char c)
{
  put_halfhex(c >> 4);
  put_halfhex(c & 15);
}

void
uart_putnum_u16
(unsigned short n, int digit)
{
  unsigned short d = 10000;
  if (digit > 0) {
    d = 1;
    for (digit--; digit > 0; digit--) d *= 10;
  }
  do {
    int num = n / d;
    n = n % d;
    d /= 10;
    uart_putchar('0' + num);
  } while (0 != d);
}

void
uart_puts
(unsigned char *s)
{
  while (0 != *s) uart_putchar(*s++);
}

int
uart_getchar
(void)
{
  return 0;
}

int
uart_peek
(void)
{
  return 0;
}
