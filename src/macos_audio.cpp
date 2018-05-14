#include "macos_audio.h"

OSStatus MacOSAudioUnitCallback(void* inRefCon,
	AudioUnitRenderActionFlags* ioActionFlags,
	const AudioTimeStamp* inTimeStamp,
	UInt32 inBusNumber, UInt32 inNumberFrames,
	AudioBufferList* ioData)
{
	// NOTE(jeff): Don't do anything too time consuming in this function.
	//             It is a high-priority "real-time" thread.
	//             Even too many printf calls can throw off the timing.
	#pragma unused(ioActionFlags)
	#pragma unused(inTimeStamp)
	#pragma unused(inBusNumber)

	//double currentPhase = *((double*)inRefCon);

	MacOSAudio* audio = ((MacOSAudio*)inRefCon);

	if (audio->readCursor == audio->writeCursor) {
		//audio->soundBuffer.SampleCount = 0;
		//printf("AudioCallback: No Samples Yet!\n");
	}

	//printf("AudioCallback: SampleCount = %d\n", SoundOutput->SoundBuffer.SampleCount);

	int sampleCount = inNumberFrames;
	/*if (soundOutput->SoundBuffer.SampleCount < inNumberFrames)
	{
		SampleCount = SoundOutput->SoundBuffer.SampleCount;
	}*/

	int16* outputBufferL = (int16*)ioData->mBuffers[0].mData;
	int16* outputBufferR = (int16*)ioData->mBuffers[1].mData;

	for (uint32 i = 0; i < sampleCount; i++) {
		outputBufferL[i] = *audio->readCursor++;
		outputBufferR[i] = *audio->readCursor++;

		/*if ((char*)SoundOutput->ReadCursor >= (char*)((char*)audio->CoreAudioBuffer + SoundOutput->SoundBufferSize))
		{
			//printf("Callback: Read cursor wrapped!\n");
			SoundOutput->ReadCursor = SoundOutput->CoreAudioBuffer;
		}*/
	}

	for (uint32 i = sampleCount; i < inNumberFrames; i++) {
		outputBufferL[i] = 0.0;
		outputBufferR[i] = 0.0;
	}

	return noErr;
}

void OSXInitCoreAudio(MacOSAudio* macOSAudio)
{
	AudioComponentDescription acd;
	acd.componentType         = kAudioUnitType_Output;
	acd.componentSubType      = kAudioUnitSubType_DefaultOutput;
	acd.componentManufacturer = kAudioUnitManufacturer_Apple;

	AudioComponent outputComponent = AudioComponentFindNext(NULL, &acd);

	AudioComponentInstanceNew(outputComponent, &macOSAudio->audioUnit);
	AudioUnitInitialize(macOSAudio->audioUnit);

#if 1 // uint16
	//AudioStreamBasicDescription asbd;
	/*macOSAudio->audioDescriptor.mSampleRate       = macOSAudio->SoundBuffer.SamplesPerSecond;*/
	macOSAudio->audioDescriptor.mFormatID         = kAudioFormatLinearPCM;
	macOSAudio->audioDescriptor.mFormatFlags      = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsNonInterleaved | kAudioFormatFlagIsPacked;
	macOSAudio->audioDescriptor.mFramesPerPacket  = 1;
	macOSAudio->audioDescriptor.mChannelsPerFrame = 2; // Stereo
	macOSAudio->audioDescriptor.mBitsPerChannel   = sizeof(int16) * 8;
	macOSAudio->audioDescriptor.mBytesPerFrame    = sizeof(int16); // don't multiply by channel count with non-interleaved!
	macOSAudio->audioDescriptor.mBytesPerPacket   = macOSAudio->audioDescriptor.mFramesPerPacket * macOSAudio->audioDescriptor.mBytesPerFrame;
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


	// TODO(jeff): Add some error checking...
	AudioUnitSetProperty(macOSAudio->audioUnit,
		kAudioUnitProperty_StreamFormat,
		kAudioUnitScope_Input,
		0,
		&macOSAudio->audioDescriptor,
		sizeof(macOSAudio->audioDescriptor));

	AURenderCallbackStruct cb;
	cb.inputProc = MacOSAudioUnitCallback;
	cb.inputProcRefCon = macOSAudio;

	AudioUnitSetProperty(macOSAudio->audioUnit,
		kAudioUnitProperty_SetRenderCallback,
		kAudioUnitScope_Global,
		0,
		&cb,
		sizeof(cb));

	AudioOutputUnitStart(macOSAudio->audioUnit);
}


void MacOSStopCoreAudio(MacOSAudio* macOSAudio)
{
	AudioOutputUnitStop(macOSAudio->audioUnit);
	AudioUnitUninitialize(macOSAudio->audioUnit);
	AudioComponentInstanceDispose(macOSAudio->audioUnit);
}
