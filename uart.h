#if !defined __uart_h__
# define __uart_h__

void uart_init(void);
void uart_putchar(unsigned char c);
void uart_puthex(unsigned char c);
void uart_putnum_u16(unsigned short n, int digit);
void uart_puts(char *s);
void uart_putsln(char *s);
int uart_getchar(void);
int uart_peek(void);

#endif // !defined(__uart_h__)
