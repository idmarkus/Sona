#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"


//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
	: AudioProcessor(
		BusesProperties()
			.withOutput("Output", juce::AudioChannelSet::mono(), true)),
	  mpeBuffer(mpeInstrument), synth{mpeInstrument}
{
	// TODO: Currently only legacy mode MPE is supported.
	mpeInstrument.enableLegacyMode(2);
	synth.enableLegacyMode(2);
	synth.setVoiceStealingEnabled(false);
	initialiseSynth();
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	synth.setCurrentPlaybackSampleRate(sampleRate);
	reset();

	mpeInstrument.releaseAllNotes();
	mpeBuffer.reset(sampleRate);
	m_currentSample = 0;
	m_blockSize = samplesPerBlock;

	m_currentBuffer = 0;
	m_noteLog.clear();
	m_midiNote = 0;
	m_midiWheel = 0x2000;
}

void AudioPluginAudioProcessor::releaseResources()
{
	mpeInstrument.releaseAllNotes();
	m_currentSample = 0;
	m_currentBuffer = 0;
	m_noteLog.clear();
	m_midiNote = 0;
	m_midiWheel = 0x2000;
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
	// Only stereo/mono, no input
	const auto& mainOutput = layouts.getMainOutputChannelSet();
	const auto& mainInput = layouts.getMainInputChannelSet();
	return (mainInput.isDisabled() && mainOutput.size() == 1);
}

void AudioPluginAudioProcessor::processBlock(AudioBuffer<float>& bfr, MidiBuffer& midi)
{
	juce::ScopedNoDenormals noDenormals;
	process(bfr, midi);
}
void AudioPluginAudioProcessor::processBlock(AudioBuffer<double>& bfr, MidiBuffer& midi)
{
	process(bfr, midi);
}

//==============================================================================
AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
	return new AudioPluginAudioProcessorEditor(*this);
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new AudioPluginAudioProcessor();
}