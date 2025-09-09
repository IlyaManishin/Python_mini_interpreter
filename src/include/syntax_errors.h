#pragma once

#include <stdbool.h>

#define CRIT_ERR_MSG_LENGTH 256

typedef struct TErrorFilePos
{
    const char *errStart;
    const char *errLineStart;
    int errLineIndex;
    const char *bufferEnd;
} TErrorFilePos;

typedef enum ParserErrorTypes
{
    INVALID_TOKEN,
    INVALID_RULE,
    MEMORY_ERROR,
    WARNING
} ParserErrorTypes;

typedef struct TTokenizerError
{
    ParserErrorTypes type;
    const char *textMsg;

    bool withPos;
    TErrorFilePos pos;
} TTokenizerError;

typedef struct TAstParserError
{
    char *textMsg;
    TErrorFilePos pos;
    //???
} TAstParserError;

typedef struct TCritError
{
    bool isError;
    char msg[CRIT_ERR_MSG_LENGTH + 1];
} TCritError;

void print_error_msg(const char *textMsg);
void print_error_with_pos(const char *errMsg, TErrorFilePos errorPosInfo);
TErrorFilePos get_error_file_pos(const char *errStart, const char *errLineStart, int errLineIndex, const char *bufferEnd);