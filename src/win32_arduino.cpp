#include "win32_arduino.h"

#include "km_lib.h"
#include "km_log.h"

#define ARDUINO_DATA_PACKET_SIZE (1 + 4)

const int ARDUINO_WAIT_TIME = 2000;

const char* Win32Arduino::ARDUINO_PORT_NAME = "\\\\.\\COM5";

// Attempts to find a packet prefix in the given buffer range
// If found, returns the index immediately after the end of the prefix
// If not found, returns indexStart
internal uint64 FindPacketPrefix(uint64 indexStart, uint64 indexEnd,
    const uint8* buffer, uint64 bufferSize)
{
    int matches = 0;
    uint64 index = indexStart;
    while (index != indexEnd && matches < ARDUINO_DATA_PACKET_SIZE) {
        if (buffer[index] == 0xff) {
            matches++;
        }
        else {
            matches = 0;
        }

        index++;
        if (index >= bufferSize) {
            index -= bufferSize;
        }
    }

    if (matches == ARDUINO_DATA_PACKET_SIZE) {
        return index;
    }

    return indexStart;
}

bool Win32Arduino::Init(const char* portName)
{
    handle = CreateFileA(static_cast<LPCSTR>(portName),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        LOG_ERROR("Failed to open Arduino file handle for port %s\n", portName);
        return false;
    }

    DCB dcbSerialParameters = {};
    if (!GetCommState(handle, &dcbSerialParameters)) {
        LOG_ERROR("Failed to get serial parameters for port %s\n", portName);
        return false;
    }

    dcbSerialParameters.BaudRate = CBR_57600;
    dcbSerialParameters.ByteSize = 8;
    dcbSerialParameters.StopBits = ONESTOPBIT;
    dcbSerialParameters.Parity = NOPARITY;
    dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

    if (!SetCommState(handle, &dcbSerialParameters)) {
        LOG_ERROR("Failed to set serial parameters for port %s\n", portName);
        return false;
    }

    PurgeComm(handle, PURGE_RXCLEAR | PURGE_TXCLEAR);

    bufferRead = 0;
    bufferWrite = 0;

    for (int i = 0; i < 100; i++) {
        DWORD bytesSent;
        uint8 ping = 0xff;
        WriteFile(handle, &ping, 1, &bytesSent, NULL);
    }

    return true;
}

void Win32Arduino::UpdateInput(ArduinoInput* arduinoIn)
{
    if (!ClearCommError(handle, &errors, &status)) {
        LOG_ERROR("Failed call to ClearCommError\n");
    }

    if (status.cbInQue > 0) {
        DWORD bytesToRead = status.cbInQue;
        DWORD bufferCapacity1, bufferCapacity2;
        if (bufferRead <= bufferWrite) {
            bufferCapacity1 = (DWORD)(ARDUINO_BUFFER_SIZE - bufferWrite);
            bufferCapacity2 = (DWORD)(bufferRead);
        }
        else {
            bufferCapacity1 = (DWORD)(bufferRead - bufferWrite);
            bufferCapacity2 = 0;
        }

        DWORD bytesToRead1, bytesToRead2;
        if (bytesToRead <= bufferCapacity1) {
            bytesToRead1 = bytesToRead;
            bytesToRead2 = 0;
        }
        else {
            bytesToRead1 = bufferCapacity1;
            bytesToRead -= bufferCapacity1;
            if (bytesToRead <= bufferCapacity2) {
                bytesToRead2 = bytesToRead;
            }
            else {
                bytesToRead2 = bufferCapacity2;
            }
        }

        DWORD bytesRead;
        if (!ReadFile(handle, &buffer[bufferWrite], bytesToRead1, &bytesRead, NULL)) {
            LOG_ERROR("Failed first read on Arduino data\n");
        }
        DWORD totalBytesRead = bytesRead;
        if (bytesToRead2 > 0) {
            if (!ReadFile(handle, buffer, bytesToRead2, &bytesRead, NULL)) {
                LOG_ERROR("Failed second read on Arduino data\n");
            }
            totalBytesRead += bytesRead;
        }

        bufferWrite += totalBytesRead;
        if (bufferWrite >= ARDUINO_BUFFER_SIZE) {
            bufferWrite -= ARDUINO_BUFFER_SIZE;
        }
    }

    uint64 afterPrefix1 = FindPacketPrefix(bufferRead, bufferWrite, buffer, ARDUINO_BUFFER_SIZE);
    if (afterPrefix1 == bufferRead) {
        return;
    }
    uint64 afterPrefix2 = FindPacketPrefix(afterPrefix1, bufferWrite, buffer, ARDUINO_BUFFER_SIZE);
    if (afterPrefix2 == afterPrefix1) {
        return;
    }

    uint64 beforePrefix2;
    if (afterPrefix2 >= ARDUINO_DATA_PACKET_SIZE) {
        beforePrefix2 = afterPrefix2 - ARDUINO_DATA_PACKET_SIZE;
    }
    else {
        beforePrefix2 = afterPrefix2 + ARDUINO_BUFFER_SIZE - ARDUINO_DATA_PACKET_SIZE;
    }

    if (afterPrefix1 == beforePrefix2) {
        LOG_ERROR("Arduino prefixes with no payload data in between\n");
        return;
    }

    uint8 activeChannel = buffer[afterPrefix1];
    if (++afterPrefix1 >= ARDUINO_BUFFER_SIZE) {
        afterPrefix1 -= ARDUINO_BUFFER_SIZE;
    }

    bool pedalOn = buffer[afterPrefix1] != 0;
    arduinoIn->pedal.transitions = arduinoIn->pedal.isDown == pedalOn ? 0 : 1;
    arduinoIn->pedal.isDown = pedalOn;
    if (++afterPrefix1 >= ARDUINO_BUFFER_SIZE) {
        afterPrefix1 -= ARDUINO_BUFFER_SIZE;
    }

    uint64 payloadBytes;
    if (afterPrefix1 < beforePrefix2) {
        payloadBytes = beforePrefix2 - afterPrefix1;
    }
    else {
        payloadBytes = ARDUINO_BUFFER_SIZE - afterPrefix1;
        payloadBytes += beforePrefix2;
    }

    if (payloadBytes % ARDUINO_DATA_PACKET_SIZE != 0) {
        LOG_ERROR("Data payload not a multiple of packet size: %llu\n", payloadBytes);
        bufferRead = beforePrefix2;
        return;
    }

    uint64 numPackets = payloadBytes / ARDUINO_DATA_PACKET_SIZE;
    uint64 packet = 0;
    uint64 index = afterPrefix1;
    while (packet < numPackets) {
        uint8 inputNumber = buffer[index];
        if (++index >= ARDUINO_BUFFER_SIZE) {
            index -= ARDUINO_BUFFER_SIZE;
        }

        uint8 valueBytes[4];
        for (int i = 0; i < 4; i++) {
            valueBytes[i] = buffer[index];
            if (++index >= ARDUINO_BUFFER_SIZE) {
                index -= ARDUINO_BUFFER_SIZE;
            }
        }
        float32 value;
        MemCopy(&value, valueBytes, 4);

        arduinoIn->analogValues[activeChannel][inputNumber] = value;
        packet++;
    }

    bufferRead = beforePrefix2;
}
