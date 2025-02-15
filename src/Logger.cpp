#include "Logger.hpp"

#include <chrono>
#include <ctime>
#include <iostream>

const std::chrono::system_clock::time_point Logger::start_time_{
    std::chrono::system_clock::now()};

void Logger::log(enum LEVEL lvl, std::string &msg)
{
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
	                std::chrono::system_clock::now() - start_time_)
	                .count();
	if (lvl == DEBUG) std::cout << diff << " [DEBUG] ";
	if (lvl == INFO) std::cout << diff << " [INFO] ";
	if (lvl == WARNING) std::cout << diff << " [WARNING] ";
	if (lvl == ERROR) std::cout << diff << " [ERROR] ";

	std::cout << msg << std::endl;
}
