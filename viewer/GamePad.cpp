#pragma once
#include <iostream>
#include <iomanip>
#include "GamePad.h"

using std::cout;
using std::cerr;
using std::endl;

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
			io.run(); 
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
		io.stop();
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

			for (int i = 0; i < size; i++) {
				cout << std::hex << std::setfill('0') << std::setw(2) << (int)controller_data[i] << " ";
			}
			cout << endl;
		}
		catch (std::exception& e) {
			cerr << "Serial port read error" << e.what() << endl;
			connected = false;
		}
	}
}