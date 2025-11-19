#pragma once
#include <iomanip>
#include "GamePad.h"
#include "Scale.h"

namespace SwitchPro {
	namespace Buttons0 {
		static constexpr uint8_t Y = 0x01;
		static constexpr uint8_t X = 0x02;
		static constexpr uint8_t B = 0x04;
		static constexpr uint8_t A = 0x08;
		static constexpr uint8_t R = 0x40;
		static constexpr uint8_t ZR = 0x80;
	}
	namespace Buttons1 {
		static constexpr uint8_t MINUS = 0x01;
		static constexpr uint8_t PLUS = 0x02;
		static constexpr uint8_t R3 = 0x04;
		static constexpr uint8_t L3 = 0x08;
		static constexpr uint8_t HOME = 0x10;
		static constexpr uint8_t CAPTURE = 0x20;
	}
	namespace Buttons2 {
		static constexpr uint8_t DPAD_DOWN = 0x01;
		static constexpr uint8_t DPAD_UP = 0x02;
		static constexpr uint8_t DPAD_RIGHT = 0x04;
		static constexpr uint8_t DPAD_LEFT = 0x08;
		static constexpr uint8_t L = 0x40;
		static constexpr uint8_t ZL = 0x80;
	}
}

GamePad::GamePad() : serial(io){
	
}

GamePad::~GamePad() {
	Disconnect();
}

void GamePad::Connect(const std::string& portName) {
	if (connected.load()) {
		std::cout << "Already connected." << std::endl;
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

		connected.store(true);
		std::cout << "Connected to " << portName << std::endl;
	}
	catch (std::exception& e) {
		std::cout << "Error connecting to " << portName << ": " << e.what() << std::endl;
		connected.store(false);
	}
}

void GamePad::Disconnect() {
	if (!connected.load()) {
		return;
	}
	try {
		serial.close();
		if (ioThread.joinable()) {
			ioThread.join();
		}
		connected.store(false);
		std::cout << "Disconnected." << std::endl;
	}
	catch (std::exception& e) {
		std::cout << "Error during disconnect: " << e.what() << std::endl;
	}
}

void GamePad::ReadLoop() {
	while (connected.load()) {
		try {
			uint8_t current_byte;
			asio::read(serial, asio::buffer(&current_byte, 1));
			if (current_byte != START_BYTE) continue;

			uint8_t size;
			asio::read(serial, asio::buffer(&size, 1));
			std::vector<uint8_t> controller_data(size, 0);
			asio::read(serial, asio::buffer(controller_data));

			asio::read(serial, asio::buffer(&current_byte, 1));

			if (current_byte != END_BYTE) {
				std::cerr << "Invalid end byte" << std::endl;
				continue;
			}

			//for (int i = 0; i < size; i++) {
			//	std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)controller_data[i] << " ";
			//}
			//std::cout << std::endl;

			if (controller_data.size() < 3) {
				std::cerr << "Controller data too small: " << controller_data.size() << std::endl;
				continue;
			}

			std::lock_guard<std::mutex> lock(mtx);
			gamepad[0] = controller_data[0];
			gamepad[1] = controller_data[1];
			gamepad[2] = controller_data[2];
		}
		catch (std::exception& e) {
			std::cerr << "Serial port read error" << e.what() << std::endl;
			connected.store(false);
		}
	}
}

std::array<uint8_t, 3> GamePad::GetGamePad() {
	std::lock_guard<std::mutex> lock(mtx);
	return gamepad;
}

void GamePad::Update() {
	std::lock_guard<std::mutex> lock(mtx);
	prev = curr;
	curr = gamepad;
}

std::array<uint8_t, 3> GamePad::DetectButtonPressed() {
	std::array<uint8_t, 3> diff = {0,0,0};
	for (int i = 0; i < 3; i++) {
		// rising edge: prev=0, curr=1
		diff[i] = static_cast<uint8_t>((prev[i] ^ curr[i]) & curr[i]);
	}
	return diff;
}

std::array<uint8_t, 3> GamePad::DetectButtonReleased() {
	std::array<uint8_t, 3> diff = {0,0,0};
	for (int i = 0; i < 3; i++) {
		// falling edge: prev=1, curr=0
		diff[i] = static_cast<uint8_t>((prev[i] ^ curr[i]) & prev[i]);
	}
	return diff;
}

