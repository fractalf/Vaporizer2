/*
VAST Dynamics Audio Software (TM)

- Handles and dispatches multiple single notes
. Does ARP
*/

#include "VASTPoly.h"
#include "VASTSettings.h"
#include "VASTSingleNote.h"
#include "VASTEngineHeader.h"
#include "VASTSampler.h"
#include "VASTSynthesiser.h"

//for debug info
#include <iostream>
#include <fstream>
#include <string>
/* destructor()
Destroy variables allocated in the contructor()
*/
CVASTPoly::~CVASTPoly(void) {
}

void CVASTPoly::init() {
	//executed once
	m_OscillatorSynthesizer.init(m_Set, this);

	for (int i = 0; i < C_MAX_POLY; i++) {
		m_singleNote[i] = new CVASTSingleNote(); //new is OK - will be stored in owned array as voices
		m_singleNote[i]->init(*m_Set, this, i); //voice->mVoiceNo
	}

	for (int i = 0; i < C_MAX_POLY; i++) { //check if things duplicated here
		m_singleNote[i]->prepareForPlay();
	}

	m_global_LFO_Osc[0].init(*m_Set);
	m_global_LFO_Osc[0].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO1, 1, 0, 0, 0);

	m_global_LFO_Osc[1].init(*m_Set);
	m_global_LFO_Osc[1].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO2, 1, 0, 0, 0);

	m_global_LFO_Osc[2].init(*m_Set);
	m_global_LFO_Osc[2].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO3, 1, 0, 0, 0);

	m_global_LFO_Osc[3].init(*m_Set);
	m_global_LFO_Osc[3].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO4, 1, 0, 0, 0);

	m_global_LFO_Osc[4].init(*m_Set);
	m_global_LFO_Osc[4].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO5, 1, 0, 0, 0);

	for (int stepSeq = 0; stepSeq < 3; stepSeq++) {
		if (stepSeq == 0) {
			m_Set->m_StepSeqData[stepSeq].setStepSeqTime(*m_Set->m_State->m_fStepSeqSpeed_STEPSEQ1); //is in ms
			m_Set->m_StepSeqData_changed[stepSeq].setStepSeqTime(*m_Set->m_State->m_fStepSeqSpeed_STEPSEQ1); //is in ms
		} else  if (stepSeq == 1) {
			m_Set->m_StepSeqData[stepSeq].setStepSeqTime(*m_Set->m_State->m_fStepSeqSpeed_STEPSEQ2); //is in ms
			m_Set->m_StepSeqData_changed[stepSeq].setStepSeqTime(*m_Set->m_State->m_fStepSeqSpeed_STEPSEQ2); //is in ms
		} else if (stepSeq == 2) {
			m_Set->m_StepSeqData[stepSeq].setStepSeqTime(*m_Set->m_State->m_fStepSeqSpeed_STEPSEQ3); //is in ms
			m_Set->m_StepSeqData_changed[stepSeq].setStepSeqTime(*m_Set->m_State->m_fStepSeqSpeed_STEPSEQ3); //is in ms
		}

		m_StepSeq_Envelope[stepSeq].init(*m_Set, m_Set->m_StepSeqData[stepSeq], m_Set->m_StepSeqData_changed[stepSeq], 0, 0, stepSeq); //voiceno 0??
		struct timeval tp;
		m_Set->_gettimeofday(&tp);
		m_StepSeq_Envelope[stepSeq].noteOn(tp.tv_sec * 1000 + tp.tv_usec / 1000, false);
	}

	const double smoothTime = 0.005;
	m_fCustomModulator1_smoothed.reset(m_Set->m_nSampleRate, smoothTime);
	m_fCustomModulator2_smoothed.reset(m_Set->m_nSampleRate, smoothTime);
	m_fCustomModulator3_smoothed.reset(m_Set->m_nSampleRate, smoothTime);
	m_fCustomModulator4_smoothed.reset(m_Set->m_nSampleRate, smoothTime);
	m_fARP_Speed_smoothed.reset(m_Set->m_nSampleRate, 0.0005);

	//Oscillator voices
	m_OscillatorSynthesizer.setNoteStealingEnabled(true);	
	m_OscillatorSynthesizer.setMinimumRenderingSubdivisionSize(32, false); //not strict
	for (int i = 0; i < C_MAX_POLY; i++) {
		m_OscillatorSynthesizer.addVoice((VASTSynthesiserVoice*)m_singleNote[i]);
	}
	
	VASTSynthesiserSound* l_vastSound = new VASTSynthesiserSound();
	m_OscillatorSynthesizer.addSound(l_vastSound);
	m_shallInitARP = false;

	//qfilter
	m_QFilter.initQuadFilter(m_Set);
}

