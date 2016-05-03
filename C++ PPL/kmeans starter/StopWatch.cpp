/*
  Cory D. Boatright
  Grove City College
  January 31, 2014
  updated February 10, 2015

  Do not modify or distribute this or the corresponding h file.
*/

#include "StopWatch.h"
#include <assert.h>
#include <iterator>

using namespace std;
using namespace std::chrono;

StopWatch::StopWatch() {
	running = false;
	times.push_front(0.0);
	currentTotal = 0.0;
}

StopWatch::~StopWatch() {
}

void StopWatch::start() {
	if(!running) {
		running = true;

		begin = high_resolution_clock::now();
	}
}

double StopWatch::stop() {
	if(running) {
		end = high_resolution_clock::now();
		running = false;

		microseconds diff = duration_cast<microseconds>(end - begin);
		
		currentTotal += diff.count() * (microseconds::period::num / static_cast<double>(microseconds::period::den)) / (milliseconds::period::num / static_cast<double>(milliseconds::period::den));
		times.front() += diff.count();
	}

	return currentTotal;
}

void StopWatch::reset() {
	bool prevRunning = running;
	this->stop();
	currentTotal = 0.0;
	times.clear();
	times.push_front(0.0);
	if(prevRunning) {
		this->start();
	}
}

void StopWatch::lap() {
	this->stop();
	times.push_front(0.0);
	this->start();
}

const forward_list<double>& StopWatch::getTimes() const {
	assert(!running);

	return times;
}

list<double> StopWatch::getLaps() const {
	assert(!running);

	list<double> result;
	double previous = 0.0;
	for(auto citer = times.cbegin(); citer != times.cend(); ++citer) {
		result.push_front(currentTotal - previous);
		previous += *citer;
	}

	return result;
}

double StopWatch::getAverage() const {
	assert(!running);

	if(currentTotal == 0.0) {
		return 0.0;
	}

	double n = static_cast<double>(distance(times.cbegin(), times.cend()));

	return currentTotal / n;
}