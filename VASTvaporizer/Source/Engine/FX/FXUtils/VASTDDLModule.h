/*
VAST Dynamics Audio Software (TM)
*/

#pragma once

#include "../../VASTPluginConstants.h"
#include "../../VASTSettings.h"

// abstract base class for DSP filters
class CDDLModule
{
public:
	CDDLModule(void);

	// 2. One Time Destruction
	virtual ~CDDLModule(void);

	// 3. The Prepare For Play Function is called just before audio streams
	bool prepareForPlay();

	// 4. processAudioFrame() processes an audio input to create an audio output
	bool processAudioFrame(float* pInputBuffer, float* pOutputBuffer, MYUINT uNumInputChannels, MYUINT uNumOutputChannels);

	void init(CVASTSettings &set);

	float m_fDelayInSamples;
	float m_fFeedback;
	float m_fWetLevel;

	float m_fDDLOutput; // added for dimension chorus

	void cookVariables();
	void resetDelay(int nDelayLength);
	
	juce::ScopedPointer<AudioSampleBuffer> m_pBuffer;

	int m_nReadIndex;
	int m_nWriteIndex;

	bool  m_bUseExternalFeedback; // flag for enabling/disabling
	float m_fFeedbackIn;		// the user supplied feedback sample value

	// current FB is fb*output
	float getCurrentFeedbackOutput(){ return m_fFeedback * m_pBuffer->getReadPointer(0)[m_nReadIndex]; }

	// set the feedback sample
	void  setCurrentFeedbackInput(float f){ m_fFeedbackIn = f; }

	// enable/disable external FB source
	void  setUsesExternalFeedback(bool b){ m_bUseExternalFeedback = false; }

	float m_fDelay_ms;
	float m_fFeedback_pct;
	float m_fWetLevel_pct;
	CVASTSettings *m_Set;
};