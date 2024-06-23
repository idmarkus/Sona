#include "PluginEditor.hpp"
#include "PluginProcessor.hpp"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
	: AudioProcessorEditor(&p), processorRef(p), mpeKeyboard(p.mpeInstrument, MPEKeyboardComponent::horizontalKeyboard), logList(p.m_noteLog)
{
	juce::ignoreUnused(processorRef);

	// add sliders
	addAndMakeVisible(gainSlider);
	gainSlider.setSliderStyle(Slider::LinearBar);
	gainSlider.setRange(-100, -12);
	gainSlider.setTextBoxStyle(Slider::TextBoxRight, false, 100, 20);
	gainSlider.setVisible(false);

	addAndMakeVisible(freqSlider);
	freqSlider.setSliderStyle(Slider::LinearBar);
	freqSlider.setRange(300.0, 8000.0);
	freqSlider.setTextValueSuffix(" Hz");
	freqSlider.setTextBoxIsEditable(true);
	freqSlider.setSkewFactorFromMidPoint(600.0);
	freqSlider.setValue(600.0);
	freqSlider.setNumDecimalPlacesToDisplay(2);
	freqSlider.setMinAndMaxValues(300.0, 8000.0, NotificationType::dontSendNotification);

	// note log list
	addAndMakeVisible(logList, -1);
	logList.setVisible(true);

	// play button
	addAndMakeVisible(playButton);
	makePlayShapes();
	playButton.addListener(this);
	playButton.setShape(playShape0, true, true, true);

	// mpe keyboard
	addAndMakeVisible(mpeKeyboard);
	startTimerHz(60);// For syncing freq slider with mpeInstrument

	// set resize limits
	setResizeLimits(400, 200, 1024, 700);
	setResizable(true, true);

	setSize(500, 200);

	// pass slider ptrs to processor, we are
	// lifetime bound so raw ptr should be fine
	p.freqSlider = &freqSlider;
	p.gainSlider = &gainSlider;
}


//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
	// just fill background
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized()
{
	// mpe keyboard
	auto r = getLocalBounds();
	mpeKeyboard.setBounds(r.removeFromBottom(70));
	r = r.reduced(8);

	// sliders
	auto h = r.getHeight() / 4;
	freqSlider.setBounds(r.removeFromTop((h * 3) - 8));
	r.removeFromTop(8);

	// play button next to log
	playButton.setBounds(r.removeFromRight(h - 8));
	r.removeFromRight(8);
	gainSlider.setBounds(r);
	logList.setBounds(r);
}