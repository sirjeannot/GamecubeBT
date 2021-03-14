/*
   Gamecube Controller using PS3 Controller.
   https://github.com/sirjeannot/GamecubeBT/

   Based on Nicohood Nintendo library
   https://github.com/NicoHood/Nintendo
   Based on USBHOST library
   https://github.com/felis/USB_Host_Shield_2.0
*/

#define TIMER1_MAX 70

//Nintendo gamecube bus
#include "Nintendo.h"
//PS3 controller bt
#include <PS3BT.h>
#include <usbhub.h>
#include <SPI.h>

//Define a Gamecube Controller and a Console
CGamecubeConsole GamecubeConsole1(8);
CGamecubeController GamecubeController1(7);
Gamecube_Data_t d = defaultGamecubeData;

USB Usb;
USBHub Hub1(&Usb); // Some dongles have a hub inside
BTD Btd(&Usb); // You have to create the Bluetooth Dongle instance like so
//PS3BT PS3(&Btd); // This will just create the instance
PS3BT PS3(&Btd, 0x00, 0x15, 0x83, 0x54, 0x00, 0x72); // This will also store the bluetooth address - this can be obtained from the dongle when running the sketch
//#define pinLed LED_BUILTIN

//reset function for failed usb host init
void(* resetFunc) (void) = 0;

ISR(TIMER1_COMPA_vect) {
  GamecubeConsole1.write(d);
}

void setup()
{
  //pinMode(pinLed, OUTPUT);
  GamecubeController1.read();
  //Serial.begin(250000);

  while (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    delay(500);
    resetFunc();
  }
  Serial.println(F("\r\nPS3 Bluetooth Library Started"));

  cli();
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1 = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = TIMER1_MAX;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  //testing. lowest working without killing IO for usb on timer1 : 40-1024, highest working
  //testing. lowest working without sending twice state : 70-1024. 40 barely works
  //testing. highest working without disconnect :
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();//allow interrupts
}

void loop()
{
  Usb.Task();
  if (PS3.PS3Connected || PS3.PS3NavigationConnected) {
    //out of deadzone
    if (PS3.getAnalogHat(LeftHatX) > 147 || PS3.getAnalogHat(LeftHatX) < 107 || PS3.getAnalogHat(LeftHatY) > 147 || PS3.getAnalogHat(LeftHatY) < 107 || PS3.getAnalogHat(RightHatX) > 137 || PS3.getAnalogHat(RightHatX) < 117 || PS3.getAnalogHat(RightHatY) > 137 || PS3.getAnalogHat(RightHatY) < 117) {
      d.report.xAxis = PS3.getAnalogHat(LeftHatX);
      d.report.yAxis = (255 - PS3.getAnalogHat(LeftHatY)); //yaxis values are inverted
      d.report.cxAxis = PS3.getAnalogHat(RightHatX);
      d.report.cyAxis = (255 - PS3.getAnalogHat(RightHatY)); //yaxis values are inverted
    }
    //in deadzone
    else {
      d.report.xAxis = 128;
      d.report.yAxis = 128;
      d.report.cxAxis = 128;
      d.report.cyAxis = 128;
    }
    d.report.left = PS3.getAnalogButton(L2);
    d.report.left > 200 ? d.report.l = 1 : d.report.l = 0 ;
    d.report.right = PS3.getAnalogButton(R2);
    d.report.right > 200 ?  d.report.r = 1 :  d.report.r = 0 ;
    d.report.x = PS3.getButtonPress(CIRCLE);
    d.report.y = PS3.getButtonPress(TRIANGLE);
    d.report.a = PS3.getButtonPress(CROSS);
    d.report.b = PS3.getButtonPress(SQUARE);
    d.report.dup = PS3.getButtonPress(UP);
    d.report.dright = PS3.getButtonPress(RIGHT);
    d.report.ddown = PS3.getButtonPress(DOWN);
    d.report.dleft = PS3.getButtonPress(LEFT);
    d.report.z = PS3.getButtonPress(R1);
    d.report.start = PS3.getButtonPress(START);
    if (PS3.getButtonClick(PS))
      PS3.disconnect();
  }
  //GamecubeConsole1.write(d); //disabled, run by timer1
}
