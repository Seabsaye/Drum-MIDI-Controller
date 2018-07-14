#include <Bounce2.h>

int inputVoltage1 = 0;
int inputVoltage2 = 0;
int inputVoltage3 = 0;

const byte interruptPin1 = A8;
const byte interruptPin2 = A5;
const byte interruptPin3 = A7;

const byte channelPin = A4;
const byte effectPin = A2;
const byte clearEffectPin = A1;

int currentChannel = 1;
int currentEffect[] = {1, 1, 1, 1};
int repeatTimes = 3;

int velocity = 100;

Bounce note1 = Bounce(interruptPin1, 10);
Bounce note2 = Bounce(interruptPin2, 10);
Bounce note3 = Bounce(interruptPin3, 10);
Bounce effectBounce = Bounce(effectPin, 10);
Bounce clearEffectBounce = Bounce(clearEffectPin, 10);

Bounce channelCycleBounce = Bounce(channelPin, 10);

void setup() {
  pinMode(channelPin, INPUT_PULLUP);
  pinMode(effectPin, INPUT_PULLUP);

  pinMode(interruptPin1, INPUT_PULLUP);
  pinMode(interruptPin2, INPUT_PULLUP);
  pinMode(interruptPin3, INPUT_PULLUP);
  
  Serial.begin(9600);
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
      if (currentEffect[currentChannel] <= 4) {
        usbMIDI.sendNoteOn(38, velocity, currentChannel);
      } else {
        sendRepeatedNote(38);
      }
    } else {
      usbMIDI.sendNoteOff(38, velocity, currentChannel);
    }
  }

  if (note2.update()) {
    if (note2.rose()) {
      if (currentEffect[currentChannel] <= 4) {
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
      if (currentEffect[currentChannel] <= 4) {
        usbMIDI.sendNoteOn(50, velocity, currentChannel);
      } else {
         sendRepeatedNote(50);
      }
    } else {
      usbMIDI.sendNoteOff(50, velocity, currentChannel);
    }
  }

  if (effectBounce.update() && effectBounce.rose()) {
    if (currentEffect[currentChannel] == 0) {
      currentEffect[currentChannel] = 1;
      usbMIDI.sendControlChange(2, 100, currentChannel);
    } else if (currentEffect[currentChannel] == 1) {
      currentEffect[currentChannel] = 2;
      usbMIDI.sendControlChange(2, 25, currentChannel);
      usbMIDI.sendControlChange(3, 100, currentChannel);
    } else if (currentEffect[currentChannel] == 2) {
      currentEffect[currentChannel] = 3;
      usbMIDI.sendControlChange(3, 25, currentChannel);
      usbMIDI.sendControlChange(4, 100, currentChannel);
    } else if (currentEffect[currentChannel] == 3) {
      currentEffect[currentChannel] = 4;
      usbMIDI.sendControlChange(4, 25, currentChannel);
      usbMIDI.sendControlChange(5, 100, currentChannel);
    } else if (currentEffect[currentChannel] == 4){
      currentEffect[currentChannel] = 5;
      usbMIDI.sendControlChange(5, 25, currentChannel);
    } else {
      currentEffect[currentChannel] = 0;
    }
  }

  if (clearEffectBounce.update() && clearEffectBounce.rose()) {
    currentEffect[currentChannel] = 1;
  }

  if (channelCycleBounce.update() && channelCycleBounce.rose()) {
    (currentChannel < 3) ? currentChannel++ : currentChannel = 1;
  }
}

