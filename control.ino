#include <Arduino.h>
#include "Adafruit_TinyUSB.h"
#include <RotaryEncoder.h>
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_CONSUMER()
};
Adafruit_USBD_HID usb_hid;
const int pinCLK = 3; 
const int pinDT = 2; 
const int buttonPin = 4; 
RotaryEncoder encoder(pinCLK, pinDT, RotaryEncoder::LatchMode::FOUR3);
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
void checkPosition() {
  encoder.tick();
}
void setup() {
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.begin();
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinCLK), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinDT), checkPosition, CHANGE);
  while( !TinyUSBDevice.mounted() ) {
    delay(1);
  }
}
void loop() {
  RotaryEncoder::Direction dir = encoder.getDirection();
  if (dir == RotaryEncoder::Direction::CLOCKWISE) {
    sendMediaKey(HID_USAGE_CONSUMER_VOLUME_INCREMENT);
  } 
  else if (dir == RotaryEncoder::Direction::COUNTERCLOCKWISE) {
    sendMediaKey(HID_USAGE_CONSUMER_VOLUME_DECREMENT); 
  }
  bool reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW) {
      sendMediaKey(HID_USAGE_CONSUMER_MUTE);
      while(digitalRead(buttonPin) == LOW) {
         delay(10); 
      }
    }
  }
  lastButtonState = reading;
}
void sendMediaKey(uint16_t key) {
  if (!usb_hid.ready()) return;
  usb_hid.sendReport(0, &key, sizeof(key));
  delay(5); 
  uint16_t release = 0;
  usb_hid.sendReport(0, &release, sizeof(release));
}