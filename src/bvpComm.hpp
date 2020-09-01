/*
 * bvpComm.h
 *
 *  Created on: Aug 4, 2020
 *      Author: bear
 */

#ifndef BVPCOMM_HPP_
#define BVPCOMM_HPP_

#include <stdint.h>
#include "CPP_bvpCommon/bvpCommon.hpp"
#include "external/threadbone/types.hpp"
#include "external/threadbone/tb_port.hpp"

namespace BVP {

using swissknife::types::TChannel;

/** Класс работы с МК блока БВП.
 *
 */
class BvpComm {

  enum boolSignal_t {
    BOOL_SIGNAL_regimeDisable = 0,
    BOOL_SIGNAL_regimeEnable,
    //
    BOOL_SIGNAL_MAX
  };

  /// Счетчики ошибок.
  struct errorCnt_t {
    uint32_t pkg    = 0; ///< Количество ошибок передачи.
    uint32_t read   = 0; ///< Количество ошибок приема.
    uint32_t write  = 0; ///< Количество ошибок передачи.
  };

public:

  typedef threadbone::TbPort<TChannel, 32> signalBool_t;

  enum param_t {
    PARAM_no = 0,
    PARAM_time,
    PARAM_MAX
  };

  BvpComm();

  virtual ~BvpComm();

  /** Начать работу с устройством.
   *
   *  @param[in] busname Название шины, например "/dev/i2c-1".
   *  @return true если устройство готово к работе, иначе false.
   */
  bool connect(const char *busname);

  /// Остановить работу с устройством.
  void disconnect();

  /** Проверка наличия установленного соединения.
   *
   *  @return true если соединение установлено, иначе false.
   */
  bool isConnect() const;

  /** Установить параметр.
   *
   *  @param[in] param Установить параметр.
   */
  bool setParam(param_t param);

  /** Считать параметр.
   *
   *  @param[in] param Считать параметр.
   */
  bool getParam(param_t param);

  /** Получить текущее значение счетчика ошибок.
   *
   *  @return Значение счетчика ошибок.
   */
  uint32_t getErrorCounter() const { return errorCnt.pkg; }

  /** Вывести отладочную информацию.
   *
   *  @param[in] full Вывод всей информации (true) или только новой (false).
   */
  void printDebug(bool full=false) const;

//  signalBool_t tbPortBool;

private:
  /// Дескриптор устройства для записи.
  int fd;

  /// Пакеты для общения c STM32.
  BvpPkg *bvpPkg;

  /// Счетчики ошибок.
  errorCnt_t errorCnt;

  /// Принимает пакет данных.
  bool readPacket();

  /// Передает пакет данных.
  bool writePacket();
};

} /* namespace BVP */

#endif /* BVPCOMM_HPP_ */
