#define NUM_ANALOG_INPUTS 6

// Send (input number + float value) for every analog input
#define DATA_PACKET_SIZE (1 + 4)
// Max data payload: packet for every input, plus prefix packet and active channel
#define DATA_PAYLOAD_MAX_SIZE (DATA_PACKET_SIZE * (NUM_ANALOG_INPUTS + 1) + 1)

#define ANALOG_INPUT_MAX_VALUE 1023

#define VALUE_DIFF_THRESHOLD 3

const int PIN_CHANNEL_BIT0 = 2;
const int PIN_CHANNEL_BIT1 = 3;

const int ANALOG_INPUT_PINS[NUM_ANALOG_INPUTS] = {
  A0, A1, A2, A3, A4, A5
};

int lastValues[NUM_ANALOG_INPUTS];

void setup()
{
  Serial.begin(9600);

  pinMode(PIN_CHANNEL_BIT0, INPUT);
  pinMode(PIN_CHANNEL_BIT1, INPUT);

  for (int i = 0; i < NUM_ANALOG_INPUTS; i++) {
    lastValues[i] = -1;
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
  byte buffer[DATA_PAYLOAD_MAX_SIZE];
  int bufferSize = 0;

  if (Serial.available() > 0) {
    byte in = Serial.read();
    if (in == 0xff) {
      // ping, reset lastValues to send all
      for (byte i = 0; i < NUM_ANALOG_INPUTS; i++) {
        lastValues[i] = -1;
      }
    }
  }

  for (byte i = 0; i < NUM_ANALOG_INPUTS; i++) {
    int value = ANALOG_INPUT_MAX_VALUE - analogRead(ANALOG_INPUT_PINS[i]); // Inverted, oops!
    if (abs(value - lastValues[i]) >= VALUE_DIFF_THRESHOLD || lastValues[i] == -1) {
      lastValues[i] = value;
      float valueNormalized = constrain((float)value / (float)ANALOG_INPUT_MAX_VALUE, 0.0f, 1.0f);

      if (bufferSize == 0) {
        memset(buffer, 0xff, DATA_PACKET_SIZE);
        bufferSize += DATA_PACKET_SIZE;
        buffer[bufferSize++] = getActiveChannel();
      }
      buffer[bufferSize] = i;
      memcpy(&buffer[bufferSize + 1], &valueNormalized, 4);
      bufferSize += DATA_PACKET_SIZE;
    }
  }

  if (bufferSize > 0) {
    Serial.write(buffer, bufferSize);
  }
}
