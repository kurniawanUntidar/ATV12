#include <SoftwareSerial.h>

SoftwareSerial portOne(10, 11);
SoftwareSerial portTwo(8, 9);

uint8_t scan1[7]  = {0xF8,0x2B,0x0E,0x01,0x00,0xEC,0x63};
uint8_t scan2[7]  = {0x7F,0x2B,0x0E,0x01,0x00,0x58,0x7D};
uint8_t active[7] = {0xF8,0x00,0x09,0x00,0x03,0xDC,0xC8};
uint8_t connect[8]= {0xF8,0x03,0x00,0x00,0x00,0x01,0x90,0x63};  // response F8 03 02 80 00 45 90
uint8_t hold[] =    {0xF8,0x42,0x01,0x1D,0x2E,0x00};

typedef struct{
  uint8_t address;
  uint8_t requestCode;
  uint16_t firstWord;
  uint16_t lastWord;
  uint16_t crc16;  
}modbusRequest;

typedef struct{
  uint8_t address;
  uint8_t requestCode;
  uint8_t *data;
  uint16_t crc16;  
}modbusResponse;

uint16_t crc16_modbus(const uint8_t *data, uint16_t length) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < length; i++) {
    crc ^= data[i]; // XOR byte ke dalam LSB CRC
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc = (crc >> 1) ^ 0xA001; // jika LSB 1, shift dan XOR dengan polinomial
        } else {
          crc = crc >> 1; // jika tidak, hanya geser
          }
        }
    }
    return crc;
}


void setup() {
  Serial.begin(19200);
  portOne.begin(19200);
  portTwo.begin(19200);
}

void loop() {
  portOne.listen();
  while (Serial.available() > 0) {
    uint8_t inByte = Serial.read();
    switch (inByte){
      case 's':
        for(uint8_t i=0;i<7;i++){
          portOne.write(scan1[i]);
        }
        break;
      case 'c':
        for(uint8_t i=0;i<8;i++){
          portOne.write(connect[i]);
        }
        break;
      case 'a':
        for(uint8_t i=0;i<7;i++){
          portOne.write(active[i]);
        }
        break;
      case 'h':
        for(uint8_t i=0;i<256;i++){
          uint8_t hold[] =    {0xF8,0x42,0x01,i,0x2E,0x00};
          uint16_t CRC = crc16_modbus(hold,sizeof(hold));
          uint8_t crc_lo = CRC & 0xFF;       // LSB
          uint8_t crc_hi = (CRC >> 8) & 0xFF; // MSB
          
          //------hitung CRC dan masukan kedalam frame data
          //------panjang data dari array hold[] disimpan dalam variabel dataLength
          //------membuat array baru (frame) dengan lebar length+2 untuk menyimpan crc_lo dan crc_hi
          uint8_t dataLength = sizeof(hold);
          uint8_t frame[dataLength+2];
          memcpy(frame,hold, dataLength);
          frame[dataLength] = crc_lo;
          frame[dataLength+1] = crc_hi;
          for(uint8_t j=0;j<sizeof(frame);j++){
            portOne.write(frame[j]);
          }
        }
        break;
    }
  }
  while(portOne.available()>0){
    uint8_t inByte = portOne.read();
    Serial.write(inByte);
  }
}
