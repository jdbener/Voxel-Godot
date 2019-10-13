/*
    Class which implements a simple timer. This timer will display the execution
    time of a block of code when requested or when it goes out of scope.
    File: Timer.h
    Author: The Cherno
    Modifications: Joshua "Jdbener" Dahl
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
    Timer(): start(std::chrono::high_resolution_clock::now()){}

    ~Timer(){
        stop();
    }

    // Version not requiring extra initialization for parameterized data
    void stop(){
        // Once requested to stop store the end time
        auto end = std::chrono::high_resolution_clock::now();
        // Convert to microseconds
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        // Convert to milliseconds
        double durationMS = duration * .001;

        // Display the duration in micro and milliseconds
        std::cout << "Duration: " <<  duration << "μs (" << durationMS << "ms)" << std::endl;
    }

    // Versions sacrificing a tiny amount of performance for flexibility
    void stop(std::ostream& stream){
        // Once requested to stop store the end time
        auto end = std::chrono::high_resolution_clock::now();
        // Convert to microseconds
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        // Convert to milliseconds
        double durationMS = duration * .001;

        // Display the duration is micro and milliseconds
        stream << "Duration: " <<  duration << L"μs (" << durationMS << "ms)" << std::endl;
    }
    // wchar version
    void stop(std::wostream& stream){
        // Once requested to stop store the end time
        auto end = std::chrono::high_resolution_clock::now();
        // Convert to microseconds
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        // Convert to milliseconds
        double durationMS = duration * .001;

        // Display the duration is micro and milliseconds
        stream << "Duration: " <<  duration << L"μs (" << durationMS << "ms)" << std::endl;
    }
};

#endif // _TIMER_H_
