/*
 * debug.cpp
 *
 *  Created on: Aug 10, 2020
 *      Author: bear
 */

#include "debug.h"

#include <stdio.h>
#include <string.h>

void printError(const char *file, int line, const char *function, int error) {
  printf("%s:%d: ERROR: %s: ", file, line, function);
  printf("errno(%d) is %s.\n", error, strerror(error));
}

