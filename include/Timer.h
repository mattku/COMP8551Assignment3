#pragma once

#include <chrono>
/*
This class has 3 functions: Start(), End(), and ElapsedTime()
Start and End simply sets a timepoint object to current system clock
ElapsedTime will give the difference between these two times in double
Note: if Start and End are called in the wrong order, or neither are called,
ElapsedTime will give you an erroneous value.
*/
class Timer
{
public:

	void Start();
	void End();
	std::chrono::duration<double> ElapsedTime();
private:
	std::chrono::time_point<std::chrono::system_clock> start_, end_;
	std::chrono::duration<double> elapsed_seconds_;
};
