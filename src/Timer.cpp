#include <Timer.h>

using namespace std::chrono;

void Timer::Start() {
	start_ = system_clock::now();
}

void Timer::End() {
	end_ = system_clock::now();
}

duration<double> Timer::ElapsedTime() {
	elapsed_seconds_ = end_ - start_;
	return elapsed_seconds_;
}
