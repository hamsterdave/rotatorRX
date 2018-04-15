#include <stdint.h>
#include <stdio.h>

struct payload {
  uint8_t limit_A : 1;
  uint8_t limit_B : 1;
  uint8_t limit_C : 1;
  int16_t flux_X;
  int16_t flux_Y;
  int16_t flux_Z;
};

// consistent overhead byte stuffing encoder
// encodes length data from src into dst
void cobs_encode(const uint8_t *src, size_t len, uint8_t *dst)
{
  const uint8_t *end = src + len;
  uint8_t *code = dst++;
  *code = 1;

  while (src < end) {
    if (*src == 0) {
      code = dst++;
      *code = 1;
    } 
    else {
      *dst++ = *src;
      (*code)++;
      if (*code == 255) {
        code = dst++;
        *code = 1;
      }
    }
    src++;
  }
}

// send a cobs encoded data packet with leading and trailing zeroes
// len MUST be <=60
void send_data(uint8_t *data, size_t len)
{
  uint8_t packet[64];

  // zeroes at start and end of packet
  // data is always len + 1 long
  // [ 0 | ..len + 1.. | parity | 0 ]
  packet[0] = 0;
  packet[len+3] = 0;
  
  // calculate checksum
  uint8_t *parity = data + len;
  *parity = 0;
  for (int i = 0; i < len; i++) {
    *parity ^= data[i];
  }

  // encode the data (plus parity) between the zeroes
  cobs_encode(data, len+1, packet+1);

  // send packet
  Serial1.write(packet, len+4);
}

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(7, INPUT);
}

void loop()
{
  uint8_t data[64];
  struct payload *out = (struct payload*)data;
  
  out->limit_A = 1;
  out->limit_B = 0;
  out->limit_C = 0;
  out->flux_X = 25;
  out->flux_Y = -12;
  out->flux_Z = 2348;
  
  while (digitalRead(7) == 0);
  
  send_data((uint8_t*)out, sizeof(struct payload));
  delay(5000);
}
