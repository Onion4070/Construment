#pragma once
#include <asio.hpp>
#include <wx/wx.h>

class GamePad
{
public:
	GamePad();
	~GamePad();
	void Connect(const std::string& portName);
	void Disconnect();
	bool IsConnected() const { return connected; }

private:
	void ReadLoop();
	asio::io_context io;
	asio::serial_port serial;
	std::thread ioThread;
	bool connected = false;
	uint8_t start_byte = 0xAA;
	uint8_t end_byte = 0xBB;
};

