/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "../Engine/VASTEngineHeader.h"
#include "VASTAudioProcessor.h"
#include "VASTAudioProcessorEditor.h"

//==============================================================================
VASTAudioProcessorEditor::VASTAudioProcessorEditor(VASTAudioProcessor& p)
	: AudioProcessorEditor(&p), processor(p) {

	juce::Rectangle<int> r = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
	int screenW = r.getWidth();
	int screenH = r.getHeight();
	if (m_iMaxWidth > screenW) m_iMaxWidth = screenW;
	if (m_iMaxHeight > screenH) m_iMaxHeight = screenH;

	resizeCalledFromConstructor = true;

	processor.m_mapParameterNameToControl.clear(); //clear slider mapping
	vaporizerComponent = new VASTVaporizerComponent(this, &processor);
	
	//VASTAudioProcessor* _processor = getProcessor();

	m_VASTComponentsAll = juce::Array<Component*>();
	//Make sure that before the constructor has finished, you've set the
	//editor's size to whatever you need it to be.

	setResizable(true, true);

	if ((processor.m_iUserTargetPluginHeight == 0) || (processor.m_iUserTargetPluginWidth == 0)) {
		processor.m_iUserTargetPluginWidth = processor.m_iDefaultPluginWidth;
		processor.m_iUserTargetPluginHeight = processor.m_iDefaultPluginHeight;
	}

	tooltipWindow.setLookAndFeel(getProcessor()->getCurrentVASTLookAndFeel());
	
	addAndMakeVisible(vaporizerComponent);
	resizeCalledFromConstructor = true;
	//setSize(processor.m_iUserTargetPluginWidth, processor.m_iUserTargetPluginHeight);
	resizeCalledFromConstructor = true;
	m_componentBoundsConstrainer.setFixedAspectRatio(processor.m_dPluginRatio); 
	m_componentBoundsConstrainer.setSizeLimits(m_iMinWidth, m_iMinHeight, m_iMaxWidth, m_iMaxHeight); //CHECK https://forum.juce.com/t/best-way-to-implement-resizable-plugin/12644/5	
	setConstrainer(&m_componentBoundsConstrainer);
	resizeCalledFromConstructor = true;
	setSize(processor.m_iUserTargetPluginHeight * processor.m_dPluginRatio, processor.m_iUserTargetPluginHeight);
	//vaporizerComponent->setBounds(Rectangle<int>(0, 0, processor.m_iUserTargetPluginWidth, processor.m_iUserTargetPluginHeight));
	vaporizerComponent->setVersionText(processor.getVersionString());

	//VST3 HACK CHECKTS
	/*
	if (processor.wrapperType == AudioProcessor::wrapperType_VST3) {
		Component::SafePointer<VASTAudioProcessorEditor> editor(this);
		Timer::callAfterDelay(50, [editor, this] {
			if (editor != nullptr) {
				setSize(editor.getComponent()->getProcessor()->m_iUserTargetPluginWidth + 1, editor.getComponent()->getProcessor()->m_iUserTargetPluginHeight + 1);
				setScaleFactor(1.f); //HACK TEST
			}
		});
		Timer::callAfterDelay(100, [editor, this] {
			if (editor != nullptr) {
				setSize(editor.getComponent()->getProcessor()->m_iUserTargetPluginWidth, editor.getComponent()->getProcessor()->m_iUserTargetPluginHeight);
				setScaleFactor(1.f); //HACK TEST
			}
		});
	}
	*/

	setOpaque(true);
	startTimer(0, 100); //ui update
	startTimer(1, 10); //param update
}

VASTAudioProcessorEditor::~VASTAudioProcessorEditor() {
	if (vaporizerComponent->m_wasShown) { //otherwise its an unused cached editor
		VASTAudioProcessor* _processor = getProcessor();
		_processor->createCachedVASTEditorDelayed();
	}
	m_alertWindow = nullptr;
	stopTimer(0);
	stopTimer(1); //param update
	this->setLookAndFeel(nullptr);
	vaporizerComponent = nullptr;
}

void VASTAudioProcessorEditor::registerComponentValueUpdate(Component* comp, float lValue) {
	bShallComponentValueUpdate = true;
	shallComponentUpdate = comp;
	shallComponentUpdateValue = lValue;
}

Component* VASTAudioProcessorEditor::findChildComponetWithName(Component* parent, String compName)
{
	Component* retComp = nullptr;
	for (int i = 0; i < parent->getNumChildComponents(); ++i)
	{
		Component* childComp = parent->getChildComponent(i);
		if (childComp->getName().equalsIgnoreCase(compName))
			return childComp;
		if (childComp->getNumChildComponents()) {
			if (retComp == nullptr)
				retComp = findChildComponetWithName(childComp, compName);
			else
				return retComp;
		}
	}
	return retComp;
}

