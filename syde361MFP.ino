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

void setup() {
  pinMode(interruptPin1, INPUT_PULLUP);
  pinMode(interruptPin2, INPUT_PULLUP);
  pinMode(interruptPin3, INPUT_PULLUP);
  pinMode(channelPin, INPUT_PULLUP);
  pinMode(repeatNotePin, INPUT_PULLUP);
  
  Serial.begin(9600);
  
  attachInterrupt(digitalPinToInterrupt(interruptPin1), outputMIDI1, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptPin2), outputMIDI2, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptPin3), outputMIDI3, RISING);
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
  if (inputVoltage1 > 3) {
    if (!repeat) {
       sendNote(40);
    } else {
       sendRepeatedNote(40);
    }
    inputVoltage1 = 0;
  }

  if (inputVoltage2 > 3) {
    if (!repeat) {
       sendNote(45);
    } else {
       sendRepeatedNote(45);
    }
    inputVoltage2 = 0;
  }

  if (inputVoltage3 > 3) {
    if (!repeat) {
       sendNote(50);
    } else {
       sendRepeatedNote(50);
    }
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

void toggleRepeat() {
  int repeatPinVoltage = analogRead(repeatNotePin) * 0.0049;

  if (repeatPinVoltage > 3) {
    repeat = !repeat;
  }
}

