/*
VAST Dynamics
*/

#pragma once

#include "VASTEngineHeader.h"
#include "FX/VASTStereoDelay.h"
#include "FX/VASTReverb.h"
#include "FX/VASTChorus.h"
#include "FX/VASTDistortion.h"
#include "FX/VASTEq.h"
#include "FX/VASTCombFilterEffect.h"
#include "FX/VASTCompressorExpander.h"
#include "FX/VASTMultibandCompressor.h"
#include "FX/VASTBitcrush.h"
#include "FX/VASTFormantFilter.h"
#include "FX/VASTPhaser.h"
#include "FX/VASTFlanger.h"
#include "FX/VASTAtomizer.h"
#include "FX/VASTWaveshaper.h"
#include "VASTSettings.h"
#include "Oversampler/VASTOversampler.h"

class VASTAudioProcessor; //forward declaration
class CVASTFXBus {
public:
	CVASTFXBus(VASTAudioProcessor* processor, int busnr);
	~CVASTFXBus();

	//enum EnvelopeMode { unipolar, biploar };
	
	struct insertEffect {
		//bool isOn = false;
		bool isChain = false;
		bool needsOversampling = false;
		CVASTEffect* effectPlugin;
	};

	void init(CVASTSettings &set);
	void initCompatibilityParameters();
	void initCompatibilityParameters5();

	void prepareToPlay(double sampleRate, int samplesPerBlock);
	void processBuffers(sRoutingBuffers& routingBuffers, MidiBuffer& midiBuffer);
	void updateTiming();
	OwnedArray<insertEffect> effectBus;
	void swapSlots(int first, int second);
	int getSequence(int slot) {
		return mFXBusSequence[slot];
	}
	int findPosition(int slot);

	void initSequence();
	void getValueTreeState(ValueTree* tree, UndoManager* undoManager);
	void setValueTreeState(ValueTree* tree);

private:
	VASTAudioProcessor* myProcessor;
	int myBusnr = 0;
	CVASTSettings* m_Set;
	int m_chainEffects = 0;

	Array<int> mFXBusSequence;

	//Multiband
	CVASTBiQuad m_lowPassBiQuadL;
	CVASTBiQuad m_lowPassBiQuadR;
	bool m_lastCycleLowBandCalculated = true;
	juce::ScopedPointer<AudioSampleBuffer> m_lowbandMono;
	juce::ScopedPointer<AudioSampleBuffer> m_chainBuffer;
	juce::ScopedPointer<AudioSampleBuffer> m_chainProc;
	juce::ScopedPointer<AudioSampleBuffer> m_chainResult;

	//Oversampling
	CVASTOversampler m_Oversampler;
	CVASTOversampler m_Oversampler2;
	CVASTOversampler m_Oversampler3;
	juce::ScopedPointer<AudioSampleBuffer> m_inBufferOversampled;
	
	CVASTEq m_Eq;
	CVASTChorus m_Chorus;
	CVASTDistortion m_Distortion;
	CVASTCombFilterEffect m_CombFilter;
	CVASTCompressorExpander m_CompressorExpander;
	CVASTMultibandCompressor m_MultibandCompressor;
	CVASTBitcrush m_Bitcrush;
	CVASTFormantFilter m_FormantFilter;
	CVASTPhaser m_Phaser;
	CVASTFlanger m_Flanger;
	CVASTAtomizer m_Atomizer;
	CVASTStereoDelay m_StereoDelay;
	CVASTReverb m_Reverb;
	CVASTWaveshaper m_Waveshaper;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CVASTFXBus)
};