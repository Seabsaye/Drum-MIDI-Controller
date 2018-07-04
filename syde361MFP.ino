int inputVoltage1 = 0;
int inputVoltage2 = 0;
int inputVoltage3 = 0;

const byte interruptPin1 = A8;
const byte interruptPin2 = A5;
const byte interruptPin3 = A7;

const byte channelPin = A4;

int currentChannel = 1;

int velocity = 100;

void setup() {
  pinMode(interruptPin1, INPUT_PULLUP);
  pinMode(interruptPin2, INPUT_PULLUP);
  pinMode(interruptPin3, INPUT_PULLUP);
  pinMode(channelPin, INPUT_PULLUP);
  
  Serial.begin(9600);
  
  attachInterrupt(digitalPinToInterrupt(interruptPin1), outputMIDI1, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptPin2), outputMIDI2, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptPin3), outputMIDI3, RISING);
  attachInterrupt(digitalPinToInterrupt(channelPin), changeChannel, RISING);
}

void loop() {
  if (inputVoltage1 > 3) {
    usbMIDI.sendNoteOn(40, velocity, currentChannel);
    delay(100);
    usbMIDI.sendNoteOff(40, velocity, currentChannel);
    inputVoltage1 = 0;
  }

    if (inputVoltage2 > 3) {
    usbMIDI.sendNoteOn(45, velocity, currentChannel);
    delay(100);
    usbMIDI.sendNoteOff(45, velocity, currentChannel);
    inputVoltage2 = 0;
  }

    if (inputVoltage3 > 3) {
    usbMIDI.sendNoteOn(50, velocity, currentChannel);
    delay(100);
    usbMIDI.sendNoteOff(50, velocity, currentChannel);
    inputVoltage3 = 0;
  }
}

void outputMIDI1() {
  inputVoltage1 = analogRead(interruptPin1) * 0.0049;
}

void outputMIDI2() {
  inputVoltage2 = analogRead(interruptPin2) * 0.0049;
}

void outputMIDI3() {
  inputVoltage3 = analogRead(interruptPin3) * 0.0049;
}

void changeChannel() {
  int channelPinVoltage = analogRead(channelPin) * 0.0049;
  
  if (channelPinVoltage > 3) {
    currentChannel = (currentChannel == 1) ? 2 : 1;
  }
}

