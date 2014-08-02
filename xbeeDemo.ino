//XBee demo for Flint Area Coding Meetup
//Jack Christensen 25Jul2014
//This work by Jack Christensen is licensed under CC BY-SA 4.0,
//http://creativecommons.org/licenses/by-sa/4.0/

#include <Button.h>       //http://github.com/JChristensen/Button
#include <Servo.h>        //http://arduino.cc/en/Reference/Servo
#include <Streaming.h>    //http://arduiniana.org/libraries/streaming/
#include <XBee.h>         //http://code.google.com/p/xbee-arduino/

const uint32_t minXmit = 60;    //ms
const uint8_t modePin = 7;
const uint8_t servoPin = 9;
const uint8_t redLEDPin = A1;
const uint8_t grnLEDPin = A2;
const uint8_t redBtnPin = A3;
const uint8_t grnBtnPin = A4;
const uint8_t potPin = A5;
const bool pullup = true;
const bool invert = true;
const uint32_t debounce = 25;    //ms
const long int baudRate = 115200;

Button redBtn(redBtnPin, pullup, invert, debounce);
Button grnBtn(grnBtnPin, pullup, invert, debounce);
Servo srvo;

bool redState, grnState;
uint16_t servoAngle, lastAngle;
bool transmitter;

void setup(void)
{
    pinMode(modePin, INPUT_PULLUP);
    pinMode(redLEDPin, OUTPUT);
    pinMode(grnLEDPin, OUTPUT);
    Serial.begin(baudRate);
    srvo.attach(servoPin);
    transmitter = digitalRead(modePin);
    if (transmitter) {
        Serial << F("Transmitter") << endl;
        lastAngle = servoAngle = map(analogRead(potPin), 0, 1023, 0, 179);
        delay(3000);           //give the XBee some time to get connected to the network
        sendData();            //initial values, turn off LEDs, match servo position to popt
    }
    else {
        Serial << F("Receiver") << endl;
        srvo.write(8);         //set servo to minimum position
    }
}

void loop(void)
{
    static bool xmitFlag;
    static uint32_t lastXmit;

    readXBee();

    if (transmitter) {
        redBtn.read();
        grnBtn.read();
        servoAngle = map(analogRead(potPin), 0, 1023, 0, 179);
        uint32_t ms = millis();
        
        if ( redBtn.wasPressed() ) {
            digitalWrite(redLEDPin, redState = !redState);
            xmitFlag = true;
        }
        
        if ( grnBtn.wasPressed() ) {
            digitalWrite(grnLEDPin, grnState = !grnState);
            xmitFlag = true;
        }
        
        if ( (servoAngle != lastAngle) && (ms - lastXmit >= minXmit) ) {
            lastAngle = servoAngle;
            lastXmit = ms;
            xmitFlag = true;
        }
        
        if (xmitFlag) {
            xmitFlag = false;
            lastXmit = ms;
            sendData();
        }
    }
}

