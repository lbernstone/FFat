#include <rom/crc.h>
void setup() {
  uint32_t tester[]={0,0x84000,0x1000,0x1000,0x10,0x10,2,0x20};
  Serial.begin(115200);
  Serial.println(crc32_le(0, (uint8_t*)tester, 32), HEX);
}

void loop() {
  // put your main code here, to run repeatedly:

}
