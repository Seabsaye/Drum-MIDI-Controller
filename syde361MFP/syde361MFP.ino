#include <Bounce2.h>

/*
 * MIDI mappings:
 * 1 - record mode
 * 2 - playback mode
 * 3 - overdub mode
 * 4 - stop playback
 * 5 - clear command
 */

int inputVoltage1 = 0;
int inputVoltage2 = 0;
int inputVoltage3 = 0;

const byte interruptPin1 = A8;
const byte interruptPin2 = A5;
const byte interruptPin3 = A7;

const byte channelPin = A4;
const byte effectPin = A2;
const byte clearEffectPin = A1;

const byte toggleRecordPin = A14;
const byte togglePlaybackPin = A15;
const byte deleteRecordingPin = A16;

const int ledPin = 13;

int currentChannel = 1;
int currentEffect[] = {1, 1, 1, 1};
int repeatTimes = 3;

int velocity = 100;

int inRecordMode[] = {0,0,0};
int haveRecording[] = {0,0,0};
int inPlaybackMode[] = {0,0,0};

Bounce note1 = Bounce(interruptPin1, 10);
Bounce note2 = Bounce(interruptPin2, 10);
Bounce note3 = Bounce(interruptPin3, 10);
Bounce effectBounce = Bounce(effectPin, 10);
Bounce clearEffectBounce = Bounce(clearEffectPin, 10);

Bounce channelCycleBounce = Bounce(channelPin, 10);

Bounce toggleRecordBounce = Bounce(toggleRecordPin, 10);
Bounce togglePlaybackBounce = Bounce(togglePlaybackPin, 10);
Bounce deleteRecordingBounce = Bounce(deleteRecordingPin, 10);

void setup() {
  pinMode(channelPin, INPUT_PULLUP);
  pinMode(effectPin, INPUT_PULLUP);

  pinMode(interruptPin1, INPUT_PULLUP);
  pinMode(interruptPin2, INPUT_PULLUP);
  pinMode(interruptPin3, INPUT_PULLUP);

  pinMode(toggleRecordPin, INPUT_PULLUP);
  pinMode(togglePlaybackPin, INPUT_PULLUP);
  pinMode(deleteRecordingPin, INPUT_PULLUP);

  pinMode(ledPin, OUTPUT);
  
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
  
  updateChannelState();
  
  updateToggleRecordState();
  updateTogglePlaybackState();
  updateDeleteRecordingState();
  
  usbMIDI.read();
}

int inOverdubMode() {
  return (currentChannel >= 1) ? (haveRecording[currentChannel - 1] && inRecordMode[currentChannel - 1]) : 0;
}

void updateChannelState() {
  if (channelCycleBounce.update() && channelCycleBounce.rose()) {
    (currentChannel < 3) ? currentChannel++ : currentChannel = 1;
  }
}

void updateToggleRecordState() {
  if (toggleRecordBounce.update()) {
    if (toggleRecordBounce.rose()) {
      inRecordMode[currentChannel - 1] = !inRecordMode[currentChannel - 1];
      if (inRecordMode[currentChannel - 1]) {
        digitalWrite(ledPin, HIGH);
        usbMIDI.sendNoteOn((haveRecording[currentChannel - 1]) ? 3 : 1, velocity, currentChannel);
      } else {
        digitalWrite(ledPin, LOW);
        haveRecording[currentChannel - 1] = 1;
        inPlaybackMode[currentChannel - 1] = 1;
        usbMIDI.sendNoteOn(2, velocity, currentChannel);
      }
    } else {
      usbMIDI.sendNoteOff(1, velocity, currentChannel);
      usbMIDI.sendNoteOff(2, velocity, currentChannel);
      usbMIDI.sendNoteOff(3, velocity, currentChannel);
    }
  }
}

void updateTogglePlaybackState() {
  if (togglePlaybackBounce.update()) {
    //only do something if there is a recording on the current channel
    if (haveRecording[currentChannel - 1]) {
      if (togglePlaybackBounce.rose()) {
        usbMIDI.sendNoteOn(inPlaybackMode[currentChannel - 1] ? 4 : 2, velocity, currentChannel);
        inPlaybackMode[currentChannel - 1] = !inPlaybackMode[currentChannel - 1];        
      } else {
        usbMIDI.sendNoteOff(4, velocity, currentChannel);
        usbMIDI.sendNoteOff(2, velocity, currentChannel);
      } 
    }
  }
}

void updateDeleteRecordingState() {
  if (deleteRecordingBounce.update()) {
    if (deleteRecordingBounce.rose()) {
      digitalWrite(ledPin, LOW);
      if (inOverdubMode()) {
        //send a MIDI command to get out of the overdub mode
        usbMIDI.sendNoteOn(4, velocity, currentChannel);
      }
      usbMIDI.sendNoteOn(5, velocity, currentChannel);
      haveRecording[currentChannel - 1] = 0;
      inRecordMode[currentChannel - 1] = 0;
      inPlaybackMode[currentChannel - 1] = 0;
    } else {
      usbMIDI.sendNoteOff(4, velocity, currentChannel);
      usbMIDI.sendNoteOff(5 , velocity, currentChannel);
    }
  }
}