//should be optimal now
void VASTAudioProcessorEditor::timerCallback(int timerID) {
	if (vaporizerComponent->isActive == false) return; //not yet ready?

	if (timerID == 1) { //component uzdate
		if (bShallComponentValueUpdate) {
			if (shallComponentUpdate != nullptr) {
				VASTSlider* lslider = dynamic_cast<VASTSlider*>(shallComponentUpdate);
				if (lslider != nullptr) {
					float sVal = lslider->getRange().getStart() + (lslider->getRange().getEnd() - lslider->getRange().getStart()) * jlimit<float>(0.f, 1.f, shallComponentUpdateValue);
					lslider->setValue(sVal, NotificationType::sendNotificationAsync);
				}
			}
			bShallComponentValueUpdate = false;
		}
	}
	else { //ui update
		VASTAudioProcessor* _processor = getProcessor();
		vaporizerComponent->setLicenseText(_processor->getLicenseText(), _processor->isInErrorState(), _processor->getErrorState());

		if (_processor->m_showNewerVersionPopup) {
			showNewerVersionPopup();
		}

		if (_processor->needsUIInit()) {
			vaporizerComponent->initAll();
			_processor->clearUIInitFlag();
		}

		if (_processor->needsUIUpdate()) {
			if (_processor->needsUIUpdate_tabs())
				vaporizerComponent->updateAll();

			if (_processor->needsUIUpdate_matrix())
				vaporizerComponent->updateMatrixDisplay();

			if (_processor->needsUIUpdate_sliders()) {
				if ((_processor->needsUIUpdate_slider1dest() == -1) && (_processor->needsUIUpdate_slider2dest() == -1)) { //repaint all sliders 
					for (int i = 0; i < _processor->m_mapParameterNameToControl.size(); i++) {
						VASTParameterSlider* lslider = dynamic_cast<VASTParameterSlider*>(_processor->m_mapParameterNameToControl[i]);
						if (lslider != nullptr) {
							if (lslider->isShowing())
								lslider->repaint();
						}
					}
				}
				else { //repaint only the two that are given
					String param1name = _processor->autoDestinationGetParam(_processor->needsUIUpdate_slider1dest());
					String param2name = _processor->autoDestinationGetParam(_processor->needsUIUpdate_slider2dest());

					for (int i = 0; i < _processor->m_mapParameterNameToControl.size(); i++) {
						VASTParameterSlider* lslider = dynamic_cast<VASTParameterSlider*>(_processor->m_mapParameterNameToControl[i]);
						if (lslider != nullptr) {
							if ((lslider->getComponentID().equalsIgnoreCase(param1name)) || (lslider->getComponentID().equalsIgnoreCase(param2name))) {
								if (lslider->isShowing())
									lslider->repaint();
							}
						}
					}
				}
			}

			if (_processor->needsUIPresetUpdate()) {
				VASTPresetComponent* pres = vaporizerComponent->getConcertinaPanel()->getPresetOverlay();
				if (pres != nullptr)
					pres->updateAll();
				_processor->clearUIPresetFlag();
			}
			if (_processor->needsUIPresetReloadUpdate()) {
				VASTPresetComponent* pres = vaporizerComponent->getConcertinaPanel()->getPresetOverlay();
				if (pres != nullptr)
					pres->reloadPresets();
				_processor->clearUIPresetReloadFlag();
			}

			_processor->clearUIUpdateFlag();
		}

		if (_processor->wantsUIAlert()) {
			_processor->clearUIAlertFlag();
			AlertWindow::showMessageBoxAsync(MessageBoxIconType::WarningIcon, TRANS("Load preset failed"), TRANS("Invalid data structure."), TRANS("Continue"), this);
		}

		VASTMasterVoicingComponent* voicingComp = vaporizerComponent->getMasterVoicingComponent();
		if (voicingComp != nullptr) {
			Label* labelV = voicingComp->getComponentCVoices();
			int numVoicesPlaying = _processor->m_pVASTXperience.m_Poly.numNotesPlaying();
			int numOscsPlaying = _processor->m_pVASTXperience.m_Poly.numOscsPlaying();
			labelV->setText(String(String(numVoicesPlaying) + "/" + String(numOscsPlaying)), NotificationType::dontSendNotification);
		}
	}
}

VASTAudioProcessor* VASTAudioProcessorEditor::getProcessor() {
	return &processor;
}

void VASTAudioProcessorEditor::paint(Graphics& g) {
	//do nothing, but needed for opaque
}

//==============================================================================


