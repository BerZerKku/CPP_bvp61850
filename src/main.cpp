//============================================================================
// Name        : i2c.cpp
// Author      : Bear
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "bvpComm.hpp"

#include <iostream>
#include <signal.h>
#include <unistd.h>

using namespace std;
using namespace BVP;

BvpComm bvp;

volatile static bool running = true;

/// Остановка работы приложения
void sigint_handler(int signalId) {
    running = false;
}

int main() {
  cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!

  signal(SIGINT, sigint_handler);

  if (!bvp.connect("/dev/i2c-1")) {
    cout << "Does not connect to BVP!" << endl;
  }

  while(running) {
    if (!bvp.isConnect()) {
      break;
    }

    bvp.setParam(BvpComm::PARAM_no);
    usleep(10e3);

    bvp.printDebug();
//    bvp.getParam(BvpComm::PARAM_no);
//    usleep(10e3);

  }

  cout << endl;
  bvp.printDebug(true);

  cout << "Exit from program." << endl;
  bvp.disconnect();

  return 0;
}