void CVASTPoly::prepareForPlay() {
	//executed multiple
	for (int bank = 0; bank < 4; bank++) {
		m_OscBank[bank]->prepareForPlay(m_Set->m_nExpectedSamplesPerBlock);
	}
	for (int i = 0; i < C_MAX_POLY; i++) { //check if things duplicated here
		m_singleNote[i]->prepareForPlay();
	}
	

	initArp();
	m_ppq_playing = false;
	
	m_global_LFO_Osc[0].init(*m_Set);
	m_global_LFO_Osc[0].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO1, 1, 0, 0, 0);

	m_global_LFO_Osc[1].init(*m_Set);
	m_global_LFO_Osc[1].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO2, 1, 0, 0, 0);

	m_global_LFO_Osc[2].init(*m_Set);
	m_global_LFO_Osc[2].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO3, 1, 0, 0, 0);

	m_global_LFO_Osc[3].init(*m_Set);
	m_global_LFO_Osc[3].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO4, 1, 0, 0, 0);

	m_global_LFO_Osc[4].init(*m_Set);
	m_global_LFO_Osc[4].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO5, 1, 0, 0, 0);

	updateLFO(0);
	updateLFO(1);
	updateLFO(2);
	updateLFO(3);
	updateLFO(4);

	//Synthesiser
	m_OscillatorSynthesizer.initValues(); //CHECK TS
	m_OscillatorSynthesizer.setCurrentPlaybackSampleRate(m_Set->m_nSampleRate);

	updateVariables();
	m_QFilter.updateVariables();
}

void CVASTPoly::initArpInternal(MidiBuffer& midiMessages) {
	for (int note = 0; note < m_ARP_currentARPNoteValues.size(); note++) {
		if (m_ARP_currentARPNoteValues[note] > 0)
		{
			MidiMessage msg;
			int samplePos = 0;
			//will it be started in that buffer - then no note off! also at first init
			bool willBeStarted = false;
			for (MidiBuffer::Iterator it(midiMessages); it.getNextEvent(msg, samplePos);)
			{
				if ((msg.isNoteOn() && (msg.getNoteNumber() == m_ARP_currentARPNoteValues[note]))) {
					willBeStarted = true;
					break;
				}			
			}
			if (!willBeStarted) midiMessages.addEvent(MidiMessage::noteOff(1, m_ARP_currentARPNoteValues[note]), 0);
		}
	}

	m_fARP_Speed_smoothed.reset(m_Set->m_nSampleRate, 0.0005);
	m_ARP_midiInNotes.clear();
	m_ARP_currentARPNoteValues.clear();
	m_ARP_currentStep = 0;
	m_ARP_time = 0;
	m_ARP_direction = 1;
	m_arpHasActiveStepToFinish = false;
	m_shallInitARP = false;	
}

//void CVASTPoly::updateVariables(MYUINT OscType, MYUINT Polarity, float AttackTime, float DecayTime, float SustainLevel, float ReleaseTime, float OscDetune, int NumParallelOsc, float FilterCutoff, float FilterReso, int FilterOnOff, float LFOFreq) {
void CVASTPoly::updateVariables() {
	for (int i = 0; i < C_MAX_POLY; i++) {
		m_singleNote[i]->updateVariables();
	}	
}

int CVASTPoly::numNotesPlaying() { //UI only
	//const ScopedReadLock myScopedLock(m_Set->m_RoutingBuffers.mReadWriteLock); //CHECK THIS!!!

	int num = 0;
	for (int i = 0; i < C_MAX_POLY; i++) {
		if (m_singleNote[i]->isPlayingCalledFromUI() == true) num++;  //TODO is 0 OK here - not accurate!!!
	}
	return num;
}

int CVASTPoly::numOscsPlaying() { //UI only
	//const ScopedReadLock myScopedLock(m_Set->m_RoutingBuffers.mReadWriteLock); //CHECK THIS!!!

	int num = 0;
	for (int i = 0; i < C_MAX_POLY; i++) {
		if (m_singleNote[i]->isPlayingCalledFromUI() == true) { //TODO is 0 OK here - not accurate!!!
			num += m_singleNote[i]->getNumOscsPlaying();
		}
	}
	return num;
}

int CVASTPoly::getLastNotePlayed() { //-1 if none playing
	return m_OscillatorSynthesizer.getLastPlayedVoiceNo();
}

int CVASTPoly::getOldestNotePlayed() { //-1 if none playing
	return m_OscillatorSynthesizer.getOldestPlayedVoiceNo();
}

modMatrixInputState CVASTPoly::getOldestNotePlayedInputState(int currentFrame) {
	modMatrixInputState inputState;
	int voiceNo = getOldestNotePlayed(); // make parameter oldest or newest
	inputState.voiceNo = (voiceNo < 0) ? 0 : voiceNo;
	inputState.currentFrame = currentFrame;
	return inputState;
}

modMatrixInputState CVASTPoly::getLastNotePlayedInputState(int currentFrame) {
	modMatrixInputState inputState;
	int voiceNo = getLastNotePlayed(); // make parameter oldest or newest
	inputState.voiceNo = (voiceNo < 0) ? 0 : voiceNo;
	inputState.currentFrame = currentFrame;
	return inputState;
}

