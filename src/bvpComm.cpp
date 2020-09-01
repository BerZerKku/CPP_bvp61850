/*
 * bvpComm.cpp
 *
 *  Created on: Aug 4, 2020
 *      Author: bear
 */
#include "bvpComm.hpp"

#include <assert.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "debug.h"

namespace BVP {

BvpComm::BvpComm() {
  // TODO Auto-generated constructor stub
  bvpPkg = new BvpPkg(BvpPkg::MODE_master);

  fd = -1;
}

BvpComm::~BvpComm() {
  disconnect();
}

bool BvpComm::connect(const char *busname) {
  if (fd < 0) {
    fd = open(busname, O_RDWR);
    if (fd < 0) {
      printf("%s:%d: ERROR: %s: ", __FILE__, __LINE__, __FUNCTION__);
      printf("Can't open I2C BUS to write.\n");
    } else {
      if (ioctl(fd, I2C_SLAVE, BVP_STM32_i2c_address) < 0) {
        printf("%s:%d: ERROR: %s: ", __FILE__, __LINE__, __FUNCTION__);
        printf("Can't set the I2C address for the slave write-device.\n");
        disconnect();
      }
    }
  } else {
    printf("%s:%d: ERROR: %s: ", __FILE__, __LINE__, __FUNCTION__);
    printf("Port already open.\n");
  }

  return (fd >= 0);
}

void BvpComm::disconnect() {
  if (fd >= 0) {
    close(fd);
    fd = -1;
  }
}

bool BvpComm::isConnect() const {
  return (fd >= 0) && (fd >= 0);
}

uint64_t counter = 0;

bool BvpComm::getParam(param_t param) {
  bool state = false;

  assert(param < PARAM_MAX);

//  uint8_t attempt = 3;
//  while (attempt-- > 0) {
//    state = readPacket(pkgTx);
//    if (state) {
//      break;
//    }
//  }
//
//  if (!state) {
//    errorCnt.pkg++;
//  }

  return state;
}

void BvpComm::printDebug(bool full) const {
  static errorCnt_t errorcnt;

  if (full || (memcmp(&errorcnt, &errorCnt, sizeof(errorcnt)) != 0)) {

    printf("DEBUG <<< Errors: pkg=%d, read=%d, write=%d!\n",
        errorCnt.pkg, errorCnt.read, errorCnt.write);

    errorcnt = errorCnt;
  }
}

bool BvpComm::readPacket() {
  uint16_t cnt = 0;
  uint16_t len = 0;
  uint8_t *buf = nullptr;

  buf = bvpPkg->getRxPkg(len);
  cnt = read(fd, buf, len);

  if (cnt != len) {
    printError( __FILE__, __LINE__, __FUNCTION__, errno);
    errorCnt.read++;
  }

  return  cnt == len;
}

//
bool BvpComm::setParam(param_t param) {
  bool state = false;
  uint16_t len = 0;
  uint8_t dataTx[BVP::DATA_LEN];
  uint8_t dataRx[BVP::DATA_LEN];

  assert(param < PARAM_MAX);

  for(uint16_t i = 0; i < BVP::DATA_LEN; i++) {
    dataTx[i] = '1' + i%10;
  }

  bvpPkg->addDataToPkg(dataTx, len);

  // FIXME Количество попыток может быть бесконечно!
  uint8_t attempt = 3;
  while ((!state) && (attempt > 0)) {
    state = writePacket();
    usleep(2000);

    if (state) {
      state = readPacket();
    }

    if (state) {
      state = bvpPkg->getDataFromPkg(dataRx, len);
    }

    attempt--;
  }

  if (!state) {
    errorCnt.pkg++;
  }

  return state;
}

bool BvpComm::writePacket() {
  uint16_t cnt = 0;
  uint16_t len = 0;
  uint8_t *buf = nullptr;

  buf = bvpPkg->getTxPkg(len);
  cnt = write(fd, buf, len);

  if (len != cnt) {
    printError( __FILE__, __LINE__, __FUNCTION__, errno);
    errorCnt.write++;
  }

  return len == cnt;
}

}/* namespace BVP */
