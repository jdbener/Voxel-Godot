/*
	Class which implements a simple timer. This timer will display the execution
	time of a block of code when requested or when it goes out of scope.
*/
#ifndef _TIMER_H_
#define _TIMER_H_

#include <chrono>
#include <iostream>

class Timer{
private:
	// Starting time point
	std::chrono::time_point<std::chrono::high_resolution_clock> start;

public:
	Timer(){
		// Once created start the clock
		start = std::chrono::high_resolution_clock::now();
	}

	~Timer(){
		stop();
	}

	void stop(std::ostream& stream = std::cout){
		// Once requested to stop store the end time
		auto end = std::chrono::high_resolution_clock::now();
		// Convert to microseconds
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		// Convert to milliseconds
		double durationMS = duration * .001;

		// Display the duration is micro and milliseconds
		stream << "Duration: " <<  duration << "μs (" << durationMS << "ms)" << std::endl;
	}
};

#endif // _TIMER_H_