std::vector<std::pair<Scale::Note, std::pair<std::string, std::string>>> GamePad::GetInputStream() {
	auto diff = DetectButtonPressed();
	auto releaseDiff = DetectButtonReleased();

	if (diff[1] & SwitchPro::Buttons1::L3) {
		input_stream.clear();
		return input_stream;
	}

	// compute semitone offset from adjust buttons
	int semitone_offset = 0;
	if (curr[2] & SwitchPro::Buttons2::ZL) semitone_offset -= 12;  // ZL: octave down
	if (curr[2] & SwitchPro::Buttons2::L)  semitone_offset -= 1;   // L: semitone down
	if (curr[0] & SwitchPro::Buttons0::R)  semitone_offset += 1;   // R: semitone up
	if (curr[0] & SwitchPro::Buttons0::ZR) semitone_offset += 12;  // ZR: octave up

	// build index string describing current adjust buttons (space-separated)
	std::string idx=" ";
	if (curr[2] & SwitchPro::Buttons2::ZL) idx += "ZL ";
	if (curr[2] & SwitchPro::Buttons2::L)  idx += "L ";
	if (curr[0] & SwitchPro::Buttons0::R)  idx += "R ";
	if (curr[0] & SwitchPro::Buttons0::ZR) idx += "ZR ";

	// button definitions: byte index, mask, base note, label
	struct Btn { int byteIdx; uint8_t mask; Scale::Note base; const char* label; };
	const Btn buttons[] = {
		{2, SwitchPro::Buttons2::DPAD_UP,    Scale::C4, "↑"},
		{2, SwitchPro::Buttons2::DPAD_LEFT,  Scale::D4, "←"},
		{2, SwitchPro::Buttons2::DPAD_DOWN,  Scale::E4, "↓"},
		{2, SwitchPro::Buttons2::DPAD_RIGHT, Scale::F4, "→"},
		{0, SwitchPro::Buttons0::X,          Scale::G4, "X"},
		{0, SwitchPro::Buttons0::A,          Scale::A4, "A"},
		{0, SwitchPro::Buttons0::B,          Scale::B4, "B"},
		{0, SwitchPro::Buttons0::Y,          Scale::C5, "Y"}
	};

	auto pushNote = [&](const Btn& b) {
		Scale::Note n = Scale::transpose(b.base, semitone_offset);
		input_stream.push_back({n, {std::string(b.label), idx}});
	};

	// 1) handle rising edges (newly pressed buttons)
	for (const auto& b : buttons) {
		if (diff[b.byteIdx] & b.mask) pushNote(b);
	}

	// detect adjust press / release events
	bool adjustEdge = (diff[2] & (SwitchPro::Buttons2::ZL | SwitchPro::Buttons2::L)) || (diff[0] & (SwitchPro::Buttons0::R | SwitchPro::Buttons0::ZR));
	bool adjustRelease = (releaseDiff[2] & (SwitchPro::Buttons2::ZL | SwitchPro::Buttons2::L)) || (releaseDiff[0] & (SwitchPro::Buttons0::R | SwitchPro::Buttons0::ZR));

	// 2) if adjust button pressed, re-push currently held note(s) (that didn't just have a rising edge)
	if (adjustEdge) {
		for (const auto& b : buttons) {
			if ((curr[b.byteIdx] & b.mask) && !(diff[b.byteIdx] & b.mask)) {
				pushNote(b);
			}
		}
	}

	// 3) if adjust button released, push currently held note(s) again (reflecting new offset)
	if (adjustRelease) {
		for (const auto& b : buttons) {
			if ((curr[b.byteIdx] & b.mask) && !(diff[b.byteIdx] & b.mask)) {
				pushNote(b);
			}
		}
	}

	// keep buffer bounded
	if (input_stream.size() > 12) {
		input_stream.erase(input_stream.begin(), input_stream.begin() + (input_stream.size() - 12));
	}

	return input_stream;
}

void GamePad::SendDefaultNote(Scale::Note note_l, Scale::Note note_r) {
	if (!connected.load()) {
		std::cout << "Not connected, cannot send default note." << std::endl;
		return;
	}
	// Build packet: START_BYTE, size, cmd, note_l, note_r, END_BYTE
	const uint8_t cmd_set_default = 0x01;
	const uint8_t payload_size = 3; // cmd + note_l + note_r

	uint8_t packet[6];
	packet[0] = START_BYTE;
	packet[1] = payload_size;
	packet[2] = cmd_set_default;
	packet[3] = static_cast<uint8_t>(note_l);
	packet[4] = static_cast<uint8_t>(note_r);
	packet[5] = END_BYTE;

	try {
		// protect concurrent writes to the serial port
		std::lock_guard<std::mutex> wlock(serial_write_mtx);
		asio::write(serial, asio::buffer(packet, sizeof(packet)));
		std::cout << "Sent SendDefaultNote packet: " << std::hex << (int)packet[2] << " " << (int)packet[3] << " " << (int)packet[4] << std::dec << std::endl;
	}
	catch (const std::exception& e) {
		std::cout << "Error sending default note: " << e.what() << std::endl;
	}
}