#include <SoftwareSerial.h>

SoftwareSerial portOne(10, 11);
int txEn = 8;

uint8_t scan1[7]  = {0xF8,0x2B,0x0E,0x01,0x00,0xEC,0x63};
uint8_t active[7] = {0xF8,0x00,0x09,0x00,0x03,0xDC,0xC8};
uint8_t connect[8]= {0xF8, 0x03, 0x00, 0x00, 0x00, 0x01, 0x90, 0x63};  // response F8 03 02 80 00 45 90
uint8_t ready[8] =   {0xF8, 0x06, 0x21, 0x35, 0x00, 0x06, 0x07, 0x93};  // Ready
uint8_t swOn[8] = {0xF8, 0x06, 0x21, 0x35, 0x00, 0x07, 0xC6, 0x53};     // Switch On
uint8_t swEn[8] = {0xF8, 0x06, 0x21, 0x35, 0x00, 0x0F, 0xC7, 0x95};     // Enable
uint8_t rSF[8]  =   {0xF8, 0x06, 0x21, 0x35, 0x00, 0x00, 0x87, 0x91};
uint8_t rSF2[8]  =   {0xF8, 0x06, 0x21, 0x35, 0x00, 0x80, 0x86, 0x31};
uint8_t readReg[8]= {0xF8, 0x03, 0x0C, 0x1E, 0x00, 0x04, 0x33, 0x36}; // read holding reg dari 0x0C1E s/d 0x0C21
uint8_t set20[8] =   {0xF8, 0x06, 0x21, 0x36, 0x00, 0xC8, 0x76, 0x07};
uint8_t readSP[8]=  {0xF8, 0x03, 0x21, 0x36, 0x00, 0x01, 0x7A, 0x51}; 



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
  pinMode(txEn, OUTPUT);
}

void loop() {
  while (Serial.available() > 0) {
    uint8_t inByte = Serial.read();
    
    switch (inByte){
      case 's':                   // Scan device
        digitalWrite(txEn, HIGH);
        for(uint8_t i=0;i<7;i++){
          portOne.write(scan1[i]);
        }
        digitalWrite(txEn, LOW);
        break;
        
      case 'c':                   // rSF
        digitalWrite(txEn, HIGH);
        for(uint8_t i=0;i<8;i++){
          portOne.write(rSF2[i]);
        }
        digitalWrite(txEn, LOW);
        break;
        
      case 'r':                   // Ready
        digitalWrite(txEn, HIGH);
        for(uint8_t i=0;i<8;i++){
          portOne.write(ready[i]);
        }
        digitalWrite(txEn, LOW);
        
        delay(6);
        digitalWrite(txEn, HIGH); // switch On
        for(uint8_t i=0;i<8;i++){
          portOne.write(swOn[i]);
        }
        digitalWrite(txEn, LOW);
        
        delay(6);
        digitalWrite(txEn, HIGH); // enable
        for(uint8_t i=0;i<8;i++){
          portOne.write(swEn[i]);
        }
        digitalWrite(txEn, LOW);
        
        break;

       case 't':                   // Read register
        digitalWrite(txEn, HIGH);
        for(uint8_t i=0;i<8;i++){
          portOne.write(readReg[i]);
        }
        digitalWrite(txEn, LOW);
        break;

       case 'f':                   // Read frequensi set point (Address 8502 = 0x2136)
        digitalWrite(txEn, HIGH);
        for(uint8_t i=0;i<8;i++){
          portOne.write(readSP[i]);
        }
        digitalWrite(txEn, LOW);
        break;

       case 'g':                   // Write frequensi set point (Address 8502 = 0x2136) nilai 20Hz = 200 desimal = 0x00C8
        digitalWrite(txEn, HIGH);
        for(uint8_t i=0;i<8;i++){
          portOne.write(set20[i]);
        }
        digitalWrite(txEn, LOW);
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
          digitalWrite(txEn, HIGH);
          for(uint8_t j=0;j<sizeof(frame);j++){
            portOne.write(frame[j]);
          }
          digitalWrite(txEn, LOW);
        }
        break;
    }
  }
  while(portOne.available()>0){
    uint8_t inByte = portOne.read();
    Serial.print(inByte, HEX);
  }
}
