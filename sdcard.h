#if !defined __sdcard_h__
# define __sdcard_h__

void sdcard_init(void);
int sdcard_open(void);
int sdcard_fetch(unsigned long blk_addr);
int sdcard_store(unsigned long blk_addr);
unsigned char sdcard_read(unsigned short offset);
void sdcard_write(unsigned short offset, unsigned char data);
unsigned short sdcard_crc(void);
int sdcard_flush(void);

#endif // !defined(__sdcard_h__)
