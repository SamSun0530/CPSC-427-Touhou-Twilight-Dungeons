#pragma once

// stlib
#include <chrono>
#include <string>

using Clock = std::chrono::high_resolution_clock;

class TimeDebug {
	Clock::time_point now;
	float elapsed_ms;
	Clock::time_point t;

public:
	void initTime();
	void getTime(std::string message);
};