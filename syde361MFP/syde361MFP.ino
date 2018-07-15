#include <Bounce2.h>

/*
 * MIDI mappings:
 * 1 - record mode
 * 2 - playback mode
 * 3 - overdub mode
 * 4 - stop playback
 * 5 - clear command
 */

const int RECORD_MIDI_NOTE = 1;
const int START_PLAYBACK_MIDI_NOTE = 2;
const int OVERDUB_MIDI_NOTE = 3;
const int STOP_PLAYBACK_MIDI_NOTE = 4;
const int CLEAR_RECORD_MIDI_NOTE = 5;

double bottomLeft;
double bottomRight;
double topLeft;
double topRight;
double middleLeft;
double middleRight;
int hitLocation; // 0 for top, 1 for bottom
int hitHardness;  // 0 for soft note, 1 for hard note

const byte topLeftPin = A0;
const byte topRightPin = A1;
const byte middleRightPin = A2;
const byte middleLeftPin = A3;
const byte bottomLeftPin = A4;
const byte bottomRightPin = A6;

const byte toggleRecordPin = A14;
const byte togglePlaybackPin = A15;
const byte deleteRecordingPin = A16;

const byte channelPin = A17;
const byte effectPin = A18;
const byte clearEffectPin = A19;

const int ledPin = 13;

int currentChannel = 1;
int currentEffect[] = {1, 1, 1, 1};
int repeatTimes = 3;

int velocity;

int inRecordMode[] = {0,0,0};
int haveRecording[] = {0,0,0};
int inPlaybackMode[] = {0,0,0};

Bounce effectBounce = Bounce(effectPin, 10);
Bounce clearEffectBounce = Bounce(clearEffectPin, 10);
Bounce channelCycleBounce = Bounce(channelPin, 10);

Bounce toggleRecordBounce = Bounce(toggleRecordPin, 10);
Bounce togglePlaybackBounce = Bounce(togglePlaybackPin, 10);
Bounce deleteRecordingBounce = Bounce(deleteRecordingPin, 10);

