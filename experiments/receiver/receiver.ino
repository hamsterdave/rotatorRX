struct payload {
  uint8_t limit_A : 1;
  uint8_t limit_B : 1;
  uint8_t limit_C : 1;
  int16_t flux_X;
  int16_t flux_Y;
  int16_t flux_Z;
};

uint8_t packet[64];
uint8_t packet_len;
uint8_t decoded_data[64];
bool done;

void cobs_decode(const uint8_t *src, size_t length, uint8_t *dst)
{
  const uint8_t *end = src + length;

  while (src < end) {
    int code = *src++;
    for (int i = 1; i < code; i++) *dst++ = *src++;
    if (code < 255) *dst++ = 0;
  }
}

void serialEvent() {
  uint8_t b;
  static bool reading = false;
  while (Serial.available()) {
    b = Serial.read();
    if (!reading) {
      // initial state, look for zero
      if (b == 0) reading = true;
    } else {
      // read valid (non-zero) data
      if (b == 0) {
        if (packet_len > 0) {
          cobs_decode(packet, packet_len, decoded_data);
          packet_len = 0;
          done = true;
        }
        reading = false;
      } else {
        packet[packet_len++] = b;
        if (packet_len == 15) { // too long, reset
          packet_len = 0;
          reading = false;
        }
      }
    }
  }
}

void setup()
{
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  packet_len = 0;
  done = false;
}

void loop()
{
  struct payload expected = { 1, 0, 0, 25, -12, 2348 };
  struct payload *received = (struct payload*)decoded_data;
  
  if (done) {
    if (expected.limit_A == received->limit_A &&
        expected.limit_B == received->limit_B &&
        expected.limit_C == received->limit_C &&
        expected.flux_X == received->flux_X &&
        expected.flux_Y == received->flux_Y &&
        expected.flux_Z == received->flux_Z) {
      for (byte i = 0; i < 5; i++) {
        digitalWrite(13, HIGH);
        delay(250);
        digitalWrite(13, LOW);
        delay(250);
      }
    }
    done = false;
  }
}
