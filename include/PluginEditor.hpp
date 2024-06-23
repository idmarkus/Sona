#pragma once

#include "GUI.hpp"
#include "MPESine.hpp"
#include "NoteLog.hpp"
#include "PluginProcessor.hpp"
#include <JuceHeader.h>

//==============================================================================
class AudioPluginAudioProcessorEditor : public AudioProcessorEditor, Button::Listener, Timer
{
public:
	explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
	~AudioPluginAudioProcessorEditor() override = default;

	void paint(juce::Graphics&) override;
	void resized() override;

	void timerCallback() override
	{
		auto const& synth = processorRef.getMPESynthCRef();
		if (synth.getNumVoices() > 0)
		{
			auto const& voice = dynamic_cast<MPESineVoice*>(synth.getVoice(0));
			auto const note = voice->getCurrentlyPlayingNote();

			if (note.isValid())
			{
				freqSlider.setValue(voice->getRealFrequencyInHertz());
			}
		}
	}

	int getControlParameterIndex(Component& control) override
	{
		if (&control == &gainSlider)
			return 0;
		if (&control == &freqSlider)
			return 1;
		return -1;
	}

private:
	void buttonClicked(Button* button) override
	{
		if (button == &playButton)
		{
			b_playButton = !b_playButton;
			if (b_playButton)
			{
				playButton.setShape(playShape1, true, true, true);
				playButton.setColours(Colours::yellow, Colours::yellow, Colours::yellow);
				processorRef.startFrequency(freqSlider.getValue());
			}
			else
			{
				processorRef.stopFrequency();
				playButton.setShape(playShape0, true, true, true);
				playButton.setColours(Colours::white, Colours::white, Colours::white);
			}
			resized();
		}
	}

	//==============================================================================
	// Reference to our processor
	AudioPluginAudioProcessor& processorRef;

	MPEKeyboardComponent mpeKeyboard;

	DecibelSlider gainSlider;
	SingleLog logList;
	Slider freqSlider;

	ShapeButton playButton{"play", Colours::white, Colours::white, Colours::white};
	Path playShape0, playShape1;

	/** Premake triangle and square shapes for the play button */
	void makePlayShapes()
	{
		playShape0.addTriangle(0.0f, 0.0f, 1.0f, 0.5f, 0.0f, 1.0f);
		playShape1.addRectangle(0.0f, 0.0f, 1.0f, 1.0f);
	}

	bool b_playButton = false;
	//	Colour backgroundColour;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