void setup() {
  pinMode(channelPin, INPUT_PULLUP);
  pinMode(effectPin, INPUT_PULLUP);
  pinMode(clearEffectPin, INPUT_PULLUP);

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

void sendChangeEffect(int effect, int offValue, int onValue) {
  currentEffect[currentChannel] = effect;
  usbMIDI.sendControlChange(effect, offValue, currentChannel);
  usbMIDI.sendControlChange(effect + 1, onValue, currentChannel);
}

void loop() {  
  topLeft = analogRead(topLeftPin) * 0.0049;
  topRight = analogRead(topRightPin) * 0.0049;
  middleRight = analogRead(middleRightPin) * 0.0049;
  middleLeft = analogRead(middleLeftPin) * 0.0049;
  bottomLeft = analogRead(bottomLeftPin) * 0.0049;
  bottomRight = analogRead(bottomRightPin) * 0.0049;
     
  if (bottomLeft>0.3|| bottomRight>0.3 || topLeft>0.5|| topRight>0.5 || middleLeft>0.5|| middleRight>0.5) {
    hitLocation = location();
    hitHardness = strike(hitLocation);
    velocity = hitHardness == 0 ? 50 : 100;
    
    if (hitLocation == 0) {
      if (currentEffect[currentChannel] <= 4) {
        usbMIDI.sendNoteOn(38, velocity, currentChannel);
        delay(150);
        usbMIDI.sendNoteOff(38, velocity, currentChannel);
      } else {
        sendRepeatedNote(38);
      }
    } else {
      if (currentEffect[currentChannel] <= 4) {
        usbMIDI.sendNoteOn(45, velocity, currentChannel);
        delay(150);
        usbMIDI.sendNoteOff(45, velocity, currentChannel);
      } else {
        sendRepeatedNote(45);
      }
    }
  }

  // If toggle effect button pressed, send control changes to change the effects in Ableton
  if (effectBounce.update() && effectBounce.rose()) {
    if (currentChannel == 3) {
      if (currentEffect[currentChannel] == 0) {
          sendChangeEffect(1, 25, 100); 
      } else if (currentEffect[currentChannel] == 1) {
         sendChangeEffect(2, 25, 100);
      } else if (currentEffect[currentChannel] == 2) {
         sendChangeEffect(3, 25, 100);
      } else if (currentEffect[currentChannel] == 3) {
         sendChangeEffect(4, 25, 100);
      } else if (currentEffect[currentChannel] == 4){
         currentEffect[currentChannel] = 5;
         usbMIDI.sendControlChange(5, 25, currentChannel);
      } else {
        currentEffect[currentChannel] = 0;
      }
    } else if (currentChannel == 2) {
      if (currentEffect[currentChannel] == 0) {
        currentEffect[currentChannel] = 1;
        usbMIDI.sendControlChange(4, 50, currentChannel);
      } else {
        currentEffect[currentChannel] = 0;
        usbMIDI.sendControlChange(4, 0, currentChannel);
      }
    } 
  }

  if (clearEffectBounce.update() && clearEffectBounce.rose()) {
    if (currentChannel == 2) {
      usbMIDI.sendControlChange(4, 0, currentChannel);
    } else {
      usbMIDI.sendControlChange(currentEffect[currentChannel] + 1, 25, currentChannel);
      currentEffect[currentChannel] = 0;
    }
  }
  
  updateChannelState();
  
  updateToggleRecordState();
  updateTogglePlaybackState();
  updateDeleteRecordingState();
  
  usbMIDI.read();
}

// Returns an int indicating whether Ableton Live is in overdub mode or not
int inOverdubMode() {
  return (currentChannel >= 1) ? (haveRecording[currentChannel - 1] && inRecordMode[currentChannel - 1]) : 0;
}

// Updates the current channel used in Ableton Live if the associated button is pressed
// Channels range from 1-3
void updateChannelState() {
  if (channelCycleBounce.update() && channelCycleBounce.rose()) {
    (currentChannel < 3) ? currentChannel++ : currentChannel = 1;
  }
}

// Toggles recording in either regular (MIDI note 1) or overdub (MIDI note 3) modes
// for a given channel and turns on an LED for the recording state
void updateToggleRecordState() {
  if (toggleRecordBounce.update()) {
    if (toggleRecordBounce.rose()) {
      inRecordMode[currentChannel - 1] = !inRecordMode[currentChannel - 1];
      if (inRecordMode[currentChannel - 1]) {
        digitalWrite(ledPin, HIGH);
        usbMIDI.sendNoteOn((haveRecording[currentChannel - 1]) ? OVERDUB_MIDI_NOTE : RECORD_MIDI_NOTE, velocity, currentChannel);
      } else {
        digitalWrite(ledPin, LOW);
        haveRecording[currentChannel - 1] = 1;
        inPlaybackMode[currentChannel - 1] = 1;
        usbMIDI.sendNoteOn(START_PLAYBACK_MIDI_NOTE, velocity, currentChannel);
      }
    } else {
      usbMIDI.sendNoteOff(RECORD_MIDI_NOTE, velocity, currentChannel);
      usbMIDI.sendNoteOff(START_PLAYBACK_MIDI_NOTE, velocity, currentChannel);
      usbMIDI.sendNoteOff(OVERDUB_MIDI_NOTE, velocity, currentChannel);
    }
  }
}

// Toggles playback of a recording for a given channel
void updateTogglePlaybackState() {
  if (togglePlaybackBounce.update()) {
    // Only do something if there is a recording on the current channel
    if (haveRecording[currentChannel - 1]) {
      if (togglePlaybackBounce.rose()) {
        usbMIDI.sendNoteOn(inPlaybackMode[currentChannel - 1] ? STOP_PLAYBACK_MIDI_NOTE : START_PLAYBACK_MIDI_NOTE, velocity, currentChannel);
        inPlaybackMode[currentChannel - 1] = !inPlaybackMode[currentChannel - 1];        
      } else {
        usbMIDI.sendNoteOff(STOP_PLAYBACK_MIDI_NOTE, velocity, currentChannel);
        usbMIDI.sendNoteOff(START_PLAYBACK_MIDI_NOTE, velocity, currentChannel);
      } 
    }
  }
}


// Deletes a recording for a given channel and clears all of the channel's information
void updateDeleteRecordingState() {
  if (deleteRecordingBounce.update()) {
    if (deleteRecordingBounce.rose()) {
      digitalWrite(ledPin, LOW);
      if (inOverdubMode()) {
        // Send a MIDI command to exit the overdub mode
        usbMIDI.sendNoteOn(STOP_PLAYBACK_MIDI_NOTE, velocity, currentChannel);
      }
      usbMIDI.sendNoteOn(CLEAR_RECORD_MIDI_NOTE, velocity, currentChannel);
      haveRecording[currentChannel - 1] = 0;
      inRecordMode[currentChannel - 1] = 0;
      inPlaybackMode[currentChannel - 1] = 0;
    } else {
      usbMIDI.sendNoteOff(STOP_PLAYBACK_MIDI_NOTE, velocity, currentChannel);
      usbMIDI.sendNoteOff(CLEAR_RECORD_MIDI_NOTE , velocity, currentChannel);
    }
  }
}

int location() {
  if (topLeft + topRight > bottomLeft + bottomRight + middleLeft + middleRight && (bottomLeft + middleLeft < 0.33 || topLeft > 0.7)) {
    return 0; // hit was on top
  } else {
    return 1; // hit was on bottom
  }
}

int strike(int location)
{
  if (location == 0) {
    if ((topRight + topLeft > 0.72 && abs(topLeft-topRight)>0.35)||topLeft>0.56||topRight>0.56||(topLeft+topRight>1.1) || (abs(topLeft-topRight)>0.47 && (topRight>0.53||topLeft>0.53))){
      return 1; // hard hit registered
    }
    else {
      return 0; // soft hit registered
    }
  } else {
    if (topRight+topLeft+middleRight+middleLeft+bottomLeft+bottomRight>0.9){
      return 1; // hard hit registered
    } else {
      return 0; // soft hit registered
    }
  }
}

