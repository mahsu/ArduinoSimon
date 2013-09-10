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
#define data1 0     //data for display 1 (leftmost) - display segments
#define clock1 1    //clock for display 1 (leftmost) - bit pushing pulse
#define data2 2     //data for display 2
#define clock2 3    //clock for display 2
#define data3 4     //data for display 3
#define clock3 5    //data for display 3
#define data4 6     //data for display 4 (rightmost)
#define clock4 7    //clock for display 4 (rightmost)
#define led1 8      //led 1 (leftmost)
#define led2 9      //led 2
#define led3 10     //led 3
#define led4 11     //led 4
#define buzzer 12   //buzzer

//ANALOG PINS
#define newgame 0   //button to start a new game
#define ss1 1       //button 1 (leftmost)
#define ss2 2       //button 2
#define ss3 3       //button 3
#define ss4 4       //button 4 (rightmost)
#define rndseed 5   //random seed

//CHARACTER DEFINITIONS (COM ANODDE)
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

//INTERNAL VARIABLE INITIALIZATION
int triggered;//only allow button to be triggered once
int pos; //cycle position
int state; //0=default, 1=generate array, 2=playback, 3=user input, 4=finish message, 5=lose message
int stage; //round number
int steps; //steps to repeat
int push;//which button-led pair pushed


//GAME SETTINGS
int maxStage = 10; //maximum rounds
int extra = 2; //initial steps to start with
int sequence[200];//memory to allocate for light sequence





//outputs 4  predefined chars to each display
void output(byte a=B11111111,byte b=B11111111, byte c=B11111111, byte d=B11111111) {
  //Shifting from A-H, ie LSB becomes h, MSB becomes A if LSBFIRST (74ls164)
  //7-segment displays - first 7 bits 0=high,1=low (com anode), bit 8 is dot
  shiftOut(data1, clock1, LSBFIRST, a);
  shiftOut(data2, clock2, LSBFIRST, b);
  shiftOut(data3, clock3, LSBFIRST, c);
  shiftOut(data4, clock4, LSBFIRST, d); 
}

//creates a cycle pattern
int cycle(int datapin=0,int clockpin=0,int i=0) {
  //cycles through each segment A-F
  byte data[6] = {B01111111,B10111111,B11011111,B11101111,B11110111,B11111011};
   shiftOut(datapin, clockpin, LSBFIRST, data[i]);
   i = i++;
   if (i >5) i = 0;
   return i;
}

//display an integer
int number(int ii=0) {
  byte data[10]={B00000011,B11110011,B00100101,B00001101,B10011001,B01001001,B01000001,B00011111,B00000001,B00011001};//digits 0-9
  return data[ii];
}

//configure pin outputs
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
  randomSeed(analogRead(rndseed)); //take seed from value of light dependent resistor
}

//clear all leds
void clear_lights() {
  triggered = 0;
  digitalWrite(led1,LOW);
  digitalWrite(led2,LOW);
  digitalWrite(led3,LOW);
  digitalWrite(led4,LOW);
}

//state machine loop
void loop() {
  //check if new game button has been pressed
  if (analogRead(newgame) > 500 && triggered == 0) {
    stage=1;//start at stage 1
    triggered = 1;
    state = 1;
    output(n,e,w,none);//new
    delay(2000);
  }
  
  /**
  BEGIN STATE MACHINE
  **/
  
  //game pending state
  if (state == 0) {
    cycle(data1,clock1,pos);
    cycle(data2,clock2,pos);
    cycle(data3,clock3,pos);
    cycle(data4,clock4,pos);
    pos++;
    if (pos >5) pos = 0;
    delay(150);
  }
  
  //pattern generation state
  if (state == 1) {
    //the game will always start with "extra" steps
    //each round will append another step to the end
    steps=stage+extra;
    if (extra > 0 && sequence[0] == 0) {//first round
      for (int ii=0; ii<extra; ii++) {
        sequence[ii]=random(0,4);//pick led to be high (0-3)
      }
    }
    sequence[steps-1]=random(0,4);//add another to the previous pattern
    state = 2;
  }
  
  //pattern display state
  if (state == 2) {
    output(r,n,d,number(stage));//rnd <round number>
    delay(1000);
    for (int ii=0; ii<steps; ii++) {
      digitalWrite(sequence[ii]+(led1),HIGH); //sequence ranges 0-3, add that to led1 pin for corresponding led
      delay(800-stage*75);//progressively flash faster each round
      digitalWrite(sequence[ii]+(led1),LOW);
      delay(200);
    }
    state = 3;//move to user input
  }
  
  //user input state
  if (state == 3) {
    output(none,g,o,none);//go
      for (int ii=0; ii<steps; ii++) { //cycle through each step in the sequence array
        triggered = 0;
        while (triggered == 0){//wait for a button to be pushed
          triggered = 1;//prevent continuous triggering
          if (analogRead(ss1)>500) {
            push = 1;//first button pushed
            digitalWrite(led1,HIGH);// led high
            break;
          }
          if (analogRead(ss2)>500) {
            push = 2;
            digitalWrite(led2,HIGH);
            break;
          }
          if (analogRead(ss3)>500) {
            push= 3;
            digitalWrite(led3,HIGH);
            break;
          }
          if (analogRead(ss4)>500) {
            push = 4;
            digitalWrite(led4,HIGH);
            break;
          }
        }
        while (analogRead(ss1) >500 || analogRead(ss2)>500 || analogRead(ss3)>500 || analogRead(ss4)>500) {}//freeze while button is depressed
        delay(300);//keep the led on for another 300ms
        clear_lights();//clear leds
        if (sequence[ii] == push)//check if pushed button matches value in sequence
          continue;
        else {
          state = 5;//game over
          break;
        }
      }
      if (state!=5)//everything was correct, move to the next stage
        stage++;
      
      if (stage < maxStage && state!=5) {//more stages to go, not game over
        output(none,B01101111,B00001111,none);// :)
        delay(1000);
        state = 1;
      }
      else if (stage > maxStage) {//winner!
        state = 4;
        digitalWrite(buzzer,HIGH);
        delay(100);
        digitalWrite(buzzer,LOW);
      }
  }
  
  //game win state
  if (state == 4) {
    output(y,o,u);
    delay(1000);
    output(a,r,e);
    delay(1000);
    output(g,o,d);
    delay(1000);
  }
  
  //game over state
  if (state == 5) { // you dun goofed
    //output(n,o,p,e);
    output(none,B01101111,B01100011,none);// :(
    digitalWrite(buzzer,HIGH);
    delay(500);
    digitalWrite(buzzer,LOW);
    delay(2000);
    state = 0;//return to pending state
  }
  
  /**
  END STATE MACHINE
  **/
  
  clear_lights(); //clear the leds to prevent errors
}