void CVASTPoly::updateLFO(int lfono) {
	switch (lfono) {
	case 0: {
		if (*m_Set->m_State->m_bLFOSynch_LFO1 == SWITCH::SWITCH_ON) {
			float l_fIntervalTime = m_Set->getIntervalTimeFromDAWBeats(*m_Set->m_State->m_uLFOTimeBeats_LFO1);

			if (*m_Set->m_State->m_bLFOPerVoice_LFO1 == SWITCH::SWITCH_ON) {
				for (int i = 0; i < C_MAX_POLY; i++) {
					m_singleNote[i]->m_LFO_Osc[0]->updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO1, 1, 0, 0, 0);
					m_singleNote[i]->m_LFO_Osc[0]->startLFOFrequency(1000.f / l_fIntervalTime, 0); //lfono 0
				}
			}
			else {
				m_global_LFO_Osc[0].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO1, 1, 0, 0, 0);
				m_global_LFO_Osc[0].startLFOFrequency(1000.f / l_fIntervalTime, 0); //lfono 0
			}
		}
		else {
			if (*m_Set->m_State->m_bLFOPerVoice_LFO1 == SWITCH::SWITCH_ON) {
				for (int i = 0; i < C_MAX_POLY; i++) {
					m_singleNote[i]->m_LFO_Osc[0]->updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO1, 1, 0, 0, 0);
					m_singleNote[i]->m_LFO_Osc[0]->startLFOFrequency(*m_Set->m_State->m_fLFOFreq_LFO1, 0); //lfono 0
				}
			}
			else {
				m_global_LFO_Osc[0].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO1, 1, 0, 0, 0);
				m_global_LFO_Osc[0].startLFOFrequency(*m_Set->m_State->m_fLFOFreq_LFO1, 0); //lfono 0
			}
		}
		break;
	}
	case 1: {
		if (*m_Set->m_State->m_bLFOSynch_LFO2 == SWITCH::SWITCH_ON) {
			float l_fIntervalTime = m_Set->getIntervalTimeFromDAWBeats(*m_Set->m_State->m_uLFOTimeBeats_LFO2);

			if (*m_Set->m_State->m_bLFOPerVoice_LFO2 == SWITCH::SWITCH_ON) {
				for (int i = 0; i < C_MAX_POLY; i++) {
					m_singleNote[i]->m_LFO_Osc[1]->updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO2, 1, 0, 0, 0);
					m_singleNote[i]->m_LFO_Osc[1]->startLFOFrequency(1000.f / l_fIntervalTime, 1); //lfono 1
				}
			}
			else {
				m_global_LFO_Osc[1].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO2, 1, 0, 0, 0);
				m_global_LFO_Osc[1].startLFOFrequency(1000.f / l_fIntervalTime, 1); //lfono 1
			}
		}
		else {
			if (*m_Set->m_State->m_bLFOPerVoice_LFO2 == SWITCH::SWITCH_ON) {
				for (int i = 0; i < C_MAX_POLY; i++) {
					m_singleNote[i]->m_LFO_Osc[1]->updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO2, 1, 0, 0, 0);
					m_singleNote[i]->m_LFO_Osc[1]->startLFOFrequency(*m_Set->m_State->m_fLFOFreq_LFO2, 1); //lfono 1
				}
			}
			else {
				m_global_LFO_Osc[1].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO2, 1, 0, 0, 0);
				m_global_LFO_Osc[1].startLFOFrequency(*m_Set->m_State->m_fLFOFreq_LFO2, 1); //lfono 1
			}
		}
		break;
	}
	case 2: {
		if (*m_Set->m_State->m_bLFOSynch_LFO3 == SWITCH::SWITCH_ON) {
			float l_fIntervalTime = m_Set->getIntervalTimeFromDAWBeats(*m_Set->m_State->m_uLFOTimeBeats_LFO3);

			if (*m_Set->m_State->m_bLFOPerVoice_LFO3 == SWITCH::SWITCH_ON) {
				for (int i = 0; i < C_MAX_POLY; i++) {
					m_singleNote[i]->m_LFO_Osc[2]->updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO3, 1, 0, 0, 0);
					m_singleNote[i]->m_LFO_Osc[2]->startLFOFrequency(1000.f / l_fIntervalTime, 2); //lfono 2
				}
			}
			else {
				m_global_LFO_Osc[2].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO3, 1, 0, 0, 0);
				m_global_LFO_Osc[2].startLFOFrequency(1000.f / l_fIntervalTime, 2); //lfono 2
			}
		}
		else {
			if (*m_Set->m_State->m_bLFOPerVoice_LFO3 == SWITCH::SWITCH_ON) {
				for (int i = 0; i < C_MAX_POLY; i++) {
					m_singleNote[i]->m_LFO_Osc[2]->updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO3, 1, 0, 0, 0);
					m_singleNote[i]->m_LFO_Osc[2]->startLFOFrequency(*m_Set->m_State->m_fLFOFreq_LFO3, 2); //lfono 2
				}
			}
			else {
				m_global_LFO_Osc[2].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO3, 1, 0, 0, 0);
				m_global_LFO_Osc[2].startLFOFrequency(*m_Set->m_State->m_fLFOFreq_LFO3, 2); //lfono 2
			}
		}
		break;
	}
	case 3: {
		if (*m_Set->m_State->m_bLFOSynch_LFO4 == SWITCH::SWITCH_ON) {
			float l_fIntervalTime = m_Set->getIntervalTimeFromDAWBeats(*m_Set->m_State->m_uLFOTimeBeats_LFO4);

			if (*m_Set->m_State->m_bLFOPerVoice_LFO4 == SWITCH::SWITCH_ON) {
				for (int i = 0; i < C_MAX_POLY; i++) {
					m_singleNote[i]->m_LFO_Osc[3]->updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO4, 1, 0, 0, 0);
					m_singleNote[i]->m_LFO_Osc[3]->startLFOFrequency(1000.f / l_fIntervalTime, 3); //lfono 3
				}
			}
			else {
				m_global_LFO_Osc[3].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO4, 1, 0, 0, 0);
				m_global_LFO_Osc[3].startLFOFrequency(1000.f / l_fIntervalTime, 2); //lfono 2
			}
		}
		else {
			if (*m_Set->m_State->m_bLFOPerVoice_LFO4 == SWITCH::SWITCH_ON) {
				for (int i = 0; i < C_MAX_POLY; i++) {
					m_singleNote[i]->m_LFO_Osc[3]->updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO4, 1, 0, 0, 0);
					m_singleNote[i]->m_LFO_Osc[3]->startLFOFrequency(*m_Set->m_State->m_fLFOFreq_LFO4, 3); //lfono 2
				}
			}
			else {
				m_global_LFO_Osc[3].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO4, 1, 0, 0, 0);
				m_global_LFO_Osc[3].startLFOFrequency(*m_Set->m_State->m_fLFOFreq_LFO4, 3); //lfono 3
			}
		}
		break;
	}
	case 4: {
		if (*m_Set->m_State->m_bLFOSynch_LFO5 == SWITCH::SWITCH_ON) {
			float l_fIntervalTime = m_Set->getIntervalTimeFromDAWBeats(*m_Set->m_State->m_uLFOTimeBeats_LFO5);

			if (*m_Set->m_State->m_bLFOPerVoice_LFO5 == SWITCH::SWITCH_ON) {
				for (int i = 0; i < C_MAX_POLY; i++) {
					m_singleNote[i]->m_LFO_Osc[4]->updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO5, 1, 0, 0, 0);
					m_singleNote[i]->m_LFO_Osc[4]->startLFOFrequency(1000.f / l_fIntervalTime, 4); //lfono 4
				}
			}
			else {
				m_global_LFO_Osc[4].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO5, 1, 0, 0, 0);
				m_global_LFO_Osc[4].startLFOFrequency(1000.f / l_fIntervalTime, 4); //lfono 4
			}
		}
		else {
			if (*m_Set->m_State->m_bLFOPerVoice_LFO5 == SWITCH::SWITCH_ON) {
				for (int i = 0; i < C_MAX_POLY; i++) {
					m_singleNote[i]->m_LFO_Osc[4]->updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO5, 1, 0, 0, 0);
					m_singleNote[i]->m_LFO_Osc[4]->startLFOFrequency(*m_Set->m_State->m_fLFOFreq_LFO5, 2); //lfono 4
				}
			}
			else {
				m_global_LFO_Osc[4].updateMainVariables(m_Set->m_nSampleRate, *m_Set->m_State->m_uLFOWave_LFO5, 1, 0, 0, 0);
				m_global_LFO_Osc[4].startLFOFrequency(*m_Set->m_State->m_fLFOFreq_LFO5, 4); //lfono 4
			}
		}
		break;
	}
	}
}

