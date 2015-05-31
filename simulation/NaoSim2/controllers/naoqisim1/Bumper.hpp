#ifndef BUMPER_HPP
#define BUMPER_HPP

// File:        Bumper.hpp
// Description: Foot bumper
// Author:      Yvan Bourquin - www.cyberbotics.com

#include "Device.hpp"
#include <webots/types.h>

namespace Sim {
  class BumperSensor;
}

using namespace Sim;
using namespace std;

class Bumper : public Device {
public:
  // constructor and destructor
  Bumper(const BumperSensor *sensor, int step);
  virtual ~Bumper();
  
  // reimplemented functions
  virtual void update();
  
private:
  WbDeviceTag mTag;
  const BumperSensor *mSensor;
};

#endif
