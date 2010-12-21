#include "eeprom.h"
#if defined(TEST)
# include <stdio.h>
#else // defined(TEST)
# include <avr/io.h>
#endif // defined(TEST)

#if defined(TEST)
FILE *eep_fp = NULL;

int
map
(void)
{
  if (NULL != eep_fp) return 0;
  eep_fp = fopen("eeprom.img", "r+");
  if (NULL == eep_fp) eep_fp = fopen("eeprom.img", "w");
  if (NULL == eep_fp) return -1;
  return 0;
}
#endif // defined(TEST)

void
eeprom_write
(unsigned short addr, unsigned char data)
{
#if defined(TEST)
  if (0 != map()) return;
  if (0 != fseek(eep_fp, addr, SEEK_SET)) return;
  fwrite(&data, 1, 1, eep_fp);
#else // defined(TEST)
  while (0 != (EECR & _BV(EEPE)));
  EEAR = addr;
  EEDR = data;
  EECR |= _BV(EEMPE);
  EECR |= _BV(EEPE);
#endif // defined(TEST)
}

unsigned char
eeprom_read
(unsigned short addr)
{
#if defined(TEST)
  if (0 != map()) return 0xff;
  if (0 != fseek(eep_fp, addr, SEEK_SET)) return 0xff;
  unsigned char rc;
  if (1 != fread(&rc, 1, 1, eep_fp)) return 0xff;
  return rc;
#else // defined(TEST)
  while (0 != (EECR & _BV(EEPE)));
  EEAR = addr;
  EECR |= _BV(EERE);
  return EEDR;
#endif // defined(TEST)
}

void
eeprom_write_string
(unsigned short addr, char *str)
{
  do {
    eeprom_write(addr++, *str);
  } while (0 != *str++);
}

void
eeprom_read_string
(unsigned short addr, char *str)
{
  do {
    *str = eeprom_read(addr++);
  } while (0 != *str++);
}
