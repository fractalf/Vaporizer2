/*
VAST Dynamics Audio Software (TM)
*/

#pragma once
#pragma warning(disable: 4244) //convert double float
#include "../VASTEngineHeader.h"
#include "VASTDelay.h"

// derived from CDelay
class CLPFCombFilter : public CDelay
{
public:
	// constructor/destructor
	CLPFCombFilter(void);
	~CLPFCombFilter(void);

	// members
protected:
	float m_fComb_g; 	// one Comb coefficient
	float m_fLPF_g; 	// one LPF coefficient
	float m_fLPF_z1; 	// one sample delay

public:
	// set our g value directly
	void setComb_g(float fCombg){m_fComb_g = fCombg;}

	// set our g value using RT60
	void setComb_g_with_RTSixty(float fRT)
	{
		float fExponent = -3.0*m_fDelayInSamples*(1.0/m_nSampleRate);
		fRT /= 1000.0; // RT is in mSec!

		m_fComb_g = pow((float)10.0, fExponent/fRT);
	}

	// set the LPF gain
	// NOTE: call setComb_g_with_RTSixty FIRST, then this
	void setLPF_g(float fOverAllGain)
	{
		// g2 = g*(1-g1)
		m_fLPF_g = fOverAllGain*(1.0 - m_fComb_g);
	}


	// overrides
	// init
	void init(int nDelayLength);

	// process something
	bool processAudio(float* pInput, float* pOutput);

};