//called on noteOn
void CVASTPoly::resynchLFO() {
	#define FREERUN_LFO_RETRIGGER_RESET 1000 //1s

	if (*m_Set->m_State->m_bLFORetrigOnOff_LFO1 == SWITCH::SWITCH_ON) {
		if (*m_Set->m_State->m_bLFOPerVoice_LFO1 == SWITCH::SWITCH_ON) {
			/*
			for (int i = 0; i < C_MAX_POLY; i++) {
				m_singleNote[i]->m_LFO_Osc[0].resynch(true);
			}
			*/
			//done in single note note on now
		}
		else {
			m_global_LFO_Osc[0].resynchWithFade(true);
		}
	}
	else { //freerunning - retrigger when last note older than 1s?
		int voiceNo = getLastNotePlayed(); // make parameter oldest or newest		
		if (voiceNo >= 0) {
			CVASTSingleNote* note = m_singleNote[voiceNo];
			if (note->isKeyDown() == false) {
				struct timeval tp;
				m_Set->_gettimeofday(&tp);
				int diff = (tp.tv_sec * 1000 + tp.tv_usec / 1000) - note->m_startPlayTimestamp;
				if (diff > FREERUN_LFO_RETRIGGER_RESET) {
					m_global_LFO_Osc[0].resynchWithFade(true);
				}
			}
		}
	}

	if (*m_Set->m_State->m_bLFORetrigOnOff_LFO2 == SWITCH::SWITCH_ON) {
		if (*m_Set->m_State->m_bLFOPerVoice_LFO2 == SWITCH::SWITCH_ON) {
			/*
			for (int i = 0; i < C_MAX_POLY; i++) {
				m_singleNote[i]->m_LFO_Osc[1].resynch(true);
			}
			*/
			//done in single note note on now
		}
		else {
			m_global_LFO_Osc[1].resynchWithFade(true);
		}
	}
	else { //freerunning - retrigger when last note older than 1s?
		int voiceNo = getLastNotePlayed(); // make parameter oldest or newest		
		if (voiceNo >= 0) {
			CVASTSingleNote* note = m_singleNote[voiceNo];
			if (note->isKeyDown() == false) {
				struct timeval tp;
				m_Set->_gettimeofday(&tp);
				int diff = (tp.tv_sec * 1000 + tp.tv_usec / 1000) - note->m_startPlayTimestamp;
				if (diff > FREERUN_LFO_RETRIGGER_RESET) {
					m_global_LFO_Osc[1].resynchWithFade(true);
				}
			}
		}
	}

	if (*m_Set->m_State->m_bLFORetrigOnOff_LFO3 == SWITCH::SWITCH_ON) {
		if (*m_Set->m_State->m_bLFOPerVoice_LFO3 == SWITCH::SWITCH_ON) {
			/*
			for (int i = 0; i < C_MAX_POLY; i++) {
				m_singleNote[i]->m_LFO_Osc[2].resynch(true);
			}
			*/
			//done in single note note on now

		}
		else {
			m_global_LFO_Osc[2].resynchWithFade(true);
		}
	}
	else { //freerunning - retrigger when last note older than 1s?
		int voiceNo = getLastNotePlayed(); // make parameter oldest or newest		
		if (voiceNo >= 0) {
			CVASTSingleNote* note = m_singleNote[voiceNo];
			if (note->isKeyDown() == false) {
				struct timeval tp;
				m_Set->_gettimeofday(&tp);
				int diff = (tp.tv_sec * 1000 + tp.tv_usec / 1000) - note->m_startPlayTimestamp;
				if (diff > FREERUN_LFO_RETRIGGER_RESET) {
					m_global_LFO_Osc[2].resynchWithFade(true);
				}
			}
		}
	}

	if (*m_Set->m_State->m_bLFORetrigOnOff_LFO4 == SWITCH::SWITCH_ON) {
		if (*m_Set->m_State->m_bLFOPerVoice_LFO4 == SWITCH::SWITCH_ON) {
			/*
			for (int i = 0; i < C_MAX_POLY; i++) {
			m_singleNote[i]->m_LFO_Osc[3].resynch(true);
			}
			*/
			//done in single note note on now

		}
		else {
			m_global_LFO_Osc[3].resynchWithFade(true);
		}
	}
	else { //freerunning - retrigger when last note older than 1s?
		int voiceNo = getLastNotePlayed(); // make parameter oldest or newest		
		CVASTSingleNote* note = m_singleNote[voiceNo];
		if (voiceNo >= 0) {
			if (note->isKeyDown() == false) {
				struct timeval tp;
				m_Set->_gettimeofday(&tp);
				int diff = (tp.tv_sec * 1000 + tp.tv_usec / 1000) - note->m_startPlayTimestamp;
				if (diff > FREERUN_LFO_RETRIGGER_RESET) {
					m_global_LFO_Osc[3].resynchWithFade(true);
				}
			}
		}
	}

	if (*m_Set->m_State->m_bLFORetrigOnOff_LFO5 == SWITCH::SWITCH_ON) {
		if (*m_Set->m_State->m_bLFOPerVoice_LFO5 == SWITCH::SWITCH_ON) {
			/*
			for (int i = 0; i < C_MAX_POLY; i++) {
			m_singleNote[i]->m_LFO_Osc[4].resynch(true);
			}
			*/
			//done in single note note on now

		}
		else {
			m_global_LFO_Osc[4].resynchWithFade(true);
		}
	}
	else { //freerunning - retrigger when last note older than 1s?
		int voiceNo = getLastNotePlayed(); // make parameter oldest or newest		
		CVASTSingleNote* note = m_singleNote[voiceNo];
		if (voiceNo >= 0) {
			if (note->isKeyDown() == false) {
				struct timeval tp;
				m_Set->_gettimeofday(&tp);
				int diff = (tp.tv_sec * 1000 + tp.tv_usec / 1000) - note->m_startPlayTimestamp;
				if (diff > FREERUN_LFO_RETRIGGER_RESET) {
					m_global_LFO_Osc[4].resynchWithFade(true);
				}
			}
		}
	}
}

