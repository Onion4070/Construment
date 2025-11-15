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
	asio::io_context io;
	asio::serial_port serial;
	std::thread ioThread;
	bool connected = false;
};

