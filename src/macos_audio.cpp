#include "macos_audio.h"

OSStatus MacOSAudioUnitCallback(void* inRefCon,
	AudioUnitRenderActionFlags* ioActionFlags,
	const AudioTimeStamp* inTimeStamp,
	UInt32 inBusNumber, UInt32 inNumberFrames,
	AudioBufferList* ioData)
{
	#pragma unused(ioActionFlags)
	#pragma unused(inTimeStamp)
	#pragma unused(inBusNumber)

	MacOSAudio* audio = ((MacOSAudio*)inRefCon);

	int sampleCount = inNumberFrames;
	// TODO handle write cursor before read cursor wrap
	int newSamples = audio->writeCursor - audio->readCursor;
	if (audio->writeCursor < audio->readCursor) {
		newSamples = audio->bufferSizeSamples - audio->readCursor
			+ audio->writeCursor;
	}
	if (newSamples < inNumberFrames) {
		sampleCount = newSamples;
	}
	//DEBUG_PRINT("new samples: %d\n", sampleCount);

	int16* outBufferL = (int16*)ioData->mBuffers[0].mData;
	int16* outBufferR = (int16*)ioData->mBuffers[1].mData;
	for (int i = 0; i < sampleCount; i++) {
		int sample = (audio->readCursor + i) % audio->bufferSizeSamples;
		outBufferL[i] = audio->buffer[sample * audio->channels];
		outBufferR[i] = audio->buffer[sample * audio->channels + 1];
	}

	audio->readCursor = (audio->readCursor + sampleCount)
		% audio->bufferSizeSamples;

	for (int i = sampleCount; i < inNumberFrames; i++) {
		outBufferL[i] = 0;
		outBufferR[i] = 0;
	}

	return noErr;
}

void MacOSInitCoreAudio(MacOSAudio* macOSAudio,
    int sampleRate, int channels, int bufferSizeSamples)
{
	macOSAudio->sampleRate = sampleRate;
	macOSAudio->channels = channels;
	macOSAudio->bufferSizeSamples = bufferSizeSamples;
	macOSAudio->buffer = (int16*)mmap(0,
		bufferSizeSamples * channels * sizeof(int16),
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	macOSAudio->readCursor = 0;
	macOSAudio->writeCursor = 0;

	AudioComponentDescription acd;
	acd.componentType         = kAudioUnitType_Output;
	acd.componentSubType      = kAudioUnitSubType_DefaultOutput;
	acd.componentManufacturer = kAudioUnitManufacturer_Apple;

	AudioComponent outputComponent = AudioComponentFindNext(NULL, &acd);

	AudioComponentInstanceNew(outputComponent, &macOSAudio->audioUnit);
	AudioUnitInitialize(macOSAudio->audioUnit);

#if 1 // int16
	//AudioStreamBasicDescription asbd;
	macOSAudio->audioDescriptor.mSampleRate       = sampleRate;
	macOSAudio->audioDescriptor.mFormatID         = kAudioFormatLinearPCM;
	macOSAudio->audioDescriptor.mFormatFlags      =
		kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsNonInterleaved
		| kAudioFormatFlagIsPacked;
	macOSAudio->audioDescriptor.mFramesPerPacket  = 1;
	macOSAudio->audioDescriptor.mChannelsPerFrame = channels; // Stereo
	macOSAudio->audioDescriptor.mBitsPerChannel   = sizeof(int16) * 8;
	// don't multiply by channel count with non-interleaved!
	macOSAudio->audioDescriptor.mBytesPerFrame    = sizeof(int16);
	macOSAudio->audioDescriptor.mBytesPerPacket   =
		macOSAudio->audioDescriptor.mFramesPerPacket
		* macOSAudio->audioDescriptor.mBytesPerFrame;
#else // floating point - this is the "native" format on the Mac
	AudioStreamBasicDescription asbd;
	SoundOutput->AudioDescriptor.mSampleRate       = SoundOutput->SamplesPerSecond;
	SoundOutput->AudioDescriptor.mFormatID         = kAudioFormatLinearPCM;
	SoundOutput->AudioDescriptor.mFormatFlags      = kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved;
	SoundOutput->AudioDescriptor.mFramesPerPacket  = 1;
	SoundOutput->AudioDescriptor.mChannelsPerFrame = 2;
	SoundOutput->AudioDescriptor.mBitsPerChannel   = sizeof(Float32) * 8; // 1 * sizeof(Float32) * 8;
	SoundOutput->AudioDescriptor.mBytesPerFrame    = sizeof(Float32);
	SoundOutput->AudioDescriptor.mBytesPerPacket   = SoundOutput->AudioDescriptor.mFramesPerPacket * SoundOutput->AudioDescriptor.mBytesPerFrame;
#endif


	// TODO Add some error checking...
	// But this is a mess. OSStatus is what? what is success? sigh
	AudioUnitSetProperty(macOSAudio->audioUnit,
		kAudioUnitProperty_StreamFormat,
		kAudioUnitScope_Input,
		0,
		&macOSAudio->audioDescriptor,
		sizeof(macOSAudio->audioDescriptor)
	);

	AURenderCallbackStruct cb;
	cb.inputProc = MacOSAudioUnitCallback;
	cb.inputProcRefCon = macOSAudio;

	AudioUnitSetProperty(macOSAudio->audioUnit,
		kAudioUnitProperty_SetRenderCallback,
		kAudioUnitScope_Global,
		0,
		&cb,
		sizeof(cb)
	);

	AudioOutputUnitStart(macOSAudio->audioUnit);
}


void MacOSStopCoreAudio(MacOSAudio* macOSAudio)
{
	AudioOutputUnitStop(macOSAudio->audioUnit);
	AudioUnitUninitialize(macOSAudio->audioUnit);
	AudioComponentInstanceDispose(macOSAudio->audioUnit);
}
