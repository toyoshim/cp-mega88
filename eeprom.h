#if !defined __eeprom_h__
# define __eeprom_h__

void eeprom_write(unsigned short addr, unsigned char data);
unsigned char eeprom_read(unsigned short addr);

void eeprom_write_string(unsigned short addr, char *str);
void eeprom_read_string(unsigned short addr, char *str);

#endif // !defined(__eeprom_h__)
