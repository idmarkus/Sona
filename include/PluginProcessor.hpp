#pragma once

#include "MPEBuffer.hpp"
#include "MPESine.hpp"
#include "NoteLog.hpp"
#include "SineWave.hpp"
#include <JuceHeader.h>


//==============================================================================
class AudioPluginAudioProcessor : public AudioProcessor
{
public:
	// Methods ====================================================================
	AudioPluginAudioProcessor();
	~AudioPluginAudioProcessor() override = default;

	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
	void processBlock(AudioBuffer<float>&, MidiBuffer&) override;
	void processBlock(AudioBuffer<double>&, MidiBuffer&) override;
	using AudioProcessor::processBlock;

	juce::AudioProcessorEditor* createEditor() override;

	// Parameters =================================================================
	bool hasEditor() const override
	{
		return true;
	}
	bool acceptsMidi() const override
	{
		return true;
	}
	bool producesMidi() const override
	{
		return true;
	}
	bool supportsMPE() const override
	{
		return true;
	}
	bool isMidiEffect() const override
	{
		return false;
	}
	double getTailLengthSeconds() const override
	{
		return 0.0;
	}
	const String getName() const override
	{
		return JucePlugin_Name;
	}

	// Programs
	int getNumPrograms() override
	{
		return 1;
	}
	int getCurrentProgram() override
	{
		return 0;
	}
	void setCurrentProgram(int /*index*/) override
	{
	}
	const juce::String getProgramName(int /*index*/) override
	{
		return {};
	}
	void changeProgramName(int /*index*/, const String& /*newName*/) override
	{
	}

	// State
	void getStateInformation(juce::MemoryBlock& /*destData*/) override
	{
	}
	void setStateInformation(const void* /*data*/, int /*sizeInBytes*/) override
	{
	}
	//==============================================================================

	static constexpr double hzToNote(double hz)
	{
		return (std::log2(hz / 440.0) * 12.0) + 69.0;
	}

	static constexpr int hzToNoteRound(double hz)
	{
		return int(std::round(hzToNote(hz)));
	}

	[[nodiscard]] double getBend(double hz, int root) const
	{
		double ofs = (static_cast<double>(root) - hzToNote(hz)) / static_cast<double>(mpeInstrument.getLegacyModePitchbendRange());
		return (double)std::clamp(ofs, -1.0, 1.0);
	}

	static constexpr int bendToWheel(const double bend)
	{
		return 0x2000 - (int)std::round(double(0x2000) * bend);
	}

	int m_midiNote = 0;
	int m_midiWheel = 0x2000;
	size_t m_noteStartBuffer = 0;
	size_t m_noteStartSample = 0;
	size_t m_currentBuffer = 0;

	std::vector<NoteLog> m_noteLog;
	void startFrequency(double freq)
	{
		const ScopedLock sl(lock);
		m_midiNote = hzToNoteRound(freq);
		m_midiWheel = bendToWheel(getBend(freq, m_midiNote));
	}

	void stopFrequency()
	{
		const ScopedLock sl(lock);
		m_midiNote = -1;
	}


	//==============================================================================
	// These properties are public so that our editor component can access them
	// this is kept up to date with the midi messages that arrive, and the UI component
	// registers with it, so it can represent the incoming messages
	MPEInstrument mpeInstrument;
	MPEBuffer mpeBuffer;
	size_t m_currentSample = 0;
	size_t m_blockSize = 0;
	MPESynthesiser const& getMPESynthCRef()
	{
		return synth;
	}

	// Slider pointers assigned by our editor
	Slider* freqSlider = nullptr;
	Slider* gainSlider = nullptr;

private:
	CriticalSection lock;
	//==============================================================================
	template<typename T>
	void process(AudioBuffer<T>& buffer, MidiBuffer& midiMessages)
	{
		//		auto gainParamValue= 2.0f * Decibels::decibelsToGain(state.getParameter("gain") ->getValue());
		auto numSamples = buffer.getNumSamples();

		// clear buffers for sanity
		for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
			buffer.clear(i, 0, numSamples);

		// Now pass any incoming midi messages to our keyboard state object, and let it
		// add messages to the buffer if the user is clicking on the on-screen keys
		if (m_midiNote > 0)
		{
			auto msg = MidiMessage::noteOn(2, m_midiNote, 0.9f);
			auto msg2 = MidiMessage::pitchWheel(2, m_midiWheel);
			m_noteStartBuffer = m_currentBuffer;
			m_noteStartSample = m_blockSize / 2;
			midiMessages.addEvent(msg, (int)m_noteStartSample);
			midiMessages.addEvent(msg2, (int)m_noteStartSample);

			m_noteLog.emplace_back(m_midiNote, m_noteStartBuffer, m_noteStartSample);
			m_midiNote = 0;
		}
		else if (m_midiNote < 0)
		{
			jassert(m_noteLog.size() > 0);
			NoteLog& log = m_noteLog.back();
			auto msg = MidiMessage::noteOff(2, log.note, 0.5f);
			log.endBuffer = m_currentBuffer;
			log.endSample = m_blockSize / 2;
			midiMessages.addEvent(msg, (int)log.endSample);
			m_midiNote = 0;
		}

		// let synth process incoming or generated midi events
		synth.renderNextBlock(buffer, midiMessages, 0, numSamples);
		midiMessages.clear();
		mpeBuffer.popBlock(midiMessages, (int)m_blockSize);

		++m_currentBuffer;
	}
	template<typename T>
	inline void applyGain(AudioBuffer<T>& buffer, float gainLevel)
	{
		for (auto channel = 0; channel < getTotalNumOutputChannels(); ++channel)
			buffer.applyGain(channel, 0, buffer.getNumSamples(), gainLevel);
	}

	//==============================================================================

	MPESynthesiser synth;
	void initialiseSynth()
	{
		synth.addVoice(new MPESineVoice());
		//		auto numVoices = 8;
		//		for (auto i = 0; i < numVoices; ++i)
		//			synth.addVoice(new MPESineVoice());
	}


	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};