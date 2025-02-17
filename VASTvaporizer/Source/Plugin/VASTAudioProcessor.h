/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef VASTAUDIOPROCESSOR_H_INCLUDED
#define VASTAUDIOPROCESSOR_H_INCLUDED

#include "../Engine/VASTEngineHeader.h"
#include "../Engine/VASTXperience.h"
#if defined(VASTCOMMERCIAL) || defined(VASTBUILD)
	#include "VASTCommercial/VASTLicense.h"
#endif
#include "VASTPreset/VASTPresetElement.h"
#include "VASTPreset/VASTPresetData.h"
#include "VASTVUMeterSource.h"
#include <string.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <unordered_map>
#include "VASTUtils/VASTLookAndFeel.h"

#define MAX_LEN 63
#define MAX_TRIAL_SECONDS 1200
#define CONST_USER_PATCH_INDEX 9999

#ifdef JUCE_WINDOWS
	#ifdef _WIN64	
		#define CONST_DLL_NAME "VASTvaporizer64.dll"
	#else
		#define CONST_DLL_NAME "VASTvaporizer.dll"
	#endif

#else
	#ifdef JUCE_MAC	
		#define CONST_DLL_NAME "VASTvaporizer.component"
	#endif
#endif

class VASTParameterSlider; //forward declaration

