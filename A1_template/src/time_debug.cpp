#include "time_debug.hpp"

void TimeDebug::initTime() {
	t = Clock::now();
}

void TimeDebug::getTime(std::string message) {
	now = Clock::now();
	elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;

	printf("%s elapsed: %f\n", message.c_str(), elapsed_ms);
}