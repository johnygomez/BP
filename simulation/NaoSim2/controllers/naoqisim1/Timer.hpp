#ifndef TIMER_HPP
#define TIMER_HPP

// File:        Timer.hpp
// Description: Class helping to measure delta time in milliseconds
// Author:      Fabien Rohrer - www.cyberbotics.com

class Timer {
public:
  // constructor & destructor
  Timer();
  virtual ~Timer() {}
  
  double delta(); // second
  void reset();

private:
  double time() const; // second

  double beforeTime;
};

#endif
