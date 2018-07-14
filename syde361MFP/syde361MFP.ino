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

// Function to create the repeated note effect
void sendRepeatedNote(int midiNote) {
  int i = 0;
  while (i < repeatTimes) {
    usbMIDI.sendNoteOn(midiNote, velocity, currentChannel);
    delay(100);
    usbMIDI.sendNoteOff(midiNote, velocity, currentChannel);
    i++;
  }
}

void switchEffect(int effect, int offValue, int onValue) {
   currentEffect[currentChannel] = effect;
   usbMIDI.sendControlChange(effect, offValue, currentChannel);
   usbMIDI.sendControlChange(effect + 1, onValue, currentChannel);
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

  // If toggle effect button pressed, send control changes to change the effects in Ableton
  if (effectBounce.update() && effectBounce.rose()) {
    if (currentEffect[currentChannel] == 0) {
      switchEffect(1, 25, 100);
    } else if (currentEffect[currentChannel] == 1) {
      switchEffect(2, 25, 100);
    } else if (currentEffect[currentChannel] == 2) {
      switchEffect(3, 25, 100);
    } else if (currentEffect[currentChannel] == 3) {
      switchEffect(4, 25, 100);
    } else if (currentEffect[currentChannel] == 4){
      currentEffect[currentChannel] = 5;
      usbMIDI.sendControlChange(5, 25, currentChannel);
    } else {
      currentEffect[currentChannel] = 0;
    }
  }

  // If clear effect button pressed, clear effect on current channel
  if (clearEffectBounce.update() && clearEffectBounce.rose()) {
    usbMIDI.sendControlChange(*currentEffect + 1, 25, currentChannel);
    currentEffect[currentChannel] = 0;
  }

  if (channelCycleBounce.update() && channelCycleBounce.rose()) {
    (currentChannel < 3) ? currentChannel++ : currentChannel = 1;
  }
}

