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

#endif
