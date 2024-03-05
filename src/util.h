#ifndef UTIL_H
#define UTIL_H

#include <MiniFB.h>

#define BASIL_VERSION "0.1.0"
#define MAX_PATH_SIZE 256

char *readFile(const char *path);
mfb_key stringToKey(const char *str);

#endif
