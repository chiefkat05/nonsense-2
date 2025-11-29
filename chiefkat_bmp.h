
#ifndef CHIEFKAT_BMP_H
#define CHIEFKAT_BMP_H

#include "definitions.h"

typedef struct
{
    int width;
    int height;
    unsigned char *data;
} bmp_data;

bmp_data read_bmp(const char *path);

#endif