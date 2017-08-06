

const byte rfInputPin = 2;
volatile byte state = LOW;

static void activityLed (byte on) {
  digitalWrite(LED_BUILTIN, !on);
}

void setup() {
  pinMode(rfInputPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(rfInputPin), flank, CHANGE);
}

void loop() {
  activityLed(state);
}

void flank() {
  state = !state;
}
