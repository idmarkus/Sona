#pragma once

#include <JuceHeader.h>

class MPEBuffer : public MPEInstrument::Listener
{
	//MidiBuffer m_bfr;
	//double m_rate = 48000.0;
	//double m_startTime = 0.0;
	//int m_sampleTime = 0;

	MidiMessageCollector m_collector;

public:
	explicit MPEBuffer(MPEInstrument& mpe)
	{
		mpe.addListener(this);
	}

	void reset(double rate)
	{
		m_collector.reset(rate);
	}

	void popBlock(MidiBuffer& bfr, int blockSize)
	{
		m_collector.removeNextBlockOfMessages(bfr, blockSize);
	}

	//- Listener interface
	void noteAdded(MPENote note) override
	{
		uint8 const& initialNote = note.initialNote;
		uint8 const& midiChannel = note.midiChannel;
		MPEValue const& velocity = note.noteOnVelocity;
		MPEValue const& pitchbend = note.pitchbend;
		MPEValue const& pressure = note.pressure;
		//		MPEValue const& initialTimbre = note.initialTimbre;
		MPEValue const& timbre = note.timbre;

		auto noteMessage = MidiMessage::noteOn((int)midiChannel, (int)initialNote, velocity.asUnsignedFloat());
		auto pitchMessage = MidiMessage::pitchWheel(midiChannel, pitchbend.as14BitInt());
		auto pressMessage = MidiMessage::channelPressureChange(midiChannel, pressure.as7BitInt());
		auto timbreMessage = MidiMessage::controllerEvent(midiChannel, 74, timbre.as7BitInt());

		append(noteMessage);
		append(pitchMessage);
		append(pressMessage);
		append(timbreMessage);
	}

	void notePressureChanged(MPENote note) override
	{
		auto msg = MidiMessage::channelPressureChange(note.midiChannel, note.pressure.as7BitInt());
		append(msg);
	}
	void notePitchbendChanged(MPENote note) override
	{
		auto msg = MidiMessage::pitchWheel(note.midiChannel, note.pitchbend.as14BitInt());
		append(msg);
	}

	void noteTimbreChanged(MPENote note) override
	{
		auto msg = MidiMessage::controllerEvent(note.midiChannel, 74, note.timbre.as7BitInt());
		append(msg);
	}
	void noteKeyStateChanged(MPENote note) override
	{
		ignoreUnused(note);
	}
	void noteReleased(MPENote note) override
	{
		auto msg = MidiMessage::noteOff(note.midiChannel, note.initialNote, note.noteOffVelocity.asUnsignedFloat());
		append(msg);
	}
	void zoneLayoutChanged() override
	{
	}

private:
	static double getRealTime()
	{
		return Time::getMillisecondCounterHiRes() * 0.001;
	}

	inline void append(MidiMessage& msg)
	{
		msg.setTimeStamp(getRealTime());
		m_collector.addMessageToQueue(msg);
	}
};