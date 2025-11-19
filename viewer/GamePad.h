#pragma once
#include <asio.hpp>
#include <wx/wx.h>
#include "Scale.h"
#include "globals.h"

class GamePad
{
public:
	GamePad();
	~GamePad();
	std::array<uint8_t, 3> GetGamePad();
	void Update();
	std::vector<std::pair<Scale::Note, std::pair<std::string, std::string>>> GetInputStream();
	std::array<uint8_t, 3> DetectButtonEdge();

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
	bool connected = false;
	uint8_t start_byte = 0xAA;
	uint8_t end_byte = 0xBB;

	std::mutex mtx;
	std::array<uint8_t, 3> gamepad = {0,0,0};
	std::vector<std::pair<Scale::Note, std::pair<std::string, std::string>>> input_stream = {};

	std::array<uint8_t, 3> prev = {0,0,0};
	std::array<uint8_t, 3> curr = {0,0,0};

};