void CVASTPoly::doArp(sRoutingBuffers& routingBuffers, MidiBuffer& midiMessages) {
	VASTARPData* arpData = &m_Set->m_ARPData;
	//const ScopedReadLock myScopedLock(arpData->mReadWriteLock); //this seemed to cause an issue
	int numSteps = arpData->getNumSteps();
	auto numSamples = routingBuffers.getNumSamples();

	arpData->setDispActiveStep(((m_ARP_currentStep + numSteps) - 1) % numSteps); // always one ahead - current step means waiting for the step

																				 //reset?
	bool bStart = false;

	if (!m_arpHasActiveStepToFinish) {
		if (m_ARP_midiInNotes.size() == 0) {
			m_ARP_currentStep = 0;
			arpData->setDispActiveStep(0);
			bStart = true;
		}
	}
	if (m_ARP_currentStep > (numSteps - 1))
		m_ARP_currentStep = 0;

	// get step duration
	double stepDuration = 0;
	int l_ARP_currentStep = 0;
	//if (*m_Set->m_State->m_bARPSynch == SWITCH::SWITCH_ON) {
	//if ((*m_Set->m_State->m_bARPSynch == SWITCH::SWITCH_ON) && (m_Set->m_bPpqIsPlaying)) {
	if ((*m_Set->m_State->m_bARPSynch == SWITCH::SWITCH_ON) && (m_Set->m_bPpqIsPlaying)) {
		//if (m_Set->m_bPpqIsPlaying == false)
			//return; //stop all??
		if (m_last_bpm != m_Set->m_dPpqBpm) { //bpm was changed??
			bStart = true;
		}
		m_last_bpm = m_Set->m_dPpqBpm; 

		double l_fIntervalTime = m_Set->getIntervalTimeFromDAWBeats(*m_Set->m_State->m_uARPTimeBeats);
		//stepDuration = static_cast<int> (std::ceil(m_Set->m_nSampleRate * (l_fIntervalTime / 1000.f)));  //interval time is in seconds
		stepDuration = m_Set->m_nSampleRate * (l_fIntervalTime * 0.001);  //interval time is in seconds

		double realPos = m_Set->m_dPpqPosition / m_Set->getIntervalRatio(*m_Set->m_State->m_uARPTimeBeats);
		double realPosEndOfBuffer = realPos + numSamples / stepDuration;
		l_ARP_currentStep = int(realPosEndOfBuffer) % numSteps;
		if (bStart) m_ARP_currentStep = l_ARP_currentStep;
		l_ARP_currentStep++;
		l_ARP_currentStep %= numSteps;

		double l_ARP_time = (realPos - int(realPos)) * stepDuration;
		double diff = l_ARP_time - m_ARP_time; //find out difference
		if (diff < -0.001f) 
			diff = stepDuration + l_ARP_time - m_ARP_time;
		m_ARP_time += diff; //can be larger than stepDuration!
		//while (m_ARP_time > stepDuration) m_ARP_time -= stepDuration; //needed? safety
	}
	else {
		if (*m_Set->m_State->m_bARPSynch == SWITCH::SWITCH_ON) { //synch but not playing
			double l_fIntervalTime = m_Set->getIntervalTimeFromDAWBeats(*m_Set->m_State->m_uARPTimeBeats);
			m_fARP_Speed_smoothed.setValue(l_fIntervalTime, true);
		}

		double speed = m_fARP_Speed_smoothed.getNextValue();
		if (speed == 0.0) {
			m_fARP_Speed_smoothed.setValue(*m_Set->m_State->m_fARPSpeed, true);
			speed = m_fARP_Speed_smoothed.getNextValue();
		}
		//stepDuration = static_cast<int> (std::ceil(m_Set->m_nSampleRate * (speed / 1000.f)));
		stepDuration = m_Set->m_nSampleRate * (speed * 0.001);
		if (bStart) m_ARP_time = stepDuration;
	}

	int lChannelForARP = 1;
	if (getSynthesizer()->m_MPEMasterChannel == lChannelForARP)
		lChannelForARP++;

	MidiMessage msg;
	int samplePos;

	MidiBuffer mb;
	mb.clear();
	for (MidiBuffer::Iterator it(midiMessages); it.getNextEvent(msg, samplePos);)
	{
		if (msg.isNoteOn()) {
			m_ARP_midiInNotes.add(msg.getNoteNumber());
		}
		else if (msg.isNoteOff()) {
			if (!m_ARP_midiInNotes.contains(msg.getNoteNumber())) //some leftovers
				mb.addEvent(msg, 0);
			m_ARP_midiInNotes.removeValue(msg.getNoteNumber());
		}
		else mb.addEvent(msg, samplePos);
	}

	midiMessages.clear();
	midiMessages.addEvents(mb, 0, numSamples, 0);

	//noteOff shall be send when time passes behind length of last note - and it is not hold
	bool isHold = false;
	int stepBefore = (m_ARP_currentStep > 0) ? m_ARP_currentStep - 1 : numSteps - 1;
	VASTARPData::ArpStep curStepData = arpData->getStep(m_ARP_currentStep);
	VASTARPData::ArpStep stepBeforeData = arpData->getStep(stepBefore);
	if ((m_ARP_time + numSamples) >= (stepBeforeData.gate * 0.25f) * stepDuration)
	{
		m_arpHasActiveStepToFinish = false;
		if ((stepBeforeData.gate == 4) &&
			(curStepData.semitones == stepBeforeData.semitones) &&
			(curStepData.octave == stepBeforeData.octave)) // this is hold
			isHold = true;
		if (curStepData.gate == 0) isHold = false; //no hold when current note is 0
		if (stepBefore == m_ARP_currentStep) isHold = false; //only one step and hold?
		bool allHold = true;
		for (int i = 0; i < arpData->getNumSteps(); i++) {
			VASTARPData::ArpStep thisStepData = arpData->getStep(i);
			if (thisStepData.gate != 4) allHold = false;
		}
		if (allHold && (stepBefore == (arpData->getNumSteps() - 1))) isHold = false; //no hold on last note when all is hold
		if (bStart) isHold = false; //when nothing is pressed

		if (!isHold) { //end all notes playing
			for (int note = 0; note < m_ARP_currentARPNoteValues.size(); note++) {
				if (m_ARP_currentARPNoteValues[note] > 0)
				{
					auto offset = jmax(0, jmin((int)((stepBeforeData.gate * 0.25f) * stepDuration - m_ARP_time), numSamples - 1));
					midiMessages.addEvent(MidiMessage::noteOff(lChannelForARP, m_ARP_currentARPNoteValues[note]), offset);
				}
			}
			m_ARP_currentARPNoteValues.clear();
		}
	}

	bool bStartNote = false;
	if (((m_ARP_time + numSamples) >= stepDuration) || bStart)
		bStartNote = true;

	if (bStartNote)
	{
		if ((m_ARP_midiInNotes.size() > 0) && (!isHold))
		{
			m_arpHasActiveStepToFinish = true;
			auto offset = jmax(0, jmin((int)(stepDuration - m_ARP_time), numSamples - 1));
			//collect what needs to be triggered
			m_ARP_currentARPNoteValues.clear();
			if (*m_Set->m_State->m_uARPMode == ARPMODE::POLY) {
				for (int note = 0; note < m_ARP_midiInNotes.size(); note++) {
					m_ARP_currentARPNoteValues.add(m_ARP_midiInNotes[note]);
				}
			}
			else if (*m_Set->m_State->m_uARPMode == ARPMODE::UP) {
				m_ARP_direction = +1;

				if (m_ARP_currentStep == 0) {
					m_ARP_currentNote = (m_ARP_midiInNotes.size() + m_ARP_currentNote + m_ARP_direction) % m_ARP_midiInNotes.size();
				}
				else {
					if (m_ARP_currentNote >= m_ARP_midiInNotes.size())
						m_ARP_currentNote--;
				}
				m_ARP_currentARPNoteValues.add(m_ARP_midiInNotes[m_ARP_currentNote]);
			}
			else if (*m_Set->m_State->m_uARPMode == ARPMODE::DOWN) {
				m_ARP_direction = -1;
				if (m_ARP_currentStep == 0) {
					m_ARP_currentNote = (m_ARP_midiInNotes.size() + m_ARP_currentNote + m_ARP_direction) % m_ARP_midiInNotes.size();
				}
				else {
					if (m_ARP_currentNote >= m_ARP_midiInNotes.size())
						m_ARP_currentNote--;
				}
				m_ARP_currentARPNoteValues.add(m_ARP_midiInNotes[m_ARP_currentNote]);
			}
			else if (*m_Set->m_State->m_uARPMode == ARPMODE::UPDOWN) {
				if (m_ARP_currentStep == 0) {
					m_ARP_currentNote = (m_ARP_midiInNotes.size() + m_ARP_currentNote + m_ARP_direction) % m_ARP_midiInNotes.size();
					if ((m_ARP_direction == 1) && (m_ARP_currentNote >= m_ARP_midiInNotes.size() - 1))
						m_ARP_direction *= -1;
					if ((m_ARP_direction == -1) && (m_ARP_currentNote <= 0))
						m_ARP_direction *= -1;

				}
				else {
					if (m_ARP_currentNote >= m_ARP_midiInNotes.size())
						m_ARP_currentNote--;
				}
				m_ARP_currentARPNoteValues.add(m_ARP_midiInNotes[m_ARP_currentNote]);
			}

			//trigger them all
			for (int note = 0; note < m_ARP_currentARPNoteValues.size(); note++) {
				VASTARPData::ArpStep curStepData = arpData->getStep(m_ARP_currentStep);
				float curVelocity = curStepData.velocity;
				int newNote = m_ARP_currentARPNoteValues[note] + curStepData.octave * 12 + curStepData.semitones;

				if ((curStepData.gate > 0) && (newNote > 0) && (newNote < 127)) {
					midiMessages.addEvent(MidiMessage::noteOn(lChannelForARP, newNote, (uint8)curVelocity), offset);
					m_ARP_currentARPNoteValues.getReference(note) = newNote;
				}
				else {
					m_ARP_currentARPNoteValues.getReference(note) = -1;
				}
			}
		}

		//if (*m_Set->m_State->m_bARPSynch == SWITCH::SWITCH_OFF) {
		if ((*m_Set->m_State->m_bARPSynch == SWITCH::SWITCH_OFF) || (!m_Set->m_bPpqIsPlaying)) {
			m_ARP_currentStep++;
			m_ARP_currentStep %= numSteps;
		}
		else
			m_ARP_currentStep = l_ARP_currentStep;
	}

	m_ARP_time = (m_ARP_time + numSamples);
	//while (m_ARP_time > stepDuration) 
	while (m_ARP_time >= stepDuration) 
		m_ARP_time -= stepDuration;
}

