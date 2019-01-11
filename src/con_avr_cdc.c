/*
 * Copyright (c) 2018, Takashi TOYOSHIMA <toyoshim@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the authors nor the names of its contributors may be
 *   used to endorse or promote products derived from this software with out
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUE
 * NTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include "con.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "led.h"

#if !defined(MCU_32U2) && !defined(MCU_32U4)
# error not supported mcu
#endif

#if defined(MCU_32U2)
# define PINDIV PLLP0
#endif

enum {
  dir_host_to_device = 0,
  dir_device_to_host = 1,

  type_standard = 0,
  type_class = 1,
  type_vendor = 2,

  recp_device = 0,
  recp_interface = 1,
  recp_endpoint = 2,

  req_set_address = 5,
  req_get_descriptor = 6,
  req_set_configuration = 9,

  req_cdc_set_line_coding = 32,
  req_cdc_set_control_line_state = 34,

  desc_device = 1,
  desc_configuration = 2,
  desc_string = 3,
  desc_interface = 4,
  desc_endpoint = 5,
  desc_device_qualifier = 6,
  desc_cs_interface = 36,

  desc_cs_sub_header = 0,
  desc_cs_sub_call_management = 1,
  desc_cs_sub_acm = 2,
  desc_cs_sub_union = 6,

  class_cdc = 2,
  class_cdc_data = 10,
  subclass_standard = 0,
  subclass_acm = 2,
  protocol_standard = 0,

  string_index_manufacturer = 1,
  string_index_product = 2,
  string_index_serial = 3,

  ep_type_control = 0,
  ep_type_bulk = _BV(EPTYPE1),
  ep_type_interrupt = _BV(EPTYPE1) | _BV(EPTYPE0),
  ep_dir_out = 0,
  ep_dir_in = 1,
};

static const uint8_t PROGMEM device_desc[0x12] = {
  0x12,        // bLength
  desc_device,
  0x00, 0x02,  // bcdUSB (2.00)
  class_cdc,
  subclass_acm,
  protocol_standard,
  0x20,        // bMaxPacketSize (32)
  0x66, 0x66,  // VID (0x6666)
  0x80 ,0x80,  // PID (0x8080)
  0x00, 0x01,  // bcdDevice (1.00)
  string_index_manufacturer,
  string_index_product,
  string_index_serial,
  0x01,        // bNumConfigurations
};

static const uint8_t PROGMEM config_desc[0x43] = {
  0x09,        // bLength
  desc_configuration,
  0x43, 0x00,  // wTotalLength
  0x02,        // bNumInterfaces
  0x01,        // bConfigurationValue
  0x00,        // iConfiguration
  0x80,        // bmAttributes (bus-powered)
  0xfa,        // bMaxPower (500mA)

  // interface descriptor for interface 0 (control)
  0x09,        // bLength
  desc_interface,
  0x00,        // bInterfaceNumber
  0x00,        // bAlternateSetting
  0x01,        // bNumEndpoints
  class_cdc,
  subclass_acm,
  protocol_standard,
  0x00,        // iInterface

  // CDC specific - header functional descriptor
  0x05,        // bLength
  desc_cs_interface,
  desc_cs_sub_header,
  0x10, 0x01,  // bcdCDC (1.10)

  // CDC specific - call management functional descriptor
  0x05,        // bLength
  desc_cs_interface,
  desc_cs_sub_call_management,
  0x00,        // bmCapabilities
  0x01,        // bDataInterface

  // CDC specific - ACM functional descriptor
  0x04,        // bLength
  desc_cs_interface,
  desc_cs_sub_acm,
  0x00,        // bmCapabilities

  // CDC specific - union
  0x05,        // bLength
  desc_cs_interface,
  desc_cs_sub_union,
  0x00,        // bControllerInterface
  0x01,        // bSubordinateInterface0

  // endpoint descriptor for endpoint 1-in of interface 0 (control)
  0x07,        // bLength
  desc_endpoint,
  0x81,        // bEndpointAddress
  0x03,        // bmAttributes (interrupt)
  0x40, 0x00,  // wMaxPacketSize (64)
  0x10,        // bInterval (16ms)

  // interface descriptor for interface 1 (data)
  0x09,        // bLength
  desc_interface,
  0x01,        // bInterfaceNumber
  0x00,        // bAlternateSetting
  0x02,        // bNumEndpoints
  class_cdc_data,
  subclass_standard,
  protocol_standard,
  0x00,        // iInterface

  // endpoint descriptor for endpoint 2-out of interface 1 (data)
  0x07,        // bLength
  desc_endpoint,
  0x02,        // bEndpointAddress
  0x02,        // bmAttributes (bulk)
  0x40, 0x00,  // wMaxPacketSize (64)
  0x00,        // bInterval (n/a)

  // endpoint descriptor for endpoint 3-in of interface 1 (data)
  0x07,        // bLength
  desc_endpoint,
  0x83,        // bEndpointAddress
  0x02,        // bmAttributes (bul)
  0x40, 0x00,  // wMaxPacketSize (64)
  0x00,        // bInterval (n/a)
};

static const uint8_t PROGMEM lang_string_desc[0x04] = {
  0x04,        // bLength
  desc_string,
  0x09, 0x04,
};

static const uint8_t PROGMEM manufacturer_string_desc[0x20] = {
  0x20,        // bLength
  desc_string,
  'T', 0, 'O', 0, 'Y', 0, 'O', 0, 'S', 0, 'H', 0, 'I', 0, 'M', 0, 'A',0, '-', 0,
  'H', 0, 'O', 0, 'U', 0, 'S', 0, 'E', 0,
};

static const uint8_t PROGMEM product_string_desc[0x18] = {
  0x18,        // bLength
  desc_string,
  'C', 0, 'P', 0, '/', 0, 'M', 0, 'E', 0, 'G', 0, 'A', 0, '3', 0, '2',0, 'U', 0,
  '2', 0,
};

static const uint8_t PROGMEM serial_string_desc[0x0a] = {
  0x0a,        // bLength
  desc_string,
  '0', 0, '.', 0, '2', 0, '0', 0,
};

struct device_request {
  union {
    uint8_t B;
    struct {
      uint8_t receptor : 5;
      uint8_t type : 2;
      uint8_t direction : 1;
    } b;
  } bmRequestType;
  uint8_t bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
};

struct line_coding {
  uint32_t dwOTERate;
  uint8_t bCharFormat;
  uint8_t bParityType;
  uint8_t bDataBits;
};

enum {
  state_not_ready = 0,
  state_ready = 1,
  state_error = 2,
};

static volatile uint8_t state = state_not_ready;
static volatile uint32_t tick = 0;

static volatile uint8_t tx_buf[16];
static volatile uint8_t tx_wr_index = 0;
static volatile uint8_t tx_rd_index = 0;
static const uint8_t tx_index_mask = 0x0f;

static volatile uint8_t rx_buf[4];
static volatile uint8_t rx_wr_index = 0;
static volatile uint8_t rx_rd_index = 0;
static const uint8_t rx_index_mask = 0x03;

static uint8_t
min_u8(uint8_t a, uint8_t b)
{
  return (a < b) ? a : b;
}

static int ep_init
(uint8_t num, uint8_t type, uint8_t dir)
{
  UENUM = num;
  UECONX = _BV(EPEN);
  UECFG0X = type | dir;

  const uint8_t epsize_32b = _BV(EPSIZE1);
  const uint8_t epbank_single = 0;
  UECFG1X = epsize_32b | epbank_single | _BV(ALLOC);
  return (UESTA0X & _BV(CFGOK)) ? 0 : 1;
}

static void ep_read
(void* buffer, uint8_t size)
{
  uint8_t* p = (uint8_t*)buffer;
  for (; size; --size)
    *p++ = UEDATX;
}

static void ep_write
(const void* buffer, uint8_t size)
{
  uint8_t* p = (uint8_t*)buffer;
  uint8_t written = 0;
  for (; size; --size) {
    UEDATX = pgm_read_byte(p++);
    if (++written == 0x20) {
      UEINTX = ~_BV(TXINI);
      while (!(UEINTX & _BV(TXINI)));
      written = 0;
    }
  }
}

ISR
(USB_GEN_vect)
{
  const uint8_t flags = UDINT;
  UDINT = 0;
  if (flags & _BV(EORSTI)) {
    state = state_error;
    if (ep_init(0, ep_type_control, ep_dir_out) +
        ep_init(1, ep_type_interrupt, ep_dir_in) +
        ep_init(2, ep_type_bulk, ep_dir_out)) {
      return;
    }
    UEIENX |= _BV(RXOUTE);
    if (ep_init(3, ep_type_bulk, ep_dir_in))
      return;
    state = state_not_ready;
    UENUM = 0;
    UEIENX |= _BV(RXSTPE);
  }
  if (flags & _BV(SOFI)) {
    tick++;
  }
}

ISR
(USB_COM_vect)
{
  UENUM = 2;
  if (UEINTX & _BV(RXOUTE)) {
    while (UEBCLX) {
      uint8_t next_index = (rx_wr_index + 1) & rx_index_mask;
      if (next_index == rx_rd_index)
        break;  // buffer full, data lost
      rx_buf[rx_wr_index] = UEDATX;
      rx_wr_index = next_index;
    }
    UEINTX = ~(_BV(RXOUTI) | _BV(FIFOCON)) & 0xff;
  }
  if (tx_rd_index != tx_wr_index) {
    UENUM = 3;
    if (UEINTX & _BV(TXINI)) {
      UEINTX = ~_BV(TXINI);
      while (tx_rd_index != tx_wr_index) {
        UEDATX = tx_buf[tx_rd_index];
        tx_rd_index = (tx_rd_index + 1) & tx_index_mask;
      }
      UEINTX = ~_BV(FIFOCON) & 0xff;
      UEIENX &= ~_BV(TXINE);
    }
  }
  UENUM = 0;
  if (!(UEINTX & _BV(RXSTPI)))
    return;

  struct device_request r;
  ep_read(&r, sizeof(r));
  UEINTX = ~(_BV(RXSTPI) | _BV(RXOUTI) | _BV(TXINI));
  if (r.bmRequestType.b.direction == dir_device_to_host)
    while (!(UEINTX & _BV(TXINI)));
  else
    UEINTX = ~_BV(TXINI);

  if (state == state_error) {
  } else if (r.bmRequestType.b.type == type_standard &&
      r.bmRequestType.b.receptor == recp_device) {
    switch (r.bRequest) {
    case req_set_address:
      UDADDR = r.wValue & 0x7f;
      while (!(UEINTX & _BV(TXINI)));
      UDADDR |= _BV(ADDEN);
      return;
    case req_get_descriptor:
      switch (r.wValue >> 8) {
      case desc_device:
        ep_write(device_desc, min_u8(sizeof(device_desc), r.wLength));
        break;
      case desc_configuration:
        ep_write(config_desc, min_u8(sizeof(config_desc), r.wLength));
        break;
      case desc_string:
        switch (r.wValue & 0xff) {
        case string_index_manufacturer:
          ep_write(manufacturer_string_desc,
              min_u8(sizeof(manufacturer_string_desc), r.wLength));
          break;
        case string_index_product:
          ep_write(product_string_desc,
              min_u8(sizeof(product_string_desc), r.wLength));
          break;
        case string_index_serial:
          ep_write(serial_string_desc,
              min_u8(sizeof(serial_string_desc), r.wLength));
          break;
        default:
          ep_write(lang_string_desc,
              min_u8(sizeof(lang_string_desc), r.wLength));
          break;
        }
        break;
      case desc_device_qualifier:
        // optional, not implemented.
        UECONX |= (_BV(STALLRQ) | _BV(EPEN));
        return;
      default:
        state = state_error;
        break;
      }
      break;
    case req_set_configuration:
      break;
    default:
      state = state_error;
      break;
    }
  } else if (r.bmRequestType.b.type == type_class &&
      r.bmRequestType.b.receptor == recp_interface) {
    switch (r.bRequest) {
    case req_cdc_set_line_coding:
      while (!(UEINTX & _BV(RXOUTI)));
      if (r.wLength == sizeof(struct line_coding)) {
        struct line_coding lc;
        ep_read(&lc, sizeof(lc));
      } else {
        state = state_error;
      }
      break;
    case req_cdc_set_control_line_state:
      state = state_ready;
      break;
    default:
      state = state_error;
      break;
    }
  } else {
    state = state_error;
  }
  if (state == state_error)
    UECONX |= (_BV(STALLRQ) | _BV(EPEN));
  else
    UEINTX = ~_BV(TXINI);
}

void
con_init
(void)
{
  // Skip initialization if the USB module is already enabled in order to keep
  // the CDC functionality available during software resets.
  if (USBCON & _BV(USBE))
    return;

  led_on();

  // Setup and enable USB module.
#if defined(MCU_32U4)
  UHWCON |= _BV(UVREGE);
#endif
  USBCON |= _BV(USBE) | _BV(FRZCLK);

  // Setup clocks and PLL.
  CLKPR = _BV(CLKPCE);
  CLKPR = 0;
  CLKSEL0 |= _BV(EXTE);
  CLKSEL1 |= _BV(EXCKSEL3) | _BV(EXCKSEL2) | _BV(EXCKSEL1) | _BV(EXCKSEL0);
  while (!(CLKSTA & _BV(EXTON)));
  CLKSEL0 |= _BV(CLKS);
  PLLCSR |= _BV(PINDIV) | _BV(PLLE);
  while (!(PLLCSR & _BV(PLOCK)));
  CLKSEL0 &= ~_BV(RCE);

  // Enable USB module.
  USBCON &= ~_BV(FRZCLK);
#if defined(MCU_32U4)
  USBCON |= _BV(USBE) | _BV(OTGPADE);
#else
  USBCON |= _BV(USBE);
#endif

  // Attach device.
  UDCON = 0;

  // Enable interrupts.
  UDIEN = _BV(EORSTE) | _BV(SOFE);
  sei();

  led_blink();
  while (state != state_ready);
  led_off();
}

void
con_putchar
(unsigned char c)
{
  uint8_t next_index = (tx_wr_index + 1) & tx_index_mask;
  while (next_index == tx_rd_index);  // buffer full
  tx_buf[tx_wr_index] = c;
  tx_wr_index = next_index;
  cli();
  UENUM = 3;
  UEIENX |= _BV(TXINE);
  sei();
}

int
con_getchar
(void)
{
  if (rx_wr_index == rx_rd_index) return -1;
  int rc = rx_buf[rx_rd_index];
  rx_rd_index = (rx_rd_index + 1) & rx_index_mask;
  return rc;
}

int
con_peek
(void)
{
  return (rx_wr_index - rx_rd_index) & rx_index_mask;
}
