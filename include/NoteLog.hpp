#pragma once

struct NoteLog {
	int note;
	size_t startBuffer, startSample, endBuffer = 0, endSample = 0;

	NoteLog(int note, size_t startBuffer, size_t startSample)
		: note(note), startBuffer(startBuffer), startSample(startSample)
	{
	}
};