//===========================================================================================

void CVASTPoly::processAudioBuffer(sRoutingBuffers& routingBuffers, MidiBuffer& midiMessages) {
	
	m_iLastSingleNoteCycleCalls = 0; //for single not softfade cycle 
	//===========================================================================================
	//Check for StepSeq restart
	if (m_ppq_playing != m_Set->m_bPpqIsPlaying) {	//status changed
		if (m_Set->m_bPpqIsPlaying) { //now just started
			if (*m_Set->m_State->m_bARPSynch == SWITCH::SWITCH_ON)
				initArp();

			struct timeval tp;
			m_Set->_gettimeofday(&tp);
			if (*m_Set->m_State->m_bStepSeqSynch_STEPSEQ1 == SWITCH::SWITCH_ON) {
				m_StepSeq_Envelope[0].noteOff();
			}
			if (*m_Set->m_State->m_bStepSeqSynch_STEPSEQ2 == SWITCH::SWITCH_ON) {
				m_StepSeq_Envelope[1].noteOff();
			}
			if (*m_Set->m_State->m_bStepSeqSynch_STEPSEQ3 == SWITCH::SWITCH_ON) {
				m_StepSeq_Envelope[2].noteOff();
			}

			//struct timeval tp;
			//m_Set->_gettimeofday(&tp);
			if (*m_Set->m_State->m_bStepSeqSynch_STEPSEQ1 == SWITCH::SWITCH_ON) {
				m_StepSeq_Envelope[0].noteOn(tp.tv_sec * 1000 + tp.tv_usec / 1000, false);
			}
			if (*m_Set->m_State->m_bStepSeqSynch_STEPSEQ2 == SWITCH::SWITCH_ON) {
				m_StepSeq_Envelope[1].noteOn(tp.tv_sec * 1000 + tp.tv_usec / 1000, false);
			}
			if (*m_Set->m_State->m_bStepSeqSynch_STEPSEQ3 == SWITCH::SWITCH_ON) {
				m_StepSeq_Envelope[2].noteOn(tp.tv_sec * 1000 + tp.tv_usec / 1000, false);
			}

			if (*m_Set->m_State->m_bStepSeqSynch_STEPSEQ1 == SWITCH::SWITCH_ON) {
				if (m_Set->m_dPpqBpm != 0.f) {
					float l_fIntervalTime = m_Set->getIntervalTimeFromDAWBeats(*m_Set->m_State->m_uStepSeqTimeBeats_STEPSEQ1);
					m_Set->m_StepSeqData[0].setStepSeqTime(l_fIntervalTime); //is in ms
				}
			}
			if (*m_Set->m_State->m_bStepSeqSynch_STEPSEQ2 == SWITCH::SWITCH_ON) {
				if (m_Set->m_dPpqBpm != 0.f) {
					float l_fIntervalTime = m_Set->getIntervalTimeFromDAWBeats(*m_Set->m_State->m_uStepSeqTimeBeats_STEPSEQ2);
					m_Set->m_StepSeqData[1].setStepSeqTime(l_fIntervalTime); //is in ms
				}
			}
			if (*m_Set->m_State->m_bStepSeqSynch_STEPSEQ3 == SWITCH::SWITCH_ON) {
				if (m_Set->m_dPpqBpm != 0.f) {
					float l_fIntervalTime = m_Set->getIntervalTimeFromDAWBeats(*m_Set->m_State->m_uStepSeqTimeBeats_STEPSEQ3);
					m_Set->m_StepSeqData[2].setStepSeqTime(l_fIntervalTime); //is in ms
				}
			}

		}
		else { //now just stopped			
			if (*m_Set->m_State->m_bARPSynch == SWITCH::SWITCH_ON)
				initArp();
			
			//do nothing
			/*
			if (*m_Set->m_State->m_bStepSeqSynch_STEPSEQ1 == SWITCH::SWITCH_ON)
				m_Set->m_StepSeqData[0].setStepSeqTime(*m_Set->m_State->m_fStepSeqSpeed_STEPSEQ1); //is in ms
			if (*m_Set->m_State->m_bStepSeqSynch_STEPSEQ2 == SWITCH::SWITCH_ON)
				m_Set->m_StepSeqData[1].setStepSeqTime(*m_Set->m_State->m_fStepSeqSpeed_STEPSEQ2); //is in ms
			if (*m_Set->m_State->m_bStepSeqSynch_STEPSEQ3 == SWITCH::SWITCH_ON)
				m_Set->m_StepSeqData[2].setStepSeqTime(*m_Set->m_State->m_fStepSeqSpeed_STEPSEQ3); //is in ms
			*/

		}
		m_ppq_playing = !m_ppq_playing;
	}

	//===========================================================================================
	//Check ARP
	if (m_shallInitARP) initArpInternal(midiMessages);
	if (*m_Set->m_State->m_bARPOnOff == SWITCH::SWITCH_ON) {
		doArp(routingBuffers, midiMessages);
	}
	else {
		m_Set->m_ARPData.setDispActiveStep(-1);
		MidiMessage msg;
		int samplePos;
		for (MidiBuffer::Iterator it(midiMessages); it.getNextEvent(msg, samplePos);)
		{
			if (msg.isNoteOn()) {
				m_ARP_currentARPNoteValues.add(msg.getNoteNumber());
			}
		}
	}

	//===========================================================================================
	//oscillator voices

	m_OscillatorSynthesizer.renderNextBlock(routingBuffers, midiMessages, 0, routingBuffers.getNumSamples());

	//ARP speed mod
	modMatrixInputState inputState = getOldestNotePlayedInputState(0); // make parameter oldest or newest
	m_fARP_Speed_smoothed.setValue(m_Set->getParameterValueWithMatrixModulation(m_Set->m_State->m_fARPSpeed, MODMATDEST::ArpSpeed, &inputState), false);

	//===========================================================================================
	// attenuate
	//float lGain = 0.1875f; // float(1.0 / C_MAX_POLY) * 3.0f; //attenuate  poly notes by maximum number to have the same gain for all
	float lGain = 0.27f; // float(1.0 / C_MAX_POLY) * 3.0f; //attenuate  poly notes by maximum number to have the same gain for all
	routingBuffers.OscBuffer[0]->applyGain(lGain);
	routingBuffers.OscBuffer[1]->applyGain(lGain);
	routingBuffers.OscBuffer[2]->applyGain(lGain);
	routingBuffers.OscBuffer[3]->applyGain(lGain);	
	routingBuffers.SamplerBuffer->applyGain(lGain);
	routingBuffers.FilterBuffer[0]->applyGain(lGain);
	routingBuffers.FilterBuffer[1]->applyGain(lGain);
	routingBuffers.FilterBuffer[2]->applyGain(lGain);
	routingBuffers.NoiseBuffer->applyGain(lGain);
}
