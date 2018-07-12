#include <Bounce2.h>

int inputVoltage1 = 0;
int inputVoltage2 = 0;
int inputVoltage3 = 0;

const byte interruptPin1 = A8;
const byte interruptPin2 = A5;
const byte interruptPin3 = A7;

const byte channelPin = A4;
const byte repeatNotePin = A2;

int currentChannel = 1;
bool repeat = false;
int repeatTimes = 5;

int velocity = 100;

Bounce note1 = Bounce(interruptPin1, 10);
Bounce note2 = Bounce(interruptPin2, 10);
Bounce note3 = Bounce(interruptPin3, 10);

void setup() {
  pinMode(channelPin, INPUT_PULLUP);
  pinMode(repeatNotePin, INPUT_PULLUP);

  pinMode(interruptPin1, INPUT_PULLUP);
  pinMode(interruptPin2, INPUT_PULLUP);
  pinMode(interruptPin3, INPUT_PULLUP);
  
  Serial.begin(9600);
  
  attachInterrupt(digitalPinToInterrupt(channelPin), changeChannel, RISING);
  attachInterrupt(digitalPinToInterrupt(repeatNotePin), toggleRepeat, RISING);
}

void sendNote(int midiNote) {
  usbMIDI.sendNoteOn(midiNote, velocity, currentChannel);
  delay(100);
  usbMIDI.sendNoteOff(midiNote, velocity, currentChannel);
}

void sendRepeatedNote(int midiNote) {
  int i = 0;
  while (i < repeatTimes) {
    usbMIDI.sendNoteOn(midiNote, velocity, currentChannel);
    delay(100);
    usbMIDI.sendNoteOff(midiNote, velocity, currentChannel);
    i++;
  }
}

void loop() {  
  if (note1.update()) {
    if (note1.rose()) {
      if (!repeat) {
        usbMIDI.sendNoteOn(40, velocity, currentChannel);
      } else {
        sendRepeatedNote(40);
      }
    } else {
      usbMIDI.sendNoteOff(40, velocity, currentChannel);
    }
  }

  if (note2.update()) {
    if (note2.rose()) {
      if (!repeat) {
        usbMIDI.sendNoteOn(45, velocity, currentChannel);
      } else {
         sendRepeatedNote(45);
      }
    } else {
      usbMIDI.sendNoteOff(45, velocity, currentChannel);
    }
  }

  if (note3.update()) {
    if (note3.rose()) {
      if (!repeat) {
        usbMIDI.sendNoteOn(50, velocity, currentChannel);
      } else {
         sendRepeatedNote(50);
      }
    } else {
      usbMIDI.sendNoteOff(50, velocity, currentChannel);
    }
  }
}

void changeChannel() {
  int channelPinVoltage = analogRead(channelPin) * 0.0049;
  
  if (channelPinVoltage > 3) {
    currentChannel = (currentChannel == 1) ? 2 : 1;
  }
}

void toggleRepeat() {
  int repeatPinVoltage = analogRead(repeatNotePin) * 0.0049;

  if (repeatPinVoltage > 3) {
    repeat = !repeat;
  }
}

