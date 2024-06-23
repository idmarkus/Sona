#pragma once

#include "NoteLog.hpp"
#include <JuceHeader.h>

class DecibelSlider : public Slider
{
public:
	DecibelSlider() = default;
	double getValueFromText(const String& text) override
	{
		auto minusInfinitydB = -100.0;
		auto decibelText = text.upToFirstOccurrenceOf("dB", false, false).trim();
		return decibelText.equalsIgnoreCase("-INF") ? minusInfinitydB : decibelText.getDoubleValue();
	}

	String getTextFromValue(double value) override
	{
		return Decibels::toString(value);
	}

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DecibelSlider);
};

class SingleLog : public Component, Timer
{
	std::vector<NoteLog> const& m_log;

public:
	explicit SingleLog(std::vector<NoteLog> const& log)
		: m_log(log)
	{
		startTimerHz(30);
	}

	void timerCallback() override
	{
		if (isVisible())
			repaint();
	}

	void paint(Graphics& g) override
	{
		auto r = getLocalBounds().toFloat();
		g.setColour(getLookAndFeel().findColour(Slider::rotarySliderFillColourId));
		g.fillRoundedRectangle(r, 2.0);
		g.setColour(getLookAndFeel().findColour(Slider::textBoxOutlineColourId));
		g.drawRoundedRectangle(r, 2.0f, 1.0f);
		if (!m_log.empty())
		{
			NoteLog const& item = m_log.back();

			g.setColour(Colours::white);
			g.setFont(r.getHeight() * 0.5f);
			const std::string textNote = MidiMessage::getMidiNoteName(item.note, true, true, 5).toStdString();
			const std::string textStart = std::to_string(item.startBuffer) + " : " + std::to_string(item.startSample);
			const std::string textEnd = (item.endBuffer > 0) ? " > " + std::to_string(item.endBuffer) + " : " + std::to_string(item.endSample) : "";

			g.drawFittedText("[" + textNote + "] " + textStart /* + textEnd*/, r.toNearestInt(), Justification::centred, 1);
		}
	}

	void resized() override
	{
		repaint();
	}
};