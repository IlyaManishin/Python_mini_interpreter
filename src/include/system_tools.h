#pragma once

#include <stdio.h>
#include <stdlib.h>

enum SystemSettings
{
    MAX_STRNLEN = 1028,
    MAX_STRNCMP = 1028
};

typedef struct TFileData
{
    char *str;
    size_t dataSize;
} TFileData;

size_t nsu_strnlen(const char *s, size_t maxlen);

TFileData read_file_data(FILE *file);
void delete_file_data(TFileData data);
