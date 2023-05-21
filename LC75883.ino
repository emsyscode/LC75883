/*
This code is not clean and far from perfect, that's just
a reference to extract ideas and adapt to your solution.
you can replace the BIN values with HEX... I leave it in BIN
because it is easier to relate the segment number with
  the position of the bit in BIN.
Of course, a library can be created for this purpose! But I won't 
take the time to do that, I'll leave it up to you!
*/
#include <Arduino.h>
#include <stdio.h>
//#include <math.h>
#include <stdbool.h>

void send_char(unsigned char a);
void send_data(unsigned char a);
void segments();
void buttonReleasedInterrupt();  

#define LCD_in 8  // This is the pin number 8 on Arduino UNO
#define LCD_clk 9 // This is the pin number 9 on Arduino UNO
#define LCD_CE 10 // This is the pin number 10 on Arduino UNO

//unsigned int numberSeg = 0;  // Variable to supporte the number of segment
//unsigned int numberByte = 0; // Variable to supporte the number byte 
unsigned int shiftBit=0;
unsigned int nBitOnBlock=0; // Used to count number of bits and split to 8 bits... (number of byte)
unsigned int nByteOnBlock=0; 
unsigned int sequencyByte=0x00;
byte Aa,Ab,Ac,Ad,Ae,Af,Ag;
byte blockBit =0x00;
byte nSeg=0x00;

// constants won't change. They're used here to set pin numbers:
//const int buttonPin = 7;  // the number of the pushbutton pin
const int ledPin = 12;    // the number of the LED pin

#define BUTTON_PIN 2 //Att check wich pins accept interrupts... Uno is 2 & 3
volatile byte buttonReleased = false;

// variables will change:
int buttonState = 0;  // variable for reading the pushbutton status

bool forward = false;
bool backward = false;
bool isRequest = true;
bool allOn=false;
bool cycle=false;
/*
#define BIN(x) \
( ((0x##x##L & 0x00000001L) ? 0x01 : 0) \
| ((0x##x##L & 0x00000010L) ? 0x02 : 0) \
| ((0x##x##L & 0x00000100L) ? 0x04 : 0) \
| ((0x##x##L & 0x00001000L) ? 0x08 : 0) \
| ((0x##x##L & 0x00010000L) ? 0x10 : 0) \
| ((0x##x##L & 0x00100000L) ? 0x20 : 0) \
| ((0x##x##L & 0x01000000L) ? 0x40 : 0) \
| ((0x##x##L & 0x10000000L) ? 0x80 : 0))
*/

//ATT: On the Uno and other ATMEGA based boards, unsigned ints (unsigned integers) are the same as ints in that they store a 2 byte value.
//Long variables are extended size variables for number storage, and store 32 bits (4 bytes), from -2,147,483,648 to 2,147,483,647.

