/*
VAST Dynamics Audio Software (TM)
*/

#include "../VASTEngineHeader.h"
#include "VASTBiQuadFilter.h"
#include "../VASTPluginConstants.h"

// CBiQuad Implementation ----------------------------------------------------------------
//
CBiQuad::CBiQuad(void)
{
}

CBiQuad::~CBiQuad(void)
{
}

CVASTBiQuad::CVASTBiQuad(void) :CBiQuad()
{
}

CVASTBiQuad::~CVASTBiQuad(void)
{
}

void CVASTBiQuad::calcBiquad(MYUINT type, double dFrequency, double dSampleRate, double dQ, double dpeakGain) {
	// see http://www.earlevel.com/main/2011/01/02/biquad-formulas/
	double norm = 0;	
	double K = tan(M_PI * dFrequency / dSampleRate);
	switch (type) {
	case LOWPASS:
		norm = 1 / (1 + K / dQ + K * K);
		m_d_a0 = K * K * norm;
		m_d_a1 = 2 * m_d_a0;
		m_d_a2 = m_d_a0;
		m_d_b1 = 2 * (K * K - 1) * norm;
		m_d_b2 = (1 - K / dQ + K * K) * norm;
		break;

	case HIGHPASS:
		norm = 1 / (1 + K / dQ + K * K);
		m_d_a0 = 1 * norm;
		m_d_a1 = -2 * m_d_a0;
		m_d_a2 = m_d_a0;
		m_d_b1 = 2 * (K * K - 1) * norm;
		m_d_b2 = (1 - K / dQ + K * K) * norm;
		break;

	case BANDPASS:
		norm = 1 / (1 + K / dQ + K * K);
		m_d_a0 = K / dQ * norm;
		m_d_a1 = 0;
		m_d_a2 = -m_d_a0;
		m_d_b1 = 2 * (K * K - 1) * norm;
		m_d_b2 = (1 - K / dQ + K * K) * norm;
		break;

	case NOTCH:
		norm = 1 / (1 + K / dQ + K * K);
		m_d_a0 = (1 + K * K) * norm;
		m_d_a1 = 2 * (K * K - 1) * norm;
		m_d_a2 = m_d_a0;
		m_d_b1 = m_d_a1;
		m_d_b2 = (1 - K / dQ + K * K) * norm;
		break;

	case PEAK: {
		double V = powf(10.0f, abs(dpeakGain) / 20.0f);
		if (dpeakGain >= 0) {    // boost
			norm = 1 / (1 + 1 / dQ * K + K * K);
			m_d_a0 = (1 + V / dQ * K + K * K) * norm;
			m_d_a1 = 2 * (K * K - 1) * norm;
			m_d_a2 = (1 - V / dQ * K + K * K) * norm;
			m_d_b1 = m_d_a1;
			m_d_b2 = (1 - 1 / dQ * K + K * K) * norm;
		}
		else {    // cut
			norm = 1 / (1 + V / dQ * K + K * K);
			m_d_a0 = (1 + 1 / dQ * K + K * K) * norm;
			m_d_a1 = 2 * (K * K - 1) * norm;
			m_d_a2 = (1 - 1 / dQ * K + K * K) * norm;
			m_d_b1 = m_d_a1;
			m_d_b2 = (1 - V / dQ * K + K * K) * norm;
		}
	}
		break;
	case LOWSHELF: {
		double V = powf(10.0f, abs(dpeakGain) / 20.0f);
		if (dpeakGain >= 0) {    // boost
			norm = 1 / (1 + M_SQRT2 * K + K * K);
			m_d_a0 = (1 + sqrt(2 * V) * K + V * K * K) * norm;
			m_d_a1 = 2 * (V * K * K - 1) * norm;
			m_d_a2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
			m_d_b1 = 2 * (K * K - 1) * norm;
			m_d_b2 = (1 - M_SQRT2 * K + K * K) * norm;
		}
		else {    // cut
			norm = 1 / (1 + sqrt(2 * V) * K + V * K * K);
			m_d_a0 = (1 + M_SQRT2 * K + K * K) * norm;
			m_d_a1 = 2 * (K * K - 1) * norm;
			m_d_a2 = (1 - M_SQRT2 * K + K * K) * norm;
			m_d_b1 = 2 * (V * K * K - 1) * norm;
			m_d_b2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
		}
	}
		break;
	case HIGHSHELF: {
		double V = powf(10.0f, abs(dpeakGain) / 20.0f);
		if (dpeakGain >= 0) {    // boost
			norm = 1 / (1 + M_SQRT2 * K + K * K);
			m_d_a0 = (V + sqrt(2 * V) * K + K * K) * norm;
			m_d_a1 = 2 * (K * K - V) * norm;
			m_d_a2 = (V - sqrt(2 * V) * K + K * K) * norm;
			m_d_b1 = 2 * (K * K - 1) * norm;
			m_d_b2 = (1 - M_SQRT2 * K + K * K) * norm;
		}
		else {    // cut
			norm = 1 / (V + sqrt(2 * V) * K + K * K);
			m_d_a0 = (1 + M_SQRT2 * K + K * K) * norm;
			m_d_a1 = 2 * (K * K - 1) * norm;
			m_d_a2 = (1 - M_SQRT2 * K + K * K) * norm;
			m_d_b1 = 2 * (K * K - V) * norm;
			m_d_b2 = (V - sqrt(2 * V) * K + K * K) * norm;
		}
	}
		break;
	}
}

void CVASTBiQuad::copySettingsFrom(CVASTBiQuad *source) {
	m_d_a0 = source->m_d_a0;
	m_d_a1 = source->m_d_a1;
	m_d_a2 = source->m_d_a2;
	m_d_b1 = source->m_d_b1;
	m_d_b2 = source->m_d_b2;
}