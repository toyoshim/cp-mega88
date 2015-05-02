# Schematic #
<a href='http://cp-mega88.googlecode.com/files/mega88_sch.png'><img src='http://cp-mega88.googlecode.com/files/mega88_sch.png' width='800' /></a>

# ATMEGA88 pin assignment #
| **pin** | **common** | **isp** | **usart** | **sd card** | **74hc374-0** | **74hc374-1** | **m68af127b** | **comment** |
|:--------|:-----------|:--------|:----------|:------------|:--------------|:--------------|:--------------|:------------|
|  1| /RST | /RST |  |  |  |  |  |  |
|  2|  |  |  |  | A00 | A08 | D0 |  |
|  3|  |  |  |  | A01 | A09 | D1 |  |
|  4|  |  |  |  | A02 | A10 | D2 |  |
|  5|  |  |  |  | A03 | A11 | D3 |  |
|  6|  |  |  |  | A04 | A12 | D4 |  |
|  7| VCC | VCC | VCC | VCC | VCC | VCC | VCC |  |
|  8| GND | GND | GND | GND | GND | GND | GND |  |
|  9| XTAL1 |  |  |  |  |  |  | 20MHz ceralock |
|10| XTAL2 |  |  |  |  |  |  | 20MHz ceralock |
|11|  |  |  |  | A05 | A13 | D5 |  |
|12|  |  |  |  | A06 | A14 | D6 |  |
|13|  |  |  |  | A07 | A15| D7 |  |
|14|  |  |  |  |  |  | /W | /G and /E1 is connected to GND |
|15|  |  |  |  |  |  | E2 | pull down to GND with 100Ω |
|16|  |  |  |  |  |  |  | reserved for A16 (connected but not used) |
|17|  | MOSI  |  |  |  |  |  | reserved for i8080 NMI (not used) |
|18|  | MISO |  |  | CLK |  |  |  |
|19|  | SCK |  |  |  | CLK |  |  |
|20| AVCC |  |  |  |  |  |  | connected to VCC |
|21| AREF |  |  |  |  |  |  | open (not used) |
|22| AGND |  |  |  |  |  |  | connected to GND |
|23|  |  | RXD |  |  |  |  | software usart |
|24|  |  | TXD |  |  |  |  | software usart |
|25|  |  |  | CLK |  |  |  | connected to GND via LED |
|26|  |  |  | DI |  |  |  |  |
|27|  |  |  | DO |  |  |  |  |
|28|  |  |  | CS |  |  |  |  |