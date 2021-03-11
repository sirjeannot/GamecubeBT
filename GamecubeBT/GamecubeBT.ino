/*
 * Gamecube Controller using PS3 Controller.
 * https://github.com/sirjeannot/GamecubeBT/
 * 
 * Based on Nicohood Nintendo library
 * https://github.com/NicoHood/Nintendo
 * Based on USBHOST library
 * https://github.com/felis/USB_Host_Shield_2.0
*/

/*#define __ARDUINO_X86__ 1*/

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
bool send_update;
bool send_update_old = false;
#define pinLed LED_BUILTIN

//debounce variables
uint8_t oldx = 128;
uint8_t oldy = 128;
uint8_t oldcx = 128;
uint8_t oldcy = 128;

//reset function for failed usb host init
void(* resetFunc) (void) = 0;

void bt_zero() {
  d.report.a = 0;
  d.report.b = 0;
  d.report.x = 0;
  d.report.y = 0;
  d.report.z = 0;
  d.report.start = 0;
  d.report.r = 0;
  d.report.l = 0;
  d.report.dleft = 0;
  d.report.dright = 0;
  d.report.dup = 0;
  d.report.ddown = 0;
  oldx = d.report.xAxis;
  oldy = d.report.yAxis;
  oldcx = d.report.cxAxis;
  oldcy = d.report.cyAxis;
  d.report.xAxis = 128;
  d.report.yAxis = 128;
  d.report.cxAxis = 128;
  d.report.cyAxis = 128;
}

void setup()
{
  pinMode(pinLed, OUTPUT);
  GamecubeController1.read();
  Serial.begin(250000);
  bt_zero();

  while (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    delay(500);
    resetFunc();
  }
  Serial.println(F("\r\nPS3 Bluetooth Library Started"));
}

void loop()
{
  Usb.Task();
  send_update = false;
  if (PS3.PS3Connected || PS3.PS3NavigationConnected) {
    //default x/y thresholds are 137/117. wider deadzone for older controllers.
    if (PS3.getAnalogHat(LeftHatX) > 147 || PS3.getAnalogHat(LeftHatX) < 107 || PS3.getAnalogHat(LeftHatY) > 147 || PS3.getAnalogHat(LeftHatY) < 107 || PS3.getAnalogHat(RightHatX) > 137 || PS3.getAnalogHat(RightHatX) < 117 || PS3.getAnalogHat(RightHatY) > 137 || PS3.getAnalogHat(RightHatY) < 117) {
      //debounce. sometimes both sticks jump to 0,0
      if (( PS3.getAnalogHat(LeftHatX) + PS3.getAnalogHat(LeftHatY) + PS3.getAnalogHat(RightHatX) + PS3.getAnalogHat(RightHatY) == 0 ) && ( oldx+oldy+oldcx+oldcy > 10 ) ) {
        Serial.println("Bounce!");
      }
      else {
        d.report.xAxis = PS3.getAnalogHat(LeftHatX);
        d.report.yAxis = 128 - (PS3.getAnalogHat(LeftHatY) - 128); //yaxis values are inverted
        d.report.cxAxis = PS3.getAnalogHat(RightHatX);
        d.report.cyAxis = 128 - (PS3.getAnalogHat(RightHatY) - 128); //yaxis values are inverted
        send_update = true;
      }
    }

    if (PS3.getAnalogButton(L2) || PS3.getAnalogButton(R2)) {
      d.report.l = PS3.getAnalogButton(L2);
      d.report.r = PS3.getAnalogButton(R2);
      send_update = true;
    }

    if (PS3.getButtonClick(PS))
      PS3.disconnect();
    else {
      if (PS3.getButtonPress(TRIANGLE)) {
        d.report.y = 1;
        Serial.println(F("TRIANGLE"));
        send_update = true;
      }
      if (PS3.getButtonPress(CIRCLE)) {
        d.report.x = 1;
        Serial.println(F("CIRCLE"));
        send_update = true;
      }
      if (PS3.getButtonPress(CROSS)) {
        d.report.a = 1;
        Serial.println(F("CROSS"));
        send_update = true;
      }
      if (PS3.getButtonPress(SQUARE)) {
        d.report.b = 1;
        Serial.println(F("SQUARE"));
        send_update = true;
      }
      if (PS3.getButtonPress(UP)) {
        d.report.dup = 1;
        Serial.println(F("UP"));
        send_update = true;
      }
      if (PS3.getButtonPress(RIGHT)) {
        d.report.dright = 1;
        Serial.println(F("RIGHT"));
        send_update = true;
      }
      if (PS3.getButtonPress(DOWN)) {
        d.report.ddown = 1;
        Serial.println(F("DOWN"));
        send_update = true;
      }
      if (PS3.getButtonPress(LEFT)) {
        d.report.dleft = 1;
        Serial.println(F("LEFT"));
        send_update = true;
      }
      if (PS3.getButtonPress(R1)) {
        d.report.z = 1;
        Serial.println(F("R1"));
        send_update = true;
      }
      if (PS3.getButtonClick(START)) {
        d.report.start = 1;
        Serial.println(F("START"));
        send_update = true;
      }
    }
  }
  if (send_update == true) {
    Serial.println(F("Sending input to GC."));
    if (!GamecubeConsole1.write(d))
      Serial.println(F("Error writing Gamecube controller."));
    send_update_old = send_update;
    bt_zero();
  }
}
