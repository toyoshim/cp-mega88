#if !defined __sram_h__
# define __sram_h__

void sram_init(void);
unsigned char sram_read(unsigned short addr);
void sram_write(unsigned short addr, unsigned char data);

#endif // !defined(__sram_h__)
