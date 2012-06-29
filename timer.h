#include <sys/time.h>
#include <stdio.h>

#ifndef TIMER_H
#define TIMER_H

class Timer {
protected:
   bool started;
   timeval begin;
   double duration;
public:
   Timer(double duration);
   void start();
   void reset();
   bool isExpired();
};

Timer::Timer(double duration) {
   this->duration = duration;
   this->started = false;
}

void Timer::start() {
   this->started = true;
   gettimeofday(&begin, NULL);
}

void Timer::reset() {
   this->started = true;
   gettimeofday(&begin, NULL);
}

bool Timer::isExpired() {
   timeval now;
   gettimeofday(&now, NULL);

   double millsElapsed = ((now.tv_sec + (now.tv_usec/1000000.0)) - (begin.tv_sec + (begin.tv_usec/1000000.0))) * 1000;

   //printf("ELAPSED: %f\n", millsElapsed);

   if (millsElapsed >= duration)
      return true;
   else
      return false;
}

#endif
