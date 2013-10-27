/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int shiftClearPin = 14;
int shiftClkPin = 15;
int shiftRegPin = 16;
int shiftOutPin = 17;

int romOutEnablePin = 18;
int romWriteEnablePin = 19;

int shiftBits = 12;

char *ack = "eepeepack";
char *cmd_ack = "eepeepcmd";
char cmd_clear = 'c';
char cmd_write = 'w';
char cmd_dump = 'd';

const unsigned long CHIP_SIZE = 0x40000;

void shiftInt(int num){
  digitalWrite(shiftClkPin, LOW);
  digitalWrite(shiftRegPin, LOW);
  shiftOut(shiftOutPin, shiftClkPin, MSBFIRST, num >> 8);
  shiftOut(shiftOutPin, shiftClkPin, MSBFIRST, num & 0xFF);
  digitalWrite(shiftRegPin, HIGH);
}

void writeAddress(unsigned long address){
  shiftInt(address >> 4);
  
  PORTB = (PORTB & B11000011) | ((address << 2) & B00111100);
}

void writeData(unsigned char data){
  // DDRB0,1 to write
  DDRB = (DDRB & 0xFC) | 0x03;
  // DDRD2-7 to write
  DDRD = (DDRD & 0x03) | 0xFC;
  
  PORTB = data & 0x03;
  PORTD = data & 0xFC;
}

void writeByte(unsigned long address, unsigned char data) {
  digitalWrite(romOutEnablePin, HIGH);
  digitalWrite(romWriteEnablePin, HIGH);
  writeAddress(address);
  digitalWrite(romWriteEnablePin, LOW);
  writeData(data);
  digitalWrite(romWriteEnablePin, HIGH);
}

unsigned char readData(){
  // DDRB0,1 to read
  DDRB = DDRB & 0xFC;
  // DDRD2-7 to read
  DDRD = DDRD & 0x03;
  
  PORTB = (PORTB & 0xFC) | 0x03;
  PORTD = (PORTD & 0x03) | 0xFC;
  
  return (PIND & 0xFC) | (PINB & 0x03);
}

unsigned char readByte(unsigned long address) {
  digitalWrite(romWriteEnablePin, HIGH);
  digitalWrite(romOutEnablePin, LOW);
  writeAddress(address);
  return readData();
}

void burnByte(unsigned long address, unsigned char data) {
  char written = 0;
  while(!written){
    writeByte(0x555, 0xAA);
    writeByte(0x2AA, 0x55);
    writeByte(0x555, 0xA0);
    writeByte(address, data);
    if((readData() & 0x70) == (data & 0x70)) {
      written = 1;
    }
  }
}

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);
  
  // initialize the digital pin as an output.
  pinMode(shiftClearPin, OUTPUT);
  pinMode(shiftClkPin, OUTPUT);
  pinMode(shiftRegPin, OUTPUT);
  pinMode(shiftOutPin, OUTPUT);
  
  pinMode(romOutEnablePin, OUTPUT);
  pinMode(romWriteEnablePin, OUTPUT);
  digitalWrite(romWriteEnablePin, HIGH);
  digitalWrite(romOutEnablePin, LOW);
  
  // PORTB 2-5 (pins 10-13) are the LSBs of the address output
  // PORTB 0-1 (pins 8-9) are the LSBs of the data pins
  DDRB |= B00111111;
  
  // PORTB 0-1 (LSB) and PORTD 2-7 (MSB) are the data pins
  // PORTD 0-1 are the serial pins that we can't use :P
  DDRD |= 0xFC;
  
  digitalWrite(shiftClearPin, HIGH);
}

void clearROM(){
  writeByte(0x555,0xAA);
  writeByte(0x2AA,0x55);
  writeByte(0x555,0x80);
  writeByte(0x555,0xAA);
  writeByte(0x2AA,0x55);
  writeByte(0x555,0x10);
  for(int i = 0; i < 10; i++){
    Serial.println(i);
    delay(1000);
  }
}

// the loop routine runs over and over again forever:
void loop() {
  int waiting = 1;
  while(waiting) {
    Serial.print(ack);
    if(Serial.find(ack)) {
        waiting = 0;
    }
  }
  char cmd;

  Serial.readBytes(&cmd, 1);

  Serial.write(cmd_ack);
  Serial.write(cmd);

  if(cmd == cmd_clear) {
    clearROM();
  } else if(cmd == cmd_dump) {
      unsigned long address = 0;
      for(address = 0; address < CHIP_SIZE; address++) {
        byte data = readByte(address);
        //Serial.println(data, HEX);
        Serial.write(data);
      }
  } else if(cmd == cmd_write) {
      unsigned long address;
      for(address = 0; address < CHIP_SIZE; address++) {
          char data, check;
          Serial.readBytes(&data, 1);
          do {
              burnByte(address,data);
              check = readByte(address);
              Serial.write(check);
          } while(check != data);
          //Serial.print(" ");
          //Serial.println(data,HEX);
      }
  }
}
