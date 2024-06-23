#pragma once

#include <JuceHeader.h>

class MPESineVoice : public MPESynthesiserVoice
{
private:
	juce::SmoothedValue<double> level, timbre, frequency;

	double phase = 0.0;
	double phaseDelta = 0.0;
	double tailOff = 0.0;

	// some useful constants
	static constexpr auto maxLevel = 0.05;
	static constexpr auto maxLevelDb = 31.0;
	static constexpr auto smoothingLengthInSeconds = 0.01;

public:
	[[nodiscard]] double getRealFrequencyInHertz() const noexcept
	{
		return frequency.getCurrentValue();
	}

	void noteStarted() override
	{
		jassert(currentlyPlayingNote.isValid());
		jassert(currentlyPlayingNote.keyState == juce::MPENote::keyDown
				|| currentlyPlayingNote.keyState == juce::MPENote::keyDownAndSustained);

		// get data from the current MPENote
		level.setTargetValue(currentlyPlayingNote.pressure.asUnsignedFloat());
		frequency.setTargetValue(currentlyPlayingNote.getFrequencyInHertz());
		timbre.setTargetValue(currentlyPlayingNote.timbre.asUnsignedFloat());

		phase = 0.0;
		auto cyclesPerSample = frequency.getNextValue() / currentSampleRate;
		phaseDelta = 2.0 * juce::MathConstants<double>::pi * cyclesPerSample;

		tailOff = 0.0;
	}

	void noteStopped(bool allowTailOff) override
	{
		jassert(currentlyPlayingNote.keyState == juce::MPENote::off);
		if (allowTailOff)
		{
			// start a tail-off by setting this flag. The render callback will pick up on
			// this and do a fade out, calling clearCurrentNote() when it's finished.
			if (tailOff == 0.0)// we only need to begin a tail-off if it's not already
				tailOff = 1.0;
		}
		else
		{
			// we're being told to stop playing immediately
			clearCurrentNote();

			phaseDelta = 0.0;
		}
	}

	void notePressureChanged() override
	{
		level.setTargetValue(currentlyPlayingNote.pressure.asUnsignedFloat());
	}

	void notePitchbendChanged() override
	{
		frequency.setTargetValue(currentlyPlayingNote.getFrequencyInHertz());
	}

	void noteTimbreChanged() override
	{
		timbre.setTargetValue(currentlyPlayingNote.timbre.asUnsignedFloat());
	}

	void noteKeyStateChanged() override
	{
	}

	void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
						 int startSample, int numSamples) override
	{
		if (phaseDelta != 0.0)
		{
			if (tailOff > 0.0)
			{
				while (--numSamples >= 0)
				{
					auto currentSample = getNextSample() * (float)tailOff;

					for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
						outputBuffer.addSample(i, startSample, currentSample);

					++startSample;

					tailOff *= 0.99;

					if (tailOff <= 0.005)
					{
						clearCurrentNote();

						phaseDelta = 0.0;
						break;
					}
				}
			}
			else
			{
				while (--numSamples >= 0)
				{
					auto currentSample = getNextSample();

					for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
						outputBuffer.addSample(i, startSample, currentSample);

					++startSample;
				}
			}
		}
	}

private:
	float getNextSample() noexcept
	{
		auto levelDb = (level.getNextValue() - 1.0) * maxLevelDb;
		auto amplitude = std::pow(10.0f, 0.05f * levelDb) * maxLevel;

		// timbre is used to blend between a sine and a square.
		auto f1 = std::sin(phase);
		auto f2 = std::copysign(1.0, f1);
		auto a2 = timbre.getNextValue();
		auto a1 = 1.0 - a2;

		auto nextSample = float(amplitude * ((a1 * f1) + (a2 * f2)));

		auto cyclesPerSample = frequency.getNextValue() / currentSampleRate;
		phaseDelta = 2.0 * juce::MathConstants<double>::pi * cyclesPerSample;
		phase = std::fmod(phase + phaseDelta, 2.0 * juce::MathConstants<double>::pi);

		return nextSample;
	}
};