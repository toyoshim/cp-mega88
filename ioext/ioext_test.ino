/*
 * Copyright (c) 2019, Takashi TOYOSHIMA <toyoshim@gmail.com>
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
//
// IOEXT supporting device example for Arduino UNO
//
#define nDATA 13
#define nREADY 12
#define nIOE 11
#define nW 10
static const int addr[] = { 2, 3, 4, 5, 6, 7, 8, 9 };

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 8; ++i)
    pinMode(addr[i], INPUT);
  pinMode(nDATA, INPUT);
  pinMode(nREADY, INPUT);
  pinMode(nIOE, INPUT);
  pinMode(nW, INPUT);
  Serial.println("start");
}

void loop() {
  // Wait until nIOE is activated.
  while (HIGH == digitalRead(nIOE));

  // Port is exposed as D[7:0].
  uint8_t port = 0;
  for (int i = 0; i < 8; ++i)
    port |= ((digitalRead(addr[i]) == HIGH) ? (1 << i) : 0);
  Serial.print("port: ");
  Serial.println(port, HEX);

  // Check if this is a port phase.
  if (HIGH != digitalRead(nDATA))
    Serial.println("error: port");

  // Also check /W to know if this is IN or OUT.
  uint8_t r = digitalRead(nW);
  if (r == HIGH)
    Serial.println("in");
  else
    Serial.println("out");

  // OK, everything were checked. Let's notify the host.
  // Activate /READY will move phase forward.
  pinMode(nREADY, OUTPUT);
  digitalWrite(nREADY, LOW);
  // Wait until nIOE is inactivated
  while (LOW == digitalRead(nIOE));
  // OK, host acked. /READY can and must be inactivated.
  pinMode(nREADY, INPUT);

  // Wait until nIOE is activated again for the data phase.
  while (HIGH == digitalRead(nIOE));

  if (r == HIGH) {
    // For IN, now data bus is released for the device. Let's output data.
    for (int i = 0; i < 8; ++i) {
      pinMode(addr[i], OUTPUT);
      digitalWrite(addr[i], 1);
    }
  } else {
    // For OUT, now data is exposed as D[7:0] to be read.
    uint8_t data = 0;
    for (int i = 0; i < 8; ++i)
      data |= ((digitalRead(addr[i]) == HIGH) ? (1 << i) : 0);
    Serial.print("data: ");
    Serial.println(data, HEX);
  }

  // OK, data is set, or read. Activate /READY to notify the host.
  pinMode(nREADY, OUTPUT);
  digitalWrite(nREADY, LOW);
  // Wait until nIOE is inactivated.
  while (LOW == digitalRead(nIOE));
  // OK, host acked. /READY can and must be inactivated. For IN, data must be
  // done too.
  if (r == HIGH) {
    for (int i = 0; i < 8; ++i)
      pinMode(addr[i], INPUT);
  }
  pinMode(nREADY, INPUT);

  // Everything is done!
}
