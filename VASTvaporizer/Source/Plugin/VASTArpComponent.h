/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 6.1.2

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include "../Engine/VASTEngineHeader.h"
#include "VASTControls/VASTComboBrev.h"
#include "VASTControls/VASTComboBox.h"
#include "VASTControls/VASTSlider.h"
#include "VASTControls/VASTComboPreset.h"
#include "VASTControls/VASTImageButton.h"
#include "VASTMSEGEditor/VASTStepSeqEditorPane.h"
#include "VASTMSEGEditor/VASTStepSeqEditor.h"
#include "VASTLFOMsegComponent.h"

class VASTTabbedSTEPSEQComponent : public TabbedComponent
{
public:
	std::function<void(int)> TabChangedFunc;
	VASTAudioProcessor* myProcessor = nullptr;
	VASTTabbedSTEPSEQComponent(VASTAudioProcessor* processor, TabbedButtonBar::Orientation orientation) : TabbedComponent(orientation), myProcessor(processor) {
		TabChangedFunc = [](int) {};
	}
	void currentTabChanged(int index, const String&) override
	{
		VASTStepSeqEditorPane* tab1 = ((VASTStepSeqEditorPane*)getTabContentComponent(0));
		if (tab1 == nullptr) return; //not initialied
		tab1->stopAutoUpdate();
		VASTStepSeqEditorPane* tab2 = ((VASTStepSeqEditorPane*)getTabContentComponent(1));
		if (tab2 == nullptr) return; //not initialied
		tab2->stopAutoUpdate();
		VASTStepSeqEditorPane* tab3 = ((VASTStepSeqEditorPane*)getTabContentComponent(2));
		if (tab3 == nullptr) return; //not initialied
		tab3->stopAutoUpdate();

		switch (index) {
		case 0:
			tab1->startAutoUpdate();
			break;
		case 1:
			tab2->startAutoUpdate();
			break;
		case 2:
			tab3->startAutoUpdate();
			break;
		}
		TabChangedFunc(index);
	}

	TabBarButton* createTabButton(const String &tabName, int tabIndex) {
		return new VASTDnDTabBarButton(myProcessor, tabName, getTabbedButtonBar());
	}
};
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class VASTArpComponent  : public Component,
                          public Button::Listener
{
public:
    //==============================================================================
    VASTArpComponent (AudioProcessorEditor *editor, AudioProcessor* processor);
    ~VASTArpComponent() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
	void initAll();
	void updateAll();
	void lookAndFeelChanged() override;
	void startAutoUpdate();
	void stopAutoUpdate();
	void buttonClicked(Button* buttonThatWasClicked);
	void mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel) override;
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Binary resources:
    static const char* arrow_right_corner_svg;
    static const int arrow_right_corner_svgSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
	VASTAudioProcessorEditor * myEditor = nullptr;
	VASTAudioProcessor* myProcessor = nullptr;
	juce::Time lastMouseWheelEvent = juce::Time::getCurrentTime();
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<VASTTabbedSTEPSEQComponent> c_stepseqTab;
    std::unique_ptr<juce::TabbedComponent> c_arpTab;
    std::unique_ptr<VASTDrawableButton> c_iconMaximizeEditor;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VASTArpComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

