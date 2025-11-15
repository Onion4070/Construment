#pragma once
#include <iostream>
#include "GamePad.h"
using std::cout;
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