#pragma once

#include <JuceHeader.h>

/// From Juce demos: a basic sine wave synth
class SineWaveSound : public SynthesiserSound
{
public:
	SineWaveSound() = default;

	bool appliesToNote(int /*midiNoteNumber*/) override
	{
		return true;
	}
	bool appliesToChannel(int /*midiChannel*/) override
	{
		return true;
	}
};

/// From Juce demos: a sine wave synth voice
class SineWaveVoice : public SynthesiserVoice
{
public:
	SineWaveVoice() = default;

	bool canPlaySound(SynthesiserSound* sound) override
	{
		return dynamic_cast<SineWaveSound*>(sound) != nullptr;
	}

	void startNote(int midiNoteNumber, float velocity,
				   SynthesiserSound* /*sound*/,
				   int /*currentPitchWheelPosition*/) override
	{
		currentAngle = 0.0;
		level = velocity * 0.15;
		tailOff = 0.0;

		auto cyclesPerSecond = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
		auto cyclesPerSample = cyclesPerSecond / getSampleRate();

		angleDelta = cyclesPerSample * MathConstants<double>::twoPi;
	}

	void stopNote(float /*velocity*/, bool allowTailOff) override
	{
		if (allowTailOff)
		{
			// start a tail-off by setting this flag. The render callback will pick up on
			// this and do a fade out, calling clearCurrentNote() when it's finished.
			if (approximatelyEqual(tailOff, 0.0))// we only need to begin a tail-off if it's not already doing so - the
												 // stopNote method could be called more than once.
				tailOff = 1.0;
		}
		else
		{
			// we're being told to stop playing immediately, so reset everything..
			clearCurrentNote();
			angleDelta = 0.0;
		}
	}

	void pitchWheelMoved(int /*newValue*/) override
	{
	}

	void controllerMoved(int /*controllerNumber*/, int /*newValue*/) override
	{
	}

	void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
	{
		if (!approximatelyEqual(angleDelta, 0.0))
		{
			if (tailOff > 0.0)
			{
				while (--numSamples >= 0)
				{
					auto currentSample = (float)(sin(currentAngle) * level * tailOff);

					for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
						outputBuffer.addSample(i, startSample, currentSample);

					currentAngle += angleDelta;
					++startSample;

					tailOff *= 0.99;

					if (tailOff <= 0.005)
					{
						// tells the synth that this voice has stopped
						clearCurrentNote();

						angleDelta = 0.0;
						break;
					}
				}
			}
			else
			{
				while (--numSamples >= 0)
				{
					auto currentSample = (float)(sin(currentAngle) * level);

					for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
						outputBuffer.addSample(i, startSample, currentSample);

					currentAngle += angleDelta;
					++startSample;
				}
			}
		}
	}

	using SynthesiserVoice::renderNextBlock;

private:
	double currentAngle = 0.0;
	double angleDelta = 0.0;
	double level = 0.0;
	double tailOff = 0.0;
};
