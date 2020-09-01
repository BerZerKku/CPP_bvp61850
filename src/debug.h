/*
 * debug.h
 *
 *  Created on: Aug 10, 2020
 *      Author: bear
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Вывод на экран ошибки и места где она произошла.
#define PRINT_ERROR printError( __FILE__, __LINE__, __FUNCTION__, errno)

/** Вывод на экран ошибки и места где она произошла.
 *
 *  @param[in] file
 *  @param[in] line
 *  @param[in] function
 *  @param[in] error
 */
void printError(const char *file, int line, const char *function, int error);



#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H_ */