//*************************************************//
void setup() {
  pinMode(LCD_clk, OUTPUT);
  pinMode(LCD_in, OUTPUT);
  pinMode(LCD_CE, OUTPUT);

  pinMode(13, OUTPUT);
  
// initialize the LED pin as an output:
pinMode(ledPin, OUTPUT);
// initialize the pushbutton pin as an input:
//pinMode(buttonPin, INPUT);  //Next line is the attach of interruption to pin 2
pinMode(BUTTON_PIN, INPUT);
 attachInterrupt(digitalPinToInterrupt(BUTTON_PIN),
                  buttonReleasedInterrupt,
                  FALLING);
//Dont insert any print inside of interrupt function!!!
//If you run the search function, please active the terminal to be possible print lines,
//Other way the run will be blocked!
//
  Serial.begin(115200);
  
  /*CS12  CS11 CS10 DESCRIPTION
  0        0     0  Timer/Counter1 Disabled 
  0        0     1  No Prescaling
  0        1     0  Clock / 8
  0        1     1  Clock / 64
  1        0     0  Clock / 256
  1        0     1  Clock / 1024
  1        1     0  External clock source on T1 pin, Clock on Falling edge
  1        1     1  External clock source on T1 pin, Clock on rising edge
 */
  
// Note: this counts is done to a Arduino 1 with Atmega 328... Is possible you need adjust
// a little the value 62499 upper or lower if the clock have a delay or advnce on hours.

  digitalWrite(LCD_CE, LOW);
  delayMicroseconds(5);
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);
}
void send_char(unsigned char a)
{
 unsigned char transmit = 15; //define our transmit pin
 unsigned char data = 170; //value to transmit, binary 10101010
 unsigned char mask = 1; //our bitmask
  data=a;
  // the validation of data happen when clk go from LOW to HIGH.
  // This lines is because the clk have one advance in data, see datasheet of sn74HC595
  // case don't have this signal instead of "." will se "g"
  digitalWrite(LCD_CE, LOW); // When strobe is low, all output is enable. If high, all output will be set to low.
  delayMicroseconds(5);
  digitalWrite(LCD_clk,LOW);// need invert the signal to allow 8 bits is is low only send 7 bits
  delayMicroseconds(5);
  for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
  digitalWrite(LCD_clk,LOW);// need invert the signal to allow 8 bits is is low only send 7 bits
  delayMicroseconds(5);
    if (data & mask){ // if bitwise AND resolves to true
      digitalWrite(LCD_in, HIGH);
      //Serial.print(1);
    }
    else{ //if bitwise and resolves to false
      digitalWrite(LCD_in, LOW);
      //Serial.print(0);
    }
    digitalWrite(LCD_clk,HIGH);// need invert the signal to allow 8 bits is is low only send 7 bits
    delayMicroseconds(5);
    //
    digitalWrite(LCD_CE, HIGH); // When strobe is low, all output is enable. If high, all output will be set to low.
  delayMicroseconds(5);
  }
}
// I h've created 3 functions to send bit's, one with strobe, other without strobe and one with first byte with strobe followed by remaing bits.
void send_char_without(unsigned char a)
{
 //
 unsigned char transmit = 15; //define our transmit pin
 unsigned char data = 170; //value to transmit, binary 10101010
 unsigned char mask = 1; //our bitmask
  data=a;
  for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
  digitalWrite(LCD_clk, LOW);
  delayMicroseconds(5);
    if (data & mask){ // if bitwise AND resolves to true
      digitalWrite(LCD_in, HIGH);
      //Serial.print(1);
    }
    else{ //if bitwise and resolves to false
      digitalWrite(LCD_in, LOW);
      //Serial.print(0);
    }
    digitalWrite(LCD_clk,HIGH);// need invert the signal to allow 8 bits is is low only send 7 bits
    delayMicroseconds(5);
  }
}
void send_char_8bit_stb(unsigned char a)
{
 //
 unsigned char transmit = 15; //define our transmit pin
 unsigned char data = 170; //value to transmit, binary 10101010
 unsigned char mask = 1; //our bitmask
 int i = -1;
  data=a;
  // This lines is because the clk have one advance in data, see datasheet of sn74HC595
  // case don't have this signal instead of "." will se "g"
  for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
   i++;
   digitalWrite(LCD_clk, LOW);
  delayMicroseconds(5);
    if (data & mask){ // if bitwise AND resolves to true
      digitalWrite(LCD_in, HIGH);
      //Serial.print(1);
    }
    else{ //if bitwise and resolves to false
      digitalWrite(LCD_in, LOW);
      //Serial.print(0);
    }
    digitalWrite(LCD_clk,HIGH);// need invert the signal to allow 8 bits is is low only send 7 bits
    delayMicroseconds(1);
    if (i==7){
    //Serial.println(i);
    digitalWrite(LCD_CE, HIGH);
    delayMicroseconds(2);
    }
     
  }
}
//
void allON(){
//0B: 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
 for(int i=0; i<3;i++){   // 
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          
          send_char_without(0B11111111);  send_char_without(0B11111111);  //   1:8      -9:16// 
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  17:24    -25:32// 
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  33:40    -41:48//  
          send_char_without(0B11111111);  send_char_without(0B00000011);  //  49-56
              switch (i){ //Last 3 bits is "DD" data direction, and is used
                case 0: send_char_without(0B00100100); break;
                case 1: send_char_without(0B10000000); break;
                case 2: send_char_without(0B01000000); break;
              }
      // to mark the 3 groups of 120 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
      }
}
void allOFF(){
//0B: 0, 0, 0, 0, 0, S0, S1, last --> K0, K1, P0, P1, SC, DR, 0, 0
for(int i=0; i<3;i++){ // Dx until 600 bits
digitalWrite(LCD_CE, LOW); //
delayMicroseconds(1);
send_char_8bit_stb(0B01000010); //(0x41) firts 8 bits is address, every fixed as (0B01001011), see if clk finish LOW or HIGH Very important!
delayMicroseconds(1);
// On the 75878 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
    send_char_without(0B00000000);  send_char_without(0B00000000);  //   1:8      -9:16// 
    send_char_without(0B00000000);  send_char_without(0B00000000);  //  17:24    -25:32//  
    send_char_without(0B00000000);  send_char_without(0B00000000);  //  33:40    -41:48// 
    send_char_without(0B00000000);  send_char_without(0B00000000);  //49-57
        switch (i){ //120:127// Last 3 bits is "DD" data direction, and is used
          case 0: send_char_without(0B00100100); break;
          case 1: send_char_without(0B10000000); break;
          case 2: send_char_without(0B01000000); break;
        }
// to mark the 3 groups of 120 bits, 00, 01, 01.
delayMicroseconds(1);
digitalWrite(LCD_CE, LOW); // 

 }
}
void searchOfSegments(){
// put your main code here, to run repeatedly:
   int group = 0x00;
  byte nBit =0x00;
  byte nMask = 0b00000001;
  unsigned int block =0;
  for(block=0; block<3; block++){
for( group=0; group<7; group++){   // Do until n bits 
        //for(int nBit=0; nBit<8; nBit++){
          for (nMask = 0b00000001; nMask>0; nMask <<= 1){
            Aa=0x00; Ab=0x00; Ac=0x00; Ad=0x00; Ae=0x00; Af=0x00; Ag=0x00;
                  switch (group){
                    case 0: Aa=nMask; break;
                    case 1: Ab=nMask; break;//atoi(to integer)
                    case 2: Ac=nMask; break;
                    case 3: Ad=nMask; break;
                    case 4: Ae=nMask; break;
                    case 5: Af=nMask; break;
                    case 6: Ag=nMask; break;
                  }
            
           nSeg++;
           if((nSeg >=0) && (nSeg<57)){
            blockBit=0;
            }
            if((nSeg >=57) && (nSeg<114)){
            blockBit=1;
            }
            if((nSeg >=114) && (nSeg<171)){
            blockBit=2;
            }
            if (nSeg >171){
              nSeg=0;
              group=0;
              block=0;
              break;
            }
          
      //This start the control of button to allow continue teste! 
                      while(1){
                            if(!buttonReleased){
                              delay(200);
                            }
                            else{
                              delay(15);
                               buttonReleased = false;
                               break;
                               }
                         }
               
                     segments();
            Serial.print(nSeg, DEC); Serial.print(", group: "); Serial.print(group, DEC);Serial.print(", nMask: "); Serial.println(nMask, BIN);
            delay (400);  
                }         
           }        
      }
  }
