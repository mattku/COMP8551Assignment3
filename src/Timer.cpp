#include <Timer.h>

Timer::Timer() {
	//I don't think anything needs to happen in this
}

Timer::~Timer() {
	//Shrug
}

void Timer::Start() {
	start_ = std::chrono::system_clock::now();
}

void Timer::End() {
	end_ = std::chrono::system_clock::now();
}

double Timer::ElapsedTime() {
	elapsed_seconds_ = end_ - start_;
	return elapsed_seconds_.count();
}
