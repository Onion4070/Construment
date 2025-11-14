#pragma once
#include <string>
#include <vector>

class SerialUtils
{
public:
	struct SerialPortInfo {
		std::string port;
		std::string description;
	};
	
	static std::vector<SerialPortInfo> AvailablePorts();
};