/*
  A high-resolution stopwatch for timing pieces of code.
  Functions listed as "advanced use" require more care in using because of risk of data corruption and inaccurate results if the stopwatch is not
  used correctly.  Uses doubles for all time storage and returns. Minimizes inaccuracy by stopping the timer during upkeep operations.

  Basic Use:
	start() will start/resume the stopwatch -- multiple starts are ignored.
	stop() will stop the timing and return the milliseconds passed since the last start -- multiple stops will simply return the same value.
	reset() will clear the stored information in the stopwatch. If the stopwatch was running it will continue timing from when reset() was called.
	lap() will mark the current elapsed time but the stopwatch will continue running.

  Advanced Use:
	getTimes() will return a const reference to the forward_list storing the seconds duration of each lap.
	Crashes with assert if stopwatch is running, may cause data corruption if reference is in use and watch is reset or started.
	
	getLaps() will return a list storing the seconds of each lap in increasing order
	Crashes with assert if stopwatch is running.  Will not cause corruption under any use scenario.

	getAverage() will return the average duration of each lap in seconds
	Crashes with assert if stopwatch is running.  Will not cause corruption under any use scenario.
  
  Cory D. Boatright
  Grove City College
  January 31, 2014
  updated February 10, 2015 to make it cross-platform compatible

  Do not modify or distribute this or the corresponding lib file.
*/

#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>
#include <forward_list>
#include <list>

class StopWatch {
public:
	StopWatch(void);
	virtual ~StopWatch(void);

	void start(void);
	double stop(void);
	void reset(void);
	void lap(void);

	const std::forward_list<double>& getTimes(void) const;
	std::list<double> getLaps(void) const;
	double getAverage(void) const;

private:
	std::chrono::high_resolution_clock::time_point begin;
	std::chrono::high_resolution_clock::time_point end;

	bool running;

	std::forward_list<double> times;

	double currentTotal;
};

#endif