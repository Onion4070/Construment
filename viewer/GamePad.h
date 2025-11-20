#pragma once
#include <asio.hpp>
#include <wx/wx.h>
#include "Scale.h"
#include "globals.h"
#include <atomic>

struct NoteInfo {
	Scale::Note note;
	std::string label;
	std::string indexStr;
};

class GamePad
{
public:
	GamePad();
	~GamePad();
	std::array<uint8_t, 3> GetGamePad();
	void Update();
	// Provide current default notes (from DrawPanel) so defaults are shared between viewer and GamePad.
	// Pass by reference so GamePad can modify the defaults (e.g. set to Silence when R3 is pressed).
	std::vector<std::pair<NoteInfo, NoteInfo>> GetInputStream(Scale::Note &default_left, Scale::Note &default_right);
	std::array<uint8_t, 3> DetectButtonPressed();
	std::array<uint8_t, 3> DetectButtonReleased();

	void Connect(const std::string& portName);
	void Disconnect();
	bool IsConnected() const { return connected; }

	// Send default-note command to the device. The packet format is:
	// [start=0xAA][size=3][cmd=0x01][note_l][note_r][end=0xBB]
	// note_l / note_r are values from Scale::Note (stored as uint8_t).
	void SendDefaultNote(Scale::Note note_l, Scale::Note note_r);

private:
	void ReadLoop();
	asio::io_context io;
	asio::serial_port serial;
	std::thread ioThread;
	std::atomic<bool> connected{false};
	static constexpr uint8_t START_BYTE = 0xAA;
	static constexpr uint8_t END_BYTE = 0xBB;

	// mutex protecting access to gamepad state (prev/curr/gamepad)
	std::mutex mtx;
	// mutex guarding serial writes to avoid concurrent write while ReadLoop runs
	std::mutex serial_write_mtx;

	std::array<uint8_t, 3> gamepad = {0,0,0};
	std::vector<std::pair<NoteInfo, NoteInfo>> input_stream = {};

	std::array<uint8_t, 3> prev = {0,0,0};
	std::array<uint8_t, 3> curr = {0,0,0};
	// last pair that was considered "playing" (used to detect changes)
	std::pair<NoteInfo, NoteInfo> last_playing_pair = { {Scale::Silence, "", ""}, {Scale::Silence, "", ""} };

};

