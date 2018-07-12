int inputVoltage1 = 0;
int inputVoltage2 = 0;
int inputVoltage3 = 0;

const byte interruptPin1 = A8;
const byte interruptPin2 = A5;
const byte interruptPin3 = A7;

const byte channelPin = A4;
const byte repeatNotePin = A2;

int currentChannel = 1;
int currentEffect = 0;
int repeatTimes = 5;

int velocity = 100;

void setup() {
  pinMode(channelPin, INPUT_PULLUP);
  pinMode(repeatNotePin, INPUT_PULLUP);
  
  Serial.begin(9600);
  
  attachInterrupt(digitalPinToInterrupt(channelPin), changeChannel, RISING);
  attachInterrupt(digitalPinToInterrupt(repeatNotePin), changeEffect, RISING);
}

void sendNote(int midiNote) {
  usbMIDI.sendNoteOn(midiNote, velocity, currentChannel);
  delay(150);
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
  inputVoltage1 = analogRead(interruptPin1) * 0.0049;
  inputVoltage2 = analogRead(interruptPin2) * 0.0049;
  inputVoltage3 = analogRead(interruptPin3) * 0.0049;

  if (inputVoltage1 > 3) {
    if (currentEffect <= 1) {
       sendNote(40);
    } else if (currentEffect == 2) {
       sendRepeatedNote(40);
    } 
    inputVoltage1 = 0;
  }

  if (inputVoltage2 > 3) {
    if (currentEffect <= 1) {
       sendNote(45);
    } else if (currentEffect == 2) {
       sendRepeatedNote(45);
    } 
    inputVoltage2 = 0;
  }

  if (inputVoltage3 > 3) {
    if (currentEffect <= 1) {
       sendNote(50);
    } else if (currentEffect == 2) {
       sendRepeatedNote(50);
    } 
    inputVoltage3 = 0;
  }
}

void changeChannel() {
  int channelPinVoltage = analogRead(channelPin) * 0.0049;
  
  if (channelPinVoltage > 3) {
    currentChannel = (currentChannel == 1) ? 2 : 1;
  }
}

void changeEffect() {
  int repeatPinVoltage = analogRead(repeatNotePin) * 0.0049;
   
  if (repeatPinVoltage > 3) {
    if (currentEffect == 0) {
      currentEffect = 1;
      usbMIDI.sendControlChange(2, 60, currentChannel);
    } else if (currentEffect == 1) {
      currentEffect = 2;
      usbMIDI.sendControlChange(2, 25, currentChannel);
    } else if (currentEffect == 2) {
      currentEffect = 0;
    } 
  }
}

