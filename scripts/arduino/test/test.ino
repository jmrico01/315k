#define SERIAL_BITRATE 57600

#define PIN_SERIAL_WRITE 5
#define PIN_CHANNEL_BIT0 2
#define PIN_CHANNEL_BIT1 3

#define ANALOG_INPUT_MAX_VALUE 1023

#define NUM_ANALOG_INPUTS  6
#define NUM_DIGITAL_INPUTS 1

#define DATA_PREFIX_BYTES 4
#define DATA_BUFFER_SIZE (DATA_PREFIX_BYTES + 1 + NUM_DIGITAL_INPUTS + (NUM_ANALOG_INPUTS * sizeof(unsigned short)))

byte dataBuffer[DATA_BUFFER_SIZE];

const int ANALOG_INPUT_PINS[NUM_ANALOG_INPUTS] = {
  A0, A1, A2, A3, A4, A5
};

const int DIGITAL_INPUT_PINS[NUM_DIGITAL_INPUTS] = {
  4
};

void setup()
{
  // Initialize buffer with prefix (4 0xff bytes)
  const byte dataPrefix[DATA_PREFIX_BYTES] = { 0xff, 0xff, 0xff, 0xff };
  memcpy(dataBuffer, dataPrefix, DATA_PREFIX_BYTES);

  Serial.begin(SERIAL_BITRATE);

  pinMode(PIN_SERIAL_WRITE, OUTPUT);
  digitalWrite(PIN_SERIAL_WRITE, LOW);
  pinMode(PIN_CHANNEL_BIT0, INPUT);
  pinMode(PIN_CHANNEL_BIT1, INPUT);

  for (int i = 0; i < NUM_DIGITAL_INPUTS; i++) {
    pinMode(DIGITAL_INPUT_PINS[i], INPUT);
  }
}

byte getActiveChannel()
{
  int bit0 = digitalRead(PIN_CHANNEL_BIT0);
  int bit1 = digitalRead(PIN_CHANNEL_BIT1);

  byte activeChannel = 0;
  if (bit0 == HIGH) {
    activeChannel += 1;
  }
  if (bit1 == HIGH) {
    activeChannel += 2;
  }

  return activeChannel;
}

void loop()
{
  dataBuffer[DATA_PREFIX_BYTES] = getActiveChannel();
  int dataBufferSize = DATA_PREFIX_BYTES + 1;

  for (int i = 0; i < NUM_DIGITAL_INPUTS; i++) {
    // TODO build a bit mask out of this, get 8 digital inputs in 1 byte
    dataBuffer[dataBufferSize++] = digitalRead(DIGITAL_INPUT_PINS[i]) == HIGH ? 0 : 1;
  }

  unsigned short analogValues[NUM_ANALOG_INPUTS];
  for (int i = 0; i < NUM_ANALOG_INPUTS; i++) {
    analogValues[i] = (unsigned short)(ANALOG_INPUT_MAX_VALUE - analogRead(ANALOG_INPUT_PINS[i])); // Inverted. Oops!
  }
  memcpy(&dataBuffer[dataBufferSize], analogValues, NUM_ANALOG_INPUTS * sizeof(unsigned short));
  dataBufferSize += NUM_ANALOG_INPUTS * sizeof(unsigned short);

  digitalWrite(PIN_SERIAL_WRITE, HIGH);
  Serial.write(dataBuffer, dataBufferSize);
  digitalWrite(PIN_SERIAL_WRITE, LOW);

  delay(3); // TODO be smarter here
}
