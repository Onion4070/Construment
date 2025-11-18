#pragma once
#include <iomanip>
#include "GamePad.h"
#include "Scale.h"

GamePad::GamePad() : serial(io){
	
}

GamePad::~GamePad() {
	Disconnect();
}

void GamePad::Connect(const std::string& portName) {
	if (connected) {
		cout << "Already connected." << endl;
		return;
	}

	try {
		serial.open(portName);

		serial.set_option(asio::serial_port_base::baud_rate(115200));
		serial.set_option(asio::serial_port_base::character_size(8));
		serial.set_option(asio::serial_port_base::parity(asio::serial_port_base::parity::none));
		serial.set_option(asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one));
		serial.set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none));

		// 別スレッドで実行
		ioThread = std::thread([this]() { 
			ReadLoop();
		});

		connected = true;
		cout << "Connected to " << portName << endl;
	}
	catch (std::exception& e) {
		cout << "Error connecting to " << portName << ": " << e.what() << endl;
		connected = false;
	}
}

void GamePad::Disconnect() {
	if (!connected) {
		return;
	}
	try {
		serial.close();
		if (ioThread.joinable()) {
			ioThread.join();
		}
		connected = false;
		cout << "Disconnected." << endl;
	}
	catch (std::exception& e) {
		cout << "Error during disconnect: " << e.what() << endl;
	}
}

void GamePad::ReadLoop() {
	while (connected) {
		try {
			uint8_t current_byte;
			asio::read(serial, asio::buffer(&current_byte, 1));
			if (current_byte != start_byte) continue;

			uint8_t size;
			asio::read(serial, asio::buffer(&size, 1));
			std::vector<uint8_t> controller_data(size, 0);
			asio::read(serial, asio::buffer(controller_data));

			asio::read(serial, asio::buffer(&current_byte, 1));

			if (current_byte != end_byte) {
				cerr << "Invalid end byte" << endl;
				continue;
			}

			//for (int i = 0; i < size; i++) {
			//	cout << std::hex << std::setfill('0') << std::setw(2) << (int)controller_data[i] << " ";
			//}
			//cout << endl;

			std::lock_guard<std::mutex> lock(mtx);
			gamepad = controller_data;
		}
		catch (std::exception& e) {
			cerr << "Serial port read error" << e.what() << endl;
			connected = false;
		}
	}
}

std::vector<uint8_t> GamePad::GetGamePad() {
	std::lock_guard<std::mutex> lock(mtx);
	return gamepad;
}

void GamePad::SendDefaultNote(Scale::Note note_l, Scale::Note note_r) {
	if (!connected) {
		cout << "Not connected, cannot send default note." << endl;
		return;
	}

	// Build packet: start, size, cmd, note_l, note_r, end
	const uint8_t start_byte = 0xAA;
	const uint8_t end_byte = 0xBB;
	const uint8_t cmd_set_default = 0x01;
	const uint8_t payload_size = 3; // cmd + note_l + note_r

	uint8_t packet[6];
	packet[0] = start_byte;
	packet[1] = payload_size;
	packet[2] = cmd_set_default;
	packet[3] = static_cast<uint8_t>(note_l);
	packet[4] = static_cast<uint8_t>(note_r);
	packet[5] = end_byte;

	try {
		// Write synchronously to serial port
		asio::write(serial, asio::buffer(packet, sizeof(packet)));
		cout << "Sent SendDefaultNote packet: " << std::hex << (int)packet[2] << " " << (int)packet[3] << " " << (int)packet[4] << std::dec << endl;
	}
	catch (const std::exception& e) {
		cout << "Error sending default note: " << e.what() << endl;
	}
}