/* ===========================================================
 * Arduino-based Simon Electronic Minigame
 * ===========================================================
 * Copyright 2013 Matthew Hsu.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ========================================================== */
 
//Display mechanism for 4 7-segment com anode displays driven with 74lss164 shift registers w/ simon mini-game

//2/19/13

//DIGITAL PINS
#define data1 0
#define clock1 1
#define data2 2
#define clock2 3
#define data3 4
#define clock3 5
#define data4 6
#define clock4 7
#define led1 8
#define led2 9
#define led3 10
#define led4 11
#define buzzer 12

//ANALOG PINS
#define newgame 0
#define ss1 1
#define ss2 2
#define ss3 3
#define ss4 4

//CHAR DEFS (COM ANODDE)
byte a = B00010001;
byte b = B11000001;
byte d = B10000101;
byte e = B01100001;
byte g = B01000011;
byte h = B10010001;
byte i = B10011111;
byte j = B10000111;
byte k = B10100001;
byte l = B11100011;
byte m = B01010111;
byte n = B11010101;
byte o = B11000101;
byte p = B00110001;
byte r = B11110101;
byte s = B01001001;
byte t = B11100001;
byte u = B11000111;
byte w = B10101011;
byte y = B10001001;
byte none = B11111111;
byte all = B00000000;
byte dot = B11111110;

void output(byte a=B11111111,byte b=B11111111, byte c=B11111111, byte d=B11111111) {//outputs 4  predefined chars to each display
  //Shifting from A-H, ie LSB becomes h, MSB becomes A if LSBFIRST (74ls164)
  //7-segment displays - first 7 bits 0=high,1=low (com anode), bit 8 arbitrary
  shiftOut(data1, clock1, LSBFIRST, a);
  shiftOut(data2, clock2, LSBFIRST, b);
  shiftOut(data3, clock3, LSBFIRST, c);
  shiftOut(data4, clock4, LSBFIRST, d);  
}

int cycle(int datapin=0,int clockpin=0,int i=0) {//creates a cycle pattern
  //cycles through each segment A-F
  byte data[6] = {B01111111,B10111111,B11011111,B11101111,B11110111,B11111011};
   shiftOut(datapin, clockpin, LSBFIRST, data[i]);
   i = i++;
   if (i >5) i = 0;
   return i;
}

int number(int ii=0) {//integer to display
  byte data[10]={B00000011,B11110011,B00100101,B00001101,B10011001,B01001001,B01000001,B00011111,B00000001,B00011001};//digits 0-9
  return data[ii];
}

void setup() {
  pinMode(data1, OUTPUT);
  pinMode(clock1, OUTPUT);
  pinMode(data2, OUTPUT);
  pinMode(clock2, OUTPUT);
  pinMode(data3, OUTPUT);
  pinMode(clock3, OUTPUT);
  pinMode(data4, OUTPUT);
  pinMode(clock4, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(buzzer,OUTPUT);
  randomSeed(analogRead(5)); //random noise from disconnected
}

int triggered;//only allow button to be triggered once

void clear_lights() {
  triggered = 0;
  digitalWrite(led1,LOW);
  digitalWrite(led2,LOW);
  digitalWrite(led3,LOW);
  digitalWrite(led4,LOW);
}

int pos; //cycle position
int state; //0=default, 1=generate array, 2=playback, 3=user input, 4=finish message, 5=lose message
int stage; //round number
int steps; //steps to repeat
int maxStage = 10; //maximum rounds
int push;//which button-led pair pushed
int extra = 2; //how many more steps over the round number
int sequence[200];


void loop() {
  if (analogRead(newgame) > 500 && triggered == 0) {//check if new game button has been pressed
    stage=0;
    triggered = 1;
    state = 1;
    output(n,e,w,none);
    delay(2000);
  }
  
  if (state == 0) {//pending state
    cycle(data1,clock1,pos);
    cycle(data2,clock2,pos);
    cycle(data3,clock3,pos);
    cycle(data4,clock4,pos);
    pos++;
    if (pos >5) pos = 0;
    delay(150);
  }
  if (state == 1) {//pattern generation
    steps=stage+extra+1;//each stage has 3 more steps
    if (extra > 0 && sequence[0] == 0) {
      for (int ii=0; ii<extra; ii++) {
        sequence[ii]=random(1,5);//pick led to be high (1-4)
      }
    }
    sequence[steps-1]=random(1,5);
    state = 2;
  }
  
  if (state == 2) {//pattern display
    output(r,n,d,number(stage+1));
    delay(1000);
    for (int ii=0; ii<steps; ii++) {
      Serial.println("Step " + String(ii+1) + " : " + String(sequence[ii]));
      digitalWrite(sequence[ii]+(led1-1),HIGH); //sequence starts at 1, therefore led1-1+1 is led1 pin
      //delay(800-stage*45);
      delay(800-stage*75);//progressively flash faster each round
      digitalWrite(sequence[ii]+(led1-1),LOW);
      delay(200);
    }
    state = 3;
  }
  
  if (state == 3) { //user input
    output(none,g,o,none);
      for (int ii=0; ii<steps; ii++) {
        triggered = 0;
        while (triggered == 0){
          if (analogRead(ss1)>500) {
            push = 1;
            triggered = 1;
            digitalWrite(led1,HIGH);
            break;
          }
          if (analogRead(ss2)>500) {
            triggered = 1; 
            push = 2;
            digitalWrite(led2,HIGH);
            break;
          }
          if (analogRead(ss3)>500) {
            push= 3;
            triggered = 1;
            digitalWrite(led3,HIGH);
            break;
          }
          if (analogRead(ss4)>500) {
            push = 4;
            triggered=1;
            digitalWrite(led4,HIGH);
            break;
          }
        }
        Serial.println("Button " + String(push) + " down");
        while (analogRead(ss1) >500 || analogRead(ss2)>500 || analogRead(ss3)>500 || analogRead(ss4)>500) {}
        Serial.println("Button " + String(push) + " up");
        delay(300);
        clear_lights();
        if (sequence[ii] == push)
          continue;
        else {
          state = 5;
          break;
        }
      }
      if (state!=5)
        stage++;
      
      if (stage < maxStage && state!=5) {
        output(none,B01101111,B00001111,none);
        delay(1000);
        state = 1;
      }
      else if (stage >= maxStage) {
        state = 4;
        digitalWrite(buzzer,HIGH);
        delay(100);
        digitalWrite(buzzer,LOW);
      }
  }
  if (state == 4) {
  /*
    output(x,x,x,x);
    delay(1000);
    output(w,i,l,l);
    delay(1000);
    output(y,o,u);
    delay(1000);
    output(g,o,t,o);
    delay(1000);
    output(p,r,o,m);
    delay(1000);
    output(w,i,t,h);
    delay(1000);
    output(none,m,e,none);
    delay(1000);
	*/
    output(y,o,u);
    delay(1000);
    output(a,r,e);
    delay(1000);
    output(g,o,d);
    delay(1000);
  }
  
  if (state == 5) { // you dun goofed
    //output(n,o,p,e);
    output(none,B01101111,B01100011,none);
    digitalWrite(buzzer,HIGH);
    delay(500);
    digitalWrite(buzzer,LOW);
    delay(2000);
    state = 0;
  }
  clear_lights();
}


