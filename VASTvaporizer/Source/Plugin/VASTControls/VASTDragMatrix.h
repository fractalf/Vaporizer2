/*
VAST Dynamics
*/

#ifndef VASTDRAGMATRIX_H_INCLUDED
#define VASTDRAGMATRIX_H_INCLUDED

#include "../../Engine/VASTEngineHeader.h"
#include "VASTPopupHandler.h"
#include "VASTImageButton.h"
#include "VASTParameterComboBox.h"
#include "VASTParameterSlider.h"
#include "VASTDrawableButton.h"
#include "../VASTScopeDisplay/VASTModMatCurveDisplay.h"
#include "../VASTUtils/VASTGUINotifier.h"

class VASTDragMatrix : public GroupComponent, public Button::Listener, public DragAndDropTarget

{
public:
	VASTDragMatrix(VASTAudioProcessorEditor *editor, VASTAudioProcessor *processor, int slotno);

	~VASTDragMatrix();
	void resized() override;
	void paint(Graphics& g) override;
	void updateContent(bool force);

	void startAutoUpdate();
	void stopAutoUpdate();

	VASTAudioProcessor* getAudioProcessor();
	void lookAndFeelChanged() override;
	void buttonClicked(Button* b) override;
	void buttonStateChanged(Button* b) override;

	bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
	void itemDropped(const SourceDetails& dragSourceDetails) override;

private:
	VASTAudioProcessor* my_processor;
	VASTAudioProcessorEditor* my_editor;
	int my_slotno = 0;

	ScopedPointer<VASTImageButton> ddImageButton; //drag button
	ScopedPointer<VASTParameterComboBox> m_uModMatSrce; 
	ScopedPointer<VASTParameterComboBox> m_uModMatPolarity;
	ScopedPointer<VASTParameterComboBox> m_uModMatDest;
	ScopedPointer<VASTParameterSlider> m_fModMatVal;
	ScopedPointer<VASTParameterSlider> m_fModMatCurve;
	ScopedPointer<VASTModMatCurveDisplay> c_curveDisplay;
	ScopedPointer<VASTDrawableButton>c_iconDelSlot;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VASTDragMatrix)
};
#endif