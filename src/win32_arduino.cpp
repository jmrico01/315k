#include "win32_arduino.h"

#include "km_lib.h"
#include "km_log.h"

#define ARDUINO_PREFIX_BYTES 4
#define ARDUINO_PAYLOAD_BYTES (1 + ARDUINO_DIGITAL_INPUTS + (ARDUINO_ANALOG_INPUTS * sizeof(uint16)))

#define ARDUINO_ANALOG_MAX_VALUE 1023

const char* Win32Arduino::ARDUINO_PORT_NAME = "\\\\.\\COM5";

// Attempts to find a prefix data packet in the given buffer range
// If found, returns the index immediately after the end of the prefix
// If not found, returns indexStart
internal uint64 FindFirstPrefix(uint64 indexStart, uint64 indexEnd,
	const uint8* buffer, uint64 bufferSize)
{
	int matches = 0;
	uint64 index = indexStart;
	while (index != indexEnd && matches < ARDUINO_PREFIX_BYTES) {
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

	if (matches == ARDUINO_PREFIX_BYTES) {
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
		FILE_FLAG_NO_BUFFERING, // FILE_ATTRIBUTE_NORMAL,
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

void Win32Arduino::ReadSerialBytes()
{
	if (!ClearCommError(handle, &errors, &status)) {
		LOG_ERROR("Failed call to ClearCommError\n");
	}

	if (status.cbInQue > 0) {
		DWORD bytesToRead = status.cbInQue;
		DWORD bufferCapacity1, bufferCapacity2;
		if (bufferRead <= bufferWrite) {
			bufferCapacity1 = (DWORD)(ARDUINO_BUFFER_SIZE - bufferWrite);
			bufferCapacity2 = (DWORD)(bufferRead - 1);
		}
		else {
			bufferCapacity1 = (DWORD)(bufferRead - bufferWrite - 1);
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
}

void Win32Arduino::UpdateInput(ArduinoInput* arduinoIn)
{
	ReadSerialBytes();

	arduinoIn->pedal.transitions = 0;

	while (true) {
		uint64 dataIndex = FindFirstPrefix(bufferRead, bufferWrite, buffer, ARDUINO_BUFFER_SIZE);
		if (dataIndex == bufferRead) {
			// No prefix, no structured data to read
			break;
		}

		uint64 payloadBytes;
		if (dataIndex <= bufferWrite) {
			payloadBytes = bufferWrite - dataIndex;
		}
		else {
			payloadBytes = ARDUINO_BUFFER_SIZE - dataIndex + bufferWrite;
		}

		if (payloadBytes < ARDUINO_PAYLOAD_BYTES) {
			// Prefix followed by incomplete data payload, nothing to do
			break;
		}

		uint8 activeChannel = buffer[dataIndex];
		if (activeChannel >= ARDUINO_CHANNELS) {
			LOG_ERROR("Invalid channel %u, dropping data packet\n", activeChannel);
			bufferRead = dataIndex;
			continue;
		}
		if (++dataIndex >= ARDUINO_BUFFER_SIZE) {
			dataIndex -= ARDUINO_BUFFER_SIZE;
		}

		// TODO this should be a loop (or bit mask parse) for every digital input
		bool pedalOn = buffer[dataIndex] != 0;
		if (++dataIndex >= ARDUINO_BUFFER_SIZE) {
			dataIndex -= ARDUINO_BUFFER_SIZE;
		}
		if (arduinoIn->pedal.transitions == 1 || arduinoIn->pedal.isDown != pedalOn) {
			arduinoIn->pedal.transitions = 1;
		}
		else {
			arduinoIn->pedal.transitions = 0;
		}
		arduinoIn->pedal.isDown = pedalOn;

		uint64 analogInput = 0;
		while (analogInput < ARDUINO_ANALOG_INPUTS) {
			uint8 valueBytes[2];
			for (int i = 0; i < 2; i++) {
				valueBytes[i] = buffer[dataIndex];
				if (++dataIndex >= ARDUINO_BUFFER_SIZE) {
					dataIndex -= ARDUINO_BUFFER_SIZE;
				}
			}
			uint16 value = *((uint16*)valueBytes);
			float32 valueNormalized = (float32)value / ARDUINO_ANALOG_MAX_VALUE;
			arduinoIn->analogValues[activeChannel][analogInput] = valueNormalized;
			analogInput++;
		}

		bufferRead = dataIndex;
	}
}