void segments(){
//0B: 0, 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
Serial.print(Aa, HEX);Serial.print(Ab, HEX);Serial.print(Ac, HEX);Serial.print(Ad, HEX);Serial.print(Ae, HEX);Serial.print(Af, HEX);Serial.println(Ag, HEX);
Serial.print("BlockBit: "); Serial.println(blockBit, HEX);
//Serial.print(Ba, HEX);Serial.print(Bb, HEX);Serial.print(Bc, HEX);Serial.print(Bd, HEX);Serial.print(Be, HEX);Serial.print(Bf, HEX);Serial.println(Bg, HEX);
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75883 the message have first 7*8 bits (last 2 byte is control: 0B: 0, 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0)
          send_char_without(Aa);  send_char_without(Ab);  //   1:8      -9:16// 
          send_char_without(Ac);  send_char_without(Ad);  //  17:24    -25:32// 
          send_char_without(Ae);  send_char_without(Af);  //  33:40    -41:48//  
          send_char_without(Ag);  send_char_without(0B00000000);  //  49-56
              switch (blockBit){ //Last 3 bits is "DD" data direction, and is used to mark the 3 groups of 57 bits, 00, 01, 10.                                 
                case 0: send_char_without(0B00000000); break; //Block 00
                case 1: send_char_without(0B10000000); break; //Block 01
                case 2: send_char_without(0B01000000); break; //Block 10
              }
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); //                   
}
void testModeAllGroups(){
//0B: 0, 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
Serial.print(Aa, HEX);Serial.print(Ab, HEX);Serial.print(Ac, HEX);Serial.print(Ad, HEX);Serial.print(Ae, HEX);Serial.print(Af, HEX);Serial.println(Ag, HEX);
//Serial.print(Ba, HEX);Serial.print(Bb, HEX);Serial.print(Bc, HEX);Serial.print(Bd, HEX);Serial.print(Be, HEX);Serial.print(Bf, HEX);Serial.println(Bg, HEX);
 for(int i=0; i<3;i++){   // 
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75883 the message have first 7*8 bits (last 2 byte is control: 0B: 0, 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0)
          send_char_without(Aa);  send_char_without(Ab);  //   1:8      -9:16// 
          send_char_without(Ac);  send_char_without(Ad);  //  17:24    -25:32// 
          send_char_without(Ae);  send_char_without(Af);  //  33:40    -41:48//  
          send_char_without(Ag);  send_char_without(0B00000000);  //  49-56
              switch (i){ //Last 3 bits is "DD" data direction, and is used
                case 0: send_char_without(0B00000000); break;
                case 1: send_char_without(0B00000000); break;
                case 2: send_char_without(0B00000000); break;
              }
      // to mark the 3 groups of 57 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
      }
}
void seg1(){
//0B: 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          
          send_char_without(0B11111111);  send_char_without(0B11111111);  //   1:8      -9:16// 
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  17:24    -25:32// 
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  33:40    -41:48//  
          send_char_without(0B11111111);  send_char_without(0B00000001);  //  49-56 -(bit 57), 0, 0, 0, 0, 0, S0, S1
          send_char_without(0B00100000); //K0, K1, P0, P1, SC, DR, 0, 0
      // to mark the 3 groups of a totla of 171 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg2(){
//0B: 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  58:65    -66:73//
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  74:81    -82:89//  
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  90:97    -98:105//   
          send_char_without(0B11111111);  send_char_without(0B00000001);  //  106-113  -Controle
          send_char_without(0B10000000);  //K0, K1, P0, P1, SC, DR, 0, 0
      // to mark the 3 groups of a total of 171 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg3(){
//0B: 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75853 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  115:122    -123:130// 
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  131:138    -139:146// 
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  147:154    -155:162//  
          send_char_without(0B11111111);  send_char_without(0B00000001);  //  163-170 - Controle   -(bit171), 0, 0, 0, 0, 0, S0, S1
          send_char_without(0B01000000); //K0, K1, P0, P1, SC, DR, 0, 0
      // to mark the 3 groups of atotoal of 171 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void messageHiFolks_Block00W0(){
//0B: 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75883 the message have 57*8 bits more 3 times to performe 171 bits(last 2 byte is control)
          
          send_char_without(0B00000000);  send_char_without(0B00000110);  //   1:8      -9:16// 
          send_char_without(0B00101001);  send_char_without(0B01000001);  //  17:24    -25:32// 
          send_char_without(0B11100101);  send_char_without(0B00001101);  //  33:40    -41:48//  
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  49-56    --(bit57), 0, 0, 0, 0, 0, S0, S1
              //Last 3 bits is "DD" data direction, and is used
                 send_char_without(0B00000100); 
      // to mark the 3 groups of 171 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void messageHiFolks_Block00W1(){
//0B: 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75883 the message have 57*8 bits more 3 times to performe 171 bits(last 2 byte is control)
          
          send_char_without(0B00000000);  send_char_without(0B00000110);  //   1:8      -9:16// 
          send_char_without(0B00101001);  send_char_without(0B01000001);  //  17:24    -25:32// 
          send_char_without(0B01100101);  send_char_without(0B00001101);  //  33:40    -41:48//  
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  49-56    --(bit57), 0, 0, 0, 0, 0, S0, S1
              //Last 3 bits is "DD" data direction, and is used
                 send_char_without(0B00000100); 
      // to mark the 3 groups of 171 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void messageHiFolks_Block00W2(){
//0B: 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75883 the message have 57*8 bits more 3 times to performe 171 bits(last 2 byte is control)
          
          send_char_without(0B00000000);  send_char_without(0B00000110);  //   1:8      -9:16// 
          send_char_without(0B00101001);  send_char_without(0B01000001);  //  17:24    -25:32// 
          send_char_without(0B11100101);  send_char_without(0B00000101);  //  33:40    -41:48//  
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  49-56    --(bit57), 0, 0, 0, 0, 0, S0, S1
              //Last 3 bits is "DD" data direction, and is used
                 send_char_without(0B00000100); 
      // to mark the 3 groups of 171 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void messageHiFolks_Block00W3(){
//0B: 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75883 the message have 57*8 bits more 3 times to performe 171 bits(last 2 byte is control)
          
          send_char_without(0B00000000);  send_char_without(0B00000110);  //   1:8      -9:16// 
          send_char_without(0B00101001);  send_char_without(0B01000001);  //  17:24    -25:32// 
          send_char_without(0B11100101);  send_char_without(0B00001001);  //  33:40    -41:48//  
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  49-56    --(bit57), 0, 0, 0, 0, 0, S0, S1
              //Last 3 bits is "DD" data direction, and is used
                 send_char_without(0B00000100); 
      // to mark the 3 groups of 171 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void messageHiFolks_Block00W4(){
//0B: 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75883 the message have 57*8 bits more 3 times to performe 171 bits(last 2 byte is control)
          
          send_char_without(0B00000000);  send_char_without(0B00000110);  //   1:8      -9:16// 
          send_char_without(0B00101001);  send_char_without(0B01000001);  //  17:24    -25:32// 
          send_char_without(0B11100101);  send_char_without(0B00001100);  //  33:40    -41:48//  
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  49-56    --(bit57), 0, 0, 0, 0, 0, S0, S1
              //Last 3 bits is "DD" data direction, and is used
                 send_char_without(0B00000100); 
      // to mark the 3 groups of 171 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void messageHiFolks_Block00(){
//0B: 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75883 the message have 57*8 bits more 3 times to performe 171 bits(last 2 byte is control)
          
          send_char_without(0B00000000);  send_char_without(0B00000110);  //   1:8      -9:16// 
          send_char_without(0B00101001);  send_char_without(0B01000001);  //  17:24    -25:32// 
          send_char_without(0B01100101);  send_char_without(0B00000000);  //  33:40    -41:48//  
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  49-56    --(bit57), 0, 0, 0, 0, 0, S0, S1
              //Last 3 bits is "DD" data direction, and is used
                 send_char_without(0B00000100); 
      // to mark the 3 groups of 171 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void messageHiFolks_Block10(){
//0B: 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75883 the message have 57*8 bits more 3 times to performe 171 bits(last 2 byte is control)
          
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  58:65      -66:73// 
          send_char_without(0B01001100);  send_char_without(0B11010000);  //  74:81    -82:89// 
          send_char_without(0B00000000);  send_char_without(0B00000011);  //  90:97    -98:105//  
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  106-113 - -(bit114), 0, 0, 0, 0, 0, S0, S1
               //Last 3 bits is "DD" data direction, and is used
                send_char_without(0B10000000); 
      // to mark the 3 groups of 171 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void messageHiFolks_Block01(){
//0B: 0, 0, 0, 0, S0, S1 --> K0, K1, P0, P1, SC, DR, 0, 0
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000010); //(0x42) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75883 the message have 57*8 bits more 3 times to performe 171 bits(last 2 byte is control)
          
          send_char_without(0B11000000);  send_char_without(0B00010100);  //  115:122    -123:130// 
          send_char_without(0B00000000);  send_char_without(0B11001011);  //  131:138    -139:146// 
          send_char_without(0B10000001);  send_char_without(0B00000001);  //  147:154    -155:162//  
          send_char_without(0B10000100);  send_char_without(0B00000000);  //  163-170    -(bit171), 0, 0, 0, 0, 0, S0, S1
              //Last 3 bits is "DD" data direction, and is used
                send_char_without(0B01000000);
      // to mark the 3 groups of 171 bits, 00, 01, 10.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}