void VASTAudioProcessorEditor::resized() {
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	DBG("AudioProcessorEditor resized to w: " + String(this->getWidth()) + " h: " + String(this->getHeight()));
	/*
	Component::SafePointer<VASTAudioProcessorEditor> editor(this);
	if (this->getWidth() < (m_iMinWidth + 10)) {
		resizeCalledFromConstructor = true;
		Timer::callAfterDelay(100, [editor, this] { 
			if (editor != nullptr)
				editor.getComponent()->setBoundsConstrained(juce::Rectangle<int>(0,0, editor->m_iMinWidth + 30, editor->getHeight())); });
		return;
	}
	*/

	/*
	if (this->getHeight() < (m_iMinHeight + 10)) {
		resizeCalledFromConstructor = true;
		Timer::callAfterDelay(100, [editor, this] { 
			if (editor != nullptr)
				editor.getComponent()->setBoundsConstrained(juce::Rectangle<int>(0, 0, editor->getWidth(), editor->m_iMinHeight + 30)); });
		return;
	}

	if (processor.wrapperType == AudioProcessor::wrapperType_VST3) {
		if (!this->resizableCorner->isMouseOverOrDragging())
			if (resizeCalledFromConstructor == false) {
				processor.readSettingsFromFile();
				resizeCalledFromConstructor = true;
				Timer::callAfterDelay(100, [editor, this] { 
					if (editor != nullptr)
						editor.getComponent()->setBoundsConstrained(juce::Rectangle<int>(0, 0, editor.getComponent()->getProcessor()->m_iUserTargetPluginWidth, editor.getComponent()->getProcessor()->m_iUserTargetPluginHeight)); });
			}
	}
	*/

	resizeCalledFromConstructor = false;
	getCurrentVASTLookAndFeel()->setCurrentScalingFactors(getProcessor()->getPluginScaleWidthFactor(), getProcessor()->getPluginScaleHeightFactor());
	initAllLookAndFeels();

	if (m_componentBoundsConstrainer.isBeingCornerResized) {
		getProcessor()->m_iUserTargetPluginWidth = getWidth();
		getProcessor()->m_iUserTargetPluginHeight = getHeight();
		getProcessor()->writeSettingsToFile(); //must not be async
	}

	vaporizerComponent->setSize(this->getWidth(), this->getHeight());
}

void VASTAudioProcessorEditor::startPaintingToImage() {
	vaporizerComponent->startPaintingToImage();
}
void VASTAudioProcessorEditor::endPaintingToImage() {
	vaporizerComponent->endPaintingToImage();
}

void VASTAudioProcessorEditor::initAllLookAndFeels() {
	int num = getProcessor()->vastLookAndFeels.size() - 1;
	for (int i = 0; i < num; i++) {
		getProcessor()->vastLookAndFeels[i]->initAll();
	}
}

void VASTAudioProcessorEditor::setActiveLookAndFeel(int no) {
	vassert((no >= 0) && (no < getProcessor()->vastLookAndFeels.size()));
	initAllLookAndFeels();
	no = jlimit<int>(0, getProcessor()->vastLookAndFeels.size() -1, no);
	getProcessor()->m_activeLookAndFeel = no;
	setLookAndFeel(getProcessor()->getCurrentVASTLookAndFeel());
	tooltipWindow.setLookAndFeel(getProcessor()->getCurrentVASTLookAndFeel());
	if (vaporizerComponent != nullptr)
		vaporizerComponent->setLookAndFeel(getProcessor()->getCurrentVASTLookAndFeel());
	//LookAndFeel::setDefaultLookAndFeel(getProcessor()->getCurrentVASTLookAndFeel()); //causes problems with standalone app
};

void VASTAudioProcessorEditor::showNewerVersionPopup() {
	getProcessor()->m_showNewerVersionPopup = false;
	//transportable alertwindows without modalloop! https://forum.juce.com/t/is-there-a-easy-way-to-replace-alertwindow-runmodalloop/15072/16
	m_alertWindow = new juce::AlertWindow(TRANS("Newer version " + getProcessor()->m_newerVersionThatIsAvailble +" available"), TRANS("Please visit the website and download the newest version of Vaporizer2."), juce::AlertWindow::InfoIcon, this);
	m_alertWindow->setLookAndFeel(getProcessor()->getCurrentVASTLookAndFeel());
	m_alertWindow->addButton("Continue", 0, juce::KeyPress(), juce::KeyPress());
	m_alertWindow->addButton("Open Website", 1, juce::KeyPress(), juce::KeyPress());
	m_alertWindow->addTextBlock(L"Please download here: https://www.vast-dynamics.com/?q=products.");
	m_alertWindow->grabKeyboardFocus();
	vaporizerComponent->addChildComponent(m_alertWindow);
	m_alertWindow->setCentreRelative(0.5f, 0.5f);
	m_alertWindow->enterModalState(true, ModalCallbackFunction::create([this](int returnValue) {
		if (returnValue == 1) {
			const URL websiteurl = URL("https://www.vast-dynamics.com/?q=products");
			bool worked = websiteurl.launchInDefaultBrowser();
#ifdef _DEBUG
			if (!worked)
				DBG("Launch browser did not work.");
#endif

		}
		m_alertWindow->setLookAndFeel(nullptr);
		m_alertWindow = nullptr;
	}), true);
}
