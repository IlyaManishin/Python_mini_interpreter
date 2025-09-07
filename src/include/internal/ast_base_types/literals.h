#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef long double ldouble;
typedef long long llong;


enum LiteralSettings
{
    MAX_INT_DIGITS_COUNT = 8,
};

typedef uint8_t digit;

typedef enum LiteralTypes
{
    STRING_L,
    NUMBER_L,
    BOOL_L,
    NONE_L
} LiteralTypes;

typedef enum NumberTypes
{
    INT_NUMBER,
    FLOAT_NUMBER
} NumberTypes;

typedef struct TLong
{
    digit *digits;
    int digitsCount;
    bool sign;
} TLong;

typedef struct TInt
{
    bool isLong;
    union
    {
        llong intData;
        TLong longData;
    };
} TInt;

typedef struct TFloat
{
    TInt intPart;
    ldouble floatPart;
} TFloat;

typedef struct TNumber
{
    NumberTypes type;
    union
    {
        TInt intNum;
        TFloat floatNum;
    };
} TNumber;

typedef struct TString
{
    char *data;
    size_t length;
} TString;

typedef struct TBool
{
    bool value;
} TBool;

typedef struct TNone
{

} TNone;

typedef struct TLiteral
{
    LiteralTypes type;
    union
    {
        TNumber number;
        TString string;
        TBool boolLit;
        TNone none;
    };

} TLiteral;