void loop() {
long randNumber;
//buttonState = digitalRead(buttonPin);
//read the state of the pushbutton value:
//buttonState = digitalRead(buttonPin);

for(unsigned int c=0; c<2; c++){
  allON(); // All on
  delay(400);
  allOFF(); // All off
  delay(400);
 }

 allOFF();
 seg1();
 delay(1000);
 allOFF();
 seg2();
 delay(1000);
 allOFF();
 seg3();
 delay(1000);
 allOFF();
allOFF();

messageHiFolks_Block00();
messageHiFolks_Block10();
messageHiFolks_Block01();
//
      for(int i=0; i<20; i++){ 
      messageHiFolks_Block00W0();
      delay(60);
      messageHiFolks_Block00W1();
      delay(60);
      messageHiFolks_Block00W2();
      delay(60);
      messageHiFolks_Block00W3();
      delay(60);
      messageHiFolks_Block00W4();
      delay(60);
      }
//
//Uncomment this two lines to proceed identification of segments on this driver... adapt to other if necessary!
//Please don't forget of activation of Serial Monitor of IDE Arduino, to allow this running correctley,other way will block!
//allOFF();
//searchOfSegments(); 
}
void buttonReleasedInterrupt() {
  buttonReleased = true; // This is the line of interrupt button to advance one step on the search of segments!
}
