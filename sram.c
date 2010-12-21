#include "sram.h"
#if !defined(TEST)
# include <avr/io.h>
#endif // !defined(TEST)

#if defined(TEST)
static unsigned char
sram[64 * 1024];

unsigned char
sram_read
(unsigned short addr)
{
  return sram[addr];
}

void
sram_write
(unsigned short addr, unsigned char data)
{
  sram[addr] = data;
}

#endif // defined(TEST)

void
sram_init
(void)
{
#if !defined(TEST)
  /*
   * PB0: /W
   * PB1: E2
   * PB2: A16
   * PB3: N/A (input)
   * PB4: CLK for FF on Address Low
   * PB5: CLK for FF on Address High
   * PD*: Address / Data
   */
  // Port Settings
  //   All ports are Output excepr for PB3
  DDRB    =  (_BV(DDB0) | _BV(DDB1) | _BV(DDB2) | _BV(DDB4) | _BV(DDB5));
  PORTB   = 0;
  DDRD    = 0xff;
  PORTD   = 0;
#endif // !defined(TEST)
}
