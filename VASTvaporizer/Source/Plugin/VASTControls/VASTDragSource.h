/*
VAST Dynamics
*/

#ifndef VASTDRAGSOURCE_H_INCLUDED
#define VASTDRAGSOURCE_H_INCLUDED

#include "../../Engine/VASTEngineHeader.h"
#include "VASTPopupHandler.h"
#include "VASTImageButton.h"

class VASTDragSource : public GroupComponent, Label::Listener

{
public:
	VASTDragSource(const juce::String &componentName, const juce::String &dragText);

	~VASTDragSource();
	void resized() override;
	void paint(Graphics& g) override;

	void setAudioProcessor(VASTAudioProcessor &processor);
	VASTAudioProcessor* getAudioProcessor();
	void setModString(const juce::String &dragText) {
		ddLabel->setText(dragText, NotificationType::dontSendNotification);
	}
	void lookAndFeelChanged() override;
	void editorShown(Label *, TextEditor &) override;
	void labelTextChanged(Label* labelThatHasChanged) override;

private:
	ScopedPointer<VASTImageButton> ddImageButton;
	ScopedPointer<Label> ddLabel;
	bool m_noLabel = false;

	VASTAudioProcessor *m_processor;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VASTDragSource)
};
#endif