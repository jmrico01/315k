#pragma once

#define ARDUINO_BUFFER_SIZE KILOBYTES(1)

struct Win32ArduinoPacket
{
    uint8 inputNumber;
    float32 value;
};

struct Win32Arduino
{
    static const char* ARDUINO_PORT_NAME;

    HANDLE handle;
    COMSTAT status;
    DWORD errors;

    uint64 bufferRead, bufferWrite;
    uint8 buffer[ARDUINO_BUFFER_SIZE];

    bool Init(const char* portName);
    void ReadSerialBytes();
    void UpdateInput(ArduinoInput* arduinoIn);
};
