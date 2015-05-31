#include "Singletons.hpp"
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdio.h>
#include <qi/os.hpp>
#include <qi/path.hpp>
#include <qi/qi.hpp>
#include <alsimutils/sim_launcher.h>

#ifdef WIN32
#define SEP "\\"
#else
#define SEP "/"
#endif

Model *Singletons::mModel = NULL;
HALInterface *Singletons::mHal = NULL;
Sim::SimLauncher* Singletons::mLauncher = NULL;

bool Singletons::initialize(const string &model, int naoqiPort, void (*halInitializedCallback)()) {
  // model file name to upper
  string robotFileName = model;
  for (int i = 1; i < (int)robotFileName.length(); i++)
    robotFileName[i] = toupper(robotFileName[i]);
  // remove "_" in the file name
  std::string::iterator newEnd = std::remove(robotFileName.begin(),
                                             robotFileName.end(), '_');
  robotFileName.erase(newEnd, robotFileName.end());

  cout << "===== starting alsim controller =====\n" << flush;

  const string cdUp("..");

  // get SDK prefix
  string sdkPrefix = getenv("WEBOTS_NAOSIM_PATH");
  cout << "sdkPrefix: " << sdkPrefix << endl;
  
#ifdef MACOS 
  string dyld_lib_path = getenv("DYLD_LIBRARY_PATH");
  string dyld_fram_path = getenv("DYLD_FRAMEWORK_PATH");
  
  if (dyld_lib_path.find(sdkPrefix + "/lib") == string::npos)
    setenv("DYLD_LIBRARY_PATH",(dyld_lib_path + ":" + sdkPrefix + "/lib").c_str(), 1);
  if (dyld_fram_path.find(sdkPrefix) == string::npos)
    setenv("DYLD_FRAMEWORK_PATH",(dyld_fram_path + ":" + sdkPrefix).c_str(), 1); 
#endif

  // SDK to model
  const string sdkToModel(string("share") + SEP + string("alrobotmodel") + SEP + string("models") + SEP + robotFileName + ".xml");

  // Webots installed sdk
  const string pathToNaoqiSdk(sdkPrefix + SEP);

  // test if we use Aldebaran installed SDK
  string pathToModel = pathToNaoqiSdk + sdkToModel;

  // instantiate model
  try {
    mModel = new Model(pathToModel);
  }
  catch (const exception &) {
    cerr << "Could not instantiate model with path: \"" << pathToModel << "\""
         << endl;
  }

  try {
    mHal = new HALInterface(mModel, naoqiPort);
  }
  catch (const exception &e) {
    cerr << "exception raised: " << e.what() << "\n";
    cerr << "while calling HALInterface::HALInterface(" << mModel << ", " << naoqiPort << ")\n";
    return false;
  }
  mLauncher = new Sim::SimLauncher();
#ifdef MACOS
  // TODO: remove this case when Aldebaran release the fixed simulator-sdk for Mac
  return mLauncher->launch(mModel, naoqiPort, sdkPrefix);
#else
  return mLauncher->launch(mModel, naoqiPort, sdkPrefix, std::vector<std::string>(), halInitializedCallback);
#endif
}

void Singletons::shutdown() {
  cout << "===== shutting down ! =====\n" << flush;
  delete mLauncher;
  delete mHal;
  delete mModel;
}
