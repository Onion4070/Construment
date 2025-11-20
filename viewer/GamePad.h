#pragma once
#include <asio.hpp>
#include <wx/wx.h>
#include "Scale.h"
#include "globals.h"
#include "MIDIPlayer.h"

class GamePad
{
public:
	GamePad();
	~GamePad();
	std::vector<uint8_t> GetGamePad();

	void Connect(const std::string& portName);
	void Disconnect();
	bool IsConnected() const { return connected; }

	// Send default-note command to the device. The packet format is:
	// [start=0xAA][size=3][cmd=0x01][note_l][note_r][end=0xBB]
	// note_l / note_r are values from Scale::Note (stored as uint8_t).
	void SendDefaultNote(Scale::Note note_l, Scale::Note note_r);
	bool LoadMidi(const std::string& path);
	void PlayMidi();
	void StopMidi();
	void SendNotes(const ActiveNotes& notes);
	MIDIPlayer midi;
	bool midiPlaying = false;

private:
	Scale::Note MidiToScale(int midi_note);
	void ReadLoop();
	asio::io_context io;
	asio::serial_port serial;
	std::thread ioThread;
	bool connected = false;
	uint8_t start_byte = 0xAA;
	uint8_t end_byte = 0xBB;

	std::mutex mtx;
	std::vector<uint8_t> gamepad = {};
};

