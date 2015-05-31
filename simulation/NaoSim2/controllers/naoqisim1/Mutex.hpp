#ifndef MUTEX_HPP
#define MUTEX_HPP

// File:        Mutex.hpp
// Description: Class that implements a platform independent mutex
// Author:      Olivier Michel - www.cyberbotics.com

#ifdef WIN32
#include "windows.h"
#else
#include "pthread.h"
#endif

class Mutex {
public:
  Mutex();
  virtual ~Mutex();  
  void lock();
  void unlock();
private:
#ifdef WIN32
  HANDLE mutex;
#else
  pthread_mutex_t *mutex;
#endif
};

#endif