//==============================================================================
/**
*/
class VASTAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    VASTAudioProcessor();
    ~VASTAudioProcessor();

	enum VstStringConstants
	{
		//-------------------------------------------------------------------------------------------------------
		kMaxProgNameLen = 24,	///< used for #effGetProgramName, #effSetProgramName, #effGetProgramNameIndexed
		kMaxParamStrLen = 8,	///< used for #effGetParamLabel, #effGetParamDisplay, #effGetParamName
		kMaxVendorStrLen = 64,	///< used for #effGetVendorString, #audioMasterGetVendorString
		kMaxProductStrLen = 64,	///< used for #effGetProductString, #audioMasterGetProductString
		kMaxEffectNameLen = 32	///< used for #effGetEffectName
		//-------------------------------------------------------------------------------------------------------
	};

	typedef struct {
		float rangeStart;
		float rangeEnd;
		float rangeSkew;
		StringRef paramID;
		AudioProcessorParameterWithID* param;
	} sModMatrixLookup;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;
	void processBlockBypassed(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override;

    //==============================================================================
	AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
	bool initNotCompleted() { return !m_initCompleted; };
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

	bool isInErrorState() { return bIsInErrorState; };
	void setErrorState(int state) { 
		bIsInErrorState = true; 
		iErrorState = state; 
	};
	int getErrorState() { return iErrorState; };
	bool wantsUIAlert() { return mUIAlert; };
	void clearUIAlertFlag() { mUIAlert = false; };
	void requestUIAlert() { mUIAlert = true; };
	void requestUIPresetUpdate() {
		mUIUpdateFlag = true;
		mUIPresetUpdate = true; };
	bool needsUIPresetUpdate() { return mUIPresetUpdate; };
	void clearUIPresetFlag() { mUIPresetUpdate = false; };

	void requestUIPresetReloadUpdate() {
		mUIUpdateFlag = true;
		mUIPresetReloadUpdate = true;
	};
	bool needsUIPresetReloadUpdate() { return mUIPresetReloadUpdate; };
	void clearUIPresetReloadFlag() { mUIPresetReloadUpdate = false; };


	bool needsUIInit() { return mUIInitFlag; };
	void clearUIInitFlag() { mUIInitFlag = false; };
	void requestUIInit() { mUIInitFlag = true; requestUIUpdate(true, true, true); };

	bool needsUIUpdate(){ return mUIUpdateFlag; };
	bool needsUIUpdate_tabs() { return mUIUpdateFlag_tabs; };
	bool needsUIUpdate_matrix() { return mUIUpdateFlag_matrix; };
	bool needsUIUpdate_sliders() { return mUIUpdateFlag_sliders; };
	int needsUIUpdate_slider1dest() { return mUIUpdateFlag_slider1dest; };
	int needsUIUpdate_slider2dest() { return mUIUpdateFlag_slider2dest; };
	void clearUIUpdateFlag() { mUIUpdateFlag = false; 
		mUIUpdateFlag_tabs = false;
		mUIUpdateFlag_matrix = false;
		mUIUpdateFlag_sliders = false;
		mUIUpdateFlag_slider1dest = -1;
		mUIUpdateFlag_slider2dest = -1;
	}; 
	void requestUIUpdate(bool tabs = true, bool matrix = true, bool sliders = true, int slider1dest = -1, int slider2dest = -1 ) {
		mUIUpdateFlag = true; 
		mUIUpdateFlag_tabs = tabs;
		mUIUpdateFlag_matrix = matrix;
		mUIUpdateFlag_sliders = sliders;
		mUIUpdateFlag_slider1dest = slider1dest;
		mUIUpdateFlag_slider2dest = slider2dest;	
	};
	void requestUILoadAlert() { mUIAlert = true; };

    //==============================================================================
    const String getName() const override;
  
	void setParameterText(StringRef parName, StringRef textVal, bool bSilent);
	AudioProcessorValueTreeState& getParameterTree() { 
		return m_parameterState;  
	};

	VASTVUMeterSource* getMeterSource() {
		return &m_meterSource;
	};

	inline char* _strncpy(char* dst, const char* src, size_t maxLen)
	{
		char* result = strncpy(dst, src, maxLen);
		dst[maxLen] = 0;
		return result;
	}

    /*
	const String getInputChannelName (int channelIndex) const override;
    const String getOutputChannelName (int channelIndex) const override;
    bool isInputChannelStereoPair (int index) const override;
    bool isOutputChannelStereoPair (int index) const override;
	*/

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool silenceInProducesSilenceOut() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;

	int getNumFactoryPresets();
	int getNumUserPresets();

    int getCurrentProgram() override;
	int getCurrentPresetProgram();
	void setCurrentProgram (int index) override;
	void initializeToDefaults();
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override; //bank
	void getCurrentProgramStateInformation(juce::MemoryBlock &destData) override; //preset
    void setStateInformation (const void* data, int sizeInBytes) override; //bank
	void setCurrentProgramStateInformation(const void *data, int sizeInBytes) override;
	
	//void atomicPluginStateUpdate();
	void addChunkTreeState(ValueTree* treeState);

	bool isLicensed();
	String getLicenseText();
	inline bool isUserPatch() { return !m_presetData.getCurPatchData().isFactory; }
	//inline void setIsUserPatch() { m_bIsUserPatch = true; }
	inline String getUserPatchName() { return m_presetData.getCurPatchData().presetname; }
	void savePatchXML(File *selectedFile);
	bool loadPatchXML(XmlDocument* xmlDoc, bool bNameOnly, const VASTPresetElement* preset, int index, VASTPresetElement& resultPresetData);
	static String getVSTPath();
	static String getVSTPathAlternative();
	void loadPreset(int index);	
	bool loadUserPatchMetaData(File file, VASTPresetElement& lPreset);	
	void randomizePatch();

	String getVersionString();

	//static void crashHandler(void* signum) {
	//static void crashHandler(void * exceptionPointers) {
	static void crashHandler(void*);

	void loadDefaultMidiMapping();
	void loadZeroMidiMapping();
	void midiLearnParameter(int internalIndex, String compID);
	void midiForgetParameter(String componentName);
	int midiMappingGetParameterIndex(int iCC);
	int parameterIndexGetMidiMapping(int internalIndex);
	String midiMappingComponentVariableName(int iCC);
	int m_iNowLearningMidiParameter = -1;
	String m_iNowLearningMidiParameterVariableName = "";
	bool writeSettingsToFile();
	void writeSettingsToFileAsync();

	void addModMatrixLookupTable(int modMatrixDestination, float rangeStart, float rangeEnd, float rangeSkew, StringRef paramID, AudioProcessorParameterWithID* param);
	sModMatrixLookup m_modMatrixLookupTable[M_MODMATRIX_MAX_DESTINATIONS];
	std::unordered_multimap<int, String> m_mapModdestToParameterName; //fast: hashed //declare before vastxperience
	std::unordered_map<String, int> m_mapParameterNameToModdest; //declare before vastxperience	
	Array<VASTParameterSlider*> m_mapParameterNameToControl; //declare before vastxperience

	UndoManager m_undoManager; //declare before parameterState
	AudioProcessorValueTreeState m_parameterState; //declare before vastxperience
	
	//==============================================================================
	CVASTXperience m_pVASTXperience;
	//==============================================================================

	bool nonThreadsafeIsBlockedProcessingInfo() {
		return m_pVASTXperience.nonThreadsafeIsBlockedProcessingInfo();
	}

	VASTPresetData m_presetData{ this };
	VASTPresetElement getCurPatchData() {
		m_presetData.getCurPatchData();
	};

	int m_curPatchDataLoadRequestedIndex = 0;

	//--------------------------------------------------------------------------------------------------------------------
	//from settings files
	String m_UserPresetRootFolder; // root folder for user presets - read from config file, set in preset component
	String m_UserWavetableRootFolder; // root folder for user presets - read from config file, set in preset component
	String m_UserWavRootFolder; // root folder for user presets - read from config file, set in preset component
	bool m_disableOpenGLGFX = false; // settings

	int m_iUserTargetPluginWidth = 0;
	int m_iUserTargetPluginHeight = 0;
	
	float getPluginScaleWidthFactor() {
		if (m_iDefaultPluginWidth != 0)
			return m_iUserTargetPluginWidth / float(m_iDefaultPluginWidth);
		return 1.f;
	}
	float getPluginScaleHeightFactor() {
		if (m_iDefaultPluginHeight != 0)
			return m_iUserTargetPluginHeight / float(m_iDefaultPluginHeight);
		return 1.f;
	}

	void togglePerspectiveDisplay(int lOscillatorBank) {
		m_bTogglePerspectiveDisplay[lOscillatorBank] = !m_bTogglePerspectiveDisplay[lOscillatorBank];
		writeSettingsToFileAsync();
	}
	bool m_bTogglePerspectiveDisplay[4] = { false, false, false, false }; //per oscbank

	void setWTmode(int wtMode) {
		if (wtMode != m_pVASTXperience.m_Set.m_WTmode) {
			m_pVASTXperience.m_Set.m_WTmode = wtMode;
			//recalc WT
			for (int bank = 0; bank < 4; bank++)
				m_pVASTXperience.m_Poly.m_OscBank[bank]->recalcWavetable();
		}
	}
	int getWTmode() {
		return m_pVASTXperience.m_Set.m_WTmode;
	}

	int getMPEmode() {
		return m_MPEmode;
	}
	void setMPEmode(int mode) {
		m_MPEmode = jlimit<int>(0, 2, mode);
	}
	bool isMPEenabled() {
		return (m_MPEmode == 1) || ((m_MPEmode == 0) && (m_presetData.getCurPatchData().mpepreset));
	}

	int getUIFontSize() {
		return m_uiFontSize;
	}
	void setUIFontSize(int size) {
		m_uiFontSize = size;
		for (int i = 0; i < vastLookAndFeels.size(); i++)
			vastLookAndFeels[i]->setUIFontSize(size);
		requestUIInit();
	}
	void setBendRange(int bendRange) {
		m_pVASTXperience.m_Set.m_iBendRange = bendRange;
		requestUIUpdate(true, false, false);
	}
	int getBendRange() {
		return m_pVASTXperience.m_Set.m_iBendRange;
	}

	int m_iWTEditorDrawMode = OscillatorEditMode::SelectMode;
	int m_iWTEditorGridMode = OscillatorGridMode::NoGrid;
	int m_iWTEditorBinMode = BinMode::ClipBin;
	int m_iWTEditorBinEditMode = FreqEditMode::SingleBin;
	int getDrawMode() { return m_iWTEditorDrawMode; };
	int getGridMode() { return m_iWTEditorGridMode; };
	int getBinMode() { return m_iWTEditorBinMode; };
	int getBinEditMode() { return m_iWTEditorBinEditMode; };	   

	//--------------------------------------------------------------------------------------------------------------------

	const int m_iDefaultPluginWidth = 1420; //default size from projucer
	const int m_iDefaultPluginHeight = 820; //default size from projucer
	const double m_dPluginRatio = double(m_iDefaultPluginWidth) / double(m_iDefaultPluginHeight);

	int autoParamGetDestination(String parametername);
	String autoDestinationGetParam(int modmatdest);
	
	static void threadedPingCheck(Component::SafePointer<Label> safePointerLabel, VASTAudioProcessor* processor);

	static void passTreeToAudioThread(ValueTree tree, bool externalRepresentation, VASTPresetElement preset, int index, VASTAudioProcessor* processor, bool isSeparateThread, bool initOnly);
	std::atomic<bool> m_bAudioThreadRunning = false;
	std::atomic<bool> m_bCreateCachedVASTEditorDelayed = false;

	void registerThread();
	void unregisterThread();
	bool readSettingsFromFile();
	int m_loadedPresetIndexCount = 0;
	int m_activeLookAndFeel = 3; //new default: dark
	int m_uiFontSize = 0; //default 100%
	void initLookAndFeels();
	OwnedArray<VASTLookAndFeel> vastLookAndFeels; 
	VASTLookAndFeel* getCurrentVASTLookAndFeel() {
		VASTLookAndFeel* lf = vastLookAndFeels[m_activeLookAndFeel];
		vassert(lf != nullptr);
		return lf;
	};

	void dumpBuffers();
	void dumpBuffersFlush();
	bool isDumping();

	bool doUndo();

	//license information from regfile
	struct {
		String mProduct_id;
		String mUserID;
		String mActivationString;
		String mReg_name;
		String mEmail;
		String mPurchase_id;
		String mPurchase_date;
		String mLegacyLicenseString;
		bool m_bIsLicensed = false;
		bool m_bExpired = false;
		bool m_bIsLegacyLicense = true;
		bool m_bIsFreeVersion = false;
	} m_sLicenseInformation;

	String getLocalMachineID();
	String getEncodedIDString(const String& input);
	char getPlatformPrefix();

	void createCachedVASTEditor();
	void createCachedVASTEditorDelayed();

	bool m_showNewerVersionPopup = false;
	String m_newerVersionThatIsAvailble = "No newer version";
	bool readLicense();

	const String shiftHexEncryptString(const String& str);

#if defined(VASTCOMMERCIAL) || defined(VASTBUILD)
	VASTLicense mLicense;
#endif

private:
    //==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VASTAudioProcessor)

	//bool writeLicense(const String &name, const String &email, const String &cuno, const String &purdat);	
	juce::uint32 m_tSetChunkCalled = 0;

	std::string  XOREncrypt(std::string  a_sValue, std::string  a_sKey);
	std::string  XORDecrypt(std::string  a_sValue, std::string  a_sKey);
	void convertToASCIIhex(std::string  & output, std::string  & input);
	//void convertHexToBinary(std::string  & output, std::string  & input);
	void convertASCIIhexToString(std::string  & output, std::string  & input);
	//void convertBinaryToHex(std::string  & output, std::string  & input);
  
	void checkForNewerVersion(String resultString);

	String FloatArrayToString(float* fData, int numFloat)
	{//Return String of multiple float values separated by commas 
		String result = "";
		if (numFloat<1)
			return result;
		for (int i = 0; i<(numFloat - 1); i++)
			result << String(fData[i]) << ",";//Use juce::String initializer for each value
		result << String(fData[numFloat - 1]);
		return result;
	}
	int StringToFloatArray(String sFloatCSV, float* fData, int maxNumFloat)
	{//Return is number of floats copied to the fData array
		//-1 if there were more in the string than maxNumFloat
		StringArray Tokenizer;
		int TokenCount = Tokenizer.addTokens(sFloatCSV, ",", "");
		int resultCount = (maxNumFloat <= TokenCount) ? maxNumFloat : TokenCount;
		for (int i = 0; i<resultCount; i++)//only go as far as resultCount for valid data
			fData[i] = Tokenizer[i].getFloatValue();//fill data using String class float conversion
		return ((TokenCount <= maxNumFloat) ? resultCount : -1);
	}
	String StringArrayToString(String* sData, int numFloat)
	{//Return String of multiple float values separated by commas 
		String result = "";
		if (numFloat<1)
			return result;
		for (int i = 0; i<(numFloat - 1); i++)
			result << String(sData[i]) << ",";//Use juce::String initializer for each value
		result << String(sData[numFloat - 1]);
		return result;
	}
	int StringToStringArray(String sStringCSV, String* sData, int maxNumFloat)
	{//Return is number of floats copied to the fData array
		//-1 if there were more in the string than maxNumFloat
		StringArray Tokenizer;
		int TokenCount = Tokenizer.addTokens(sStringCSV, ",", "");
		int resultCount = (maxNumFloat <= TokenCount) ? maxNumFloat : TokenCount;
		for (int i = 0; i<resultCount; i++)//only go as far as resultCount for valid data
			sData[i] = Tokenizer[i];//fill data using String class 
		return ((TokenCount <= maxNumFloat) ? resultCount : -1);
	}
	bool m_initCompleted = false;

	XmlElement createPatchXML(bool externalRepresentation);
	void xmlParseToPatch(XmlElement *pRoot, bool bNameOnly, const VASTPresetElement* lPreset, int index, bool externalRepresentation, bool isFromState, VASTPresetElement& resultPresetData);

	bool mUIUpdateFlag = true;
	bool mUIUpdateFlag_tabs = true;
	bool mUIUpdateFlag_matrix = true;
	bool mUIUpdateFlag_sliders = true;
	int mUIUpdateFlag_slider1dest = -1;
	int mUIUpdateFlag_slider2dest = -1;
	bool mUIPresetUpdate = false;
	bool mUIPresetReloadUpdate = false;

	int m_midiBank = 0;

	int m_MPEmode = 0; // settings

	bool m_wasBypassed = false;
	bool bIsInErrorState = false;
	int iErrorState = 0;

	bool mUIInitFlag;
	bool mUIAlert;
	//bool mUIPresetUpdateFlag;

	int mIntPpq = 0;
	String m_sLicenseString = " n/a ";
	float m_fTrialSeconds = 0.0;
	String m_vstParams[500];  // number of vaporizer parameters must not exceed 500!
	//int mapExternalInternalParameterIndex(int vstparamindex);

	void initSettings();

	typedef struct {
		bool isParam;
		int paramID;
		String componentVariableName;
	} sMidiMap;
	sMidiMap m_MidiMapping[128]; // there are only 128 MIDI CCs
	void midiParameterLearned(int iCC);
	//void writeMappingForDocumentation();
	
	int m_iDumpFileSampleCount = 0;
	std::unique_ptr<FileOutputStream> m_DumpOutStream = nullptr;
	bool m_bDumpFileCreated = false;
	AudioSampleBuffer* m_WAVDumpBuffer_MasterOutBuffer;
	AudioSampleBuffer* m_WAVDumpBuffer_FilterBuffer1;
	AudioSampleBuffer* m_WAVDumpBuffer_FilterBuffer2;
	AudioSampleBuffer* m_WAVDumpBuffer_FilterBuffer3;
	AudioSampleBuffer* m_WAVDumpBuffer_FilterVoices00;
	AudioSampleBuffer* m_WAVDumpBuffer_FilterVoices01;
	AudioSampleBuffer* m_WAVDumpBuffer_FilterVoices10;
	AudioSampleBuffer* m_WAVDumpBuffer_FilterVoices11;

	VASTVUMeterSource m_meterSource;

	int m_iNumPassTreeThreads = 0;
	bool getTreeThreadLock();	

    Label safePointerLabel; //for safePointerOnly
    
	VASTAudioProcessorEditor* cachedVASTEditor = nullptr;
};

#endif  
