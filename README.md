ArduinoSimon
============
ArduinoSimon is a highly customizable electronic game of Simon where the objective is to pay attention to progressively longer squences of LED's and repeat the pattern by pressing buttons.

## Features
* Integrated 4 seven segment display message board with display drivers to customize your own messages.
* State machine logic makes game easy to modify and add to.
* Easily customize number of rounds and difficulty.

##Construction
The following materials will need to be obtained:
* 4 - Common anode Seven Segment Display
* 4 - 74LS164 serial in, parallel out shift register
* 5 - Pushbutton switches
* 4 - LEDs
* 1 - Buzzer
* 1 - Photoresistor / variable resistor
* 1 - Arduino
* 28 - ~220ohm resistors (for seven segment displays)
* 4 - ~100ohm resistors (for LEDs)
* 5 - ~100kohm resistors (for switch pull-down)

To build the circuit on a breadboard, follow the [schematic](https://github.com/mahsu/ArduinoSimon/blob/master/schematic/arduinosimon.jpg) and sample completed [circuit](https://github.com/mahsu/ArduinoSimon/blob/master/ArduinoSimon.JPG) as reference. Additionally, you may refer to the [Principles of Design: Hardware](#principles-of-design), below. After making sure all pins are connected correctly, you may deploy `ArduinoSimon.ino` to the Arduino or take the steps below to customize the game.

##Customization
* Under the Game Settings heading in the .ino, there are three main settings that may immediately be customized:
    ```
    int maxStage = 10;
    int extra = 2;
    int sequence[200];
    ```
    `maxStage` is the total number of stages that the player must beat before they win. `extra` is the number of extra steps in addition to the stage number that the stage will have. `sequence` is the maximum number of steps that a single stage will have. The exact value of this number is not important, but it should be larger than the maximum number of steps but small enough to not take up too much memory.

* Additionally, you may change the messages. Each message to the seven segment displays is called with the `output(display1,display2,display3,display4)` function. From left to right, the first argument is the 1st display and the last argument is the last display.The most common letters are predefined on the top of the file, and may be used. For example, the word "game" is displayed by `output(g,a,m,e)`. Additionally, the `none` and `all` variables turn all the segments off and on, respectively, for the given display.

##Gameplay
* When a power source is provided, the displays will turn on and immediately begin cycling in a pending state.
* To start a new game, press the button designated as S5 in the [schematic](https://github.com/mahsu/ArduinoSimon/blob/master/schematic/arduinosimon.jpg) or the button in the top right of the sample [circuit](https://github.com/mahsu/ArduinoSimon/blob/master/ArduinoSimon.JPG).
* The display will change to indicate the first round of a new game, and LEDs will begin to blink.
* Pay attention to the LEDs, and after the pattern finishes the display will indicate that it is your turn to mirror the pattern that was just shown.
* If you are incorrect, the buzzer will beep and the game is over.
* If you are successful, you may move onto the next round.

##Principles of Design
###Hardware
The backbone of the circuit is the Arduino, to which the I/O devices are connected to. The following are the digital outputs mapped from the Arduino.
####Digital Outputs
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
    
Of note is pins 0 to 7, which are connected to the [74LS164](http://www.fairchildsemi.com/ds/74/74VHC164.pdf) ICs. The 74LS164 is an 8-bit serial in, parallel out shift register. (A better alternative is the 74HC595, for reasons that I will cover in the [notes](#notes) section below). Shift registers are ideal for driving LED's since they dramatically reduce the amount of required pins.

With the IC's notch held to the left, and starting from the bottom going counterclockwise:
* Pins 1-2: Input data is the result of the logical AND between input pins 1 and 2. One pin should always be connected high for our purposes.
* Pins 3-6,10-13: Output pins in sequential order from Q0 to Q7
* Pin 7: GND
* Pin 8: Clock pulse input (active rising edge)
* Pin 9: Master reset (active low)
* Pin 14: VCC

The input data pin, either high or low, is the bit that should be shifted into the register at the next rising edge clock pulse. Thankfully, this is all handled by the Arduino using the `shiftOut(data, clock, LSBFIRST, byte);` function. The first and second arguments are the data and clock pins, the third can either be `LSBFIRST` or `MSBFIRST` depending on the wiring (the difference is the order in which a byte is shifted in, either with the most significant bit first or the least significant bit first. This will correspond with how the output pins are wired), and the last argument is the byte to be shifted in, for example `B00010001` is the letter "A". The seven segment displays are wired according to the [standard segment order](http://www.codeproject.com/KB/system/steppermotorcontrol/pic10.jpg) for a common anode display, where segment A is wired to Q0. Do not forget the 220ohm resistors!

On the Arduino, digital outputs 8-11 are rather self explanatory, as they are the LEDs for displaying the pattern sequence. As with the displays, do not forget about the 100ohm resistors. Likewise, the buzzer is connected between pin 12 and GND.

####Analog Inputs
    //ANALOG PINS
    #define newgame 0   //button to start a new game
    #define ss1 1       //button 1 (leftmost)
    #define ss2 2       //button 2
    #define ss3 3       //button 3
    #define ss4 4       //button 4 (rightmost)
    #define rndseed 5   //random seed
Pins 0-4 are technically digital inputs, as they only have a high and low state, but due to insufficient digital pins, they are connected as analog devices. Each of these buttons has a 100kohm pull down resistor connecting it to GND to allow the circuit to be pulled to a digital state without a short-circuit, signalling when a button is pressed. Pin 5 is an the input that the game uses for its random seed when the Arduino starts up. Variance in the value of a light dependent resistor is enough to ensure randomness.

###Software
The software component is run like a state machine.
* State 0: Pending state, LEDs cycle while waiting for user to press new button
* State 1: Pattern generation. Pattern is computed ans stored.
* State 2: Pattern display. Pattern is displayed on the LEDs.
* State 3: User is able to input the pattern previously displayed.
* State 4: Game win state
* State 5: Game lose state

The state graph may be represented as follows:
```
            success
         [-----<----]
         |          |    win
0---->1---->2---->3------>4
|                   |
[-----<---5---<-----]
         fail
```

The following utility functions exist to simplify displaying things:
* `output(byte1,byte2,byte3,byte4)` - outputs four bytes to the four displays (left to right)
* `number(integer)` - returns the byte representation of an integer on a seven segment display.
* `cycle(datapin,clockpin,i)` - cycles one segment at a time, on a display designated by the datapin and clockpin. This is best utilized in a loop function, since the cycle is only advanced on each call.
* `clear_lights()` - only ckears the LEDs, not the displays.

##Notes
* You may use a common cathode display, but be sure to apply a bitwise NOT `~` operation to each outputted byte.
* The photoresistor is used as a random seed for an unpredictable distribution of lights. Any other variable resistor may be used.
* The difference with the 74HC595 and what makes it preferable is that it has a latch pin. This allows three output pins rather than 8 to drive the four shift registers.