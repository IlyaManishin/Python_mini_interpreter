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
    INT_L,
    FLOAT_L,
    BOOL_L,
    NONE_L
} LiteralTypes;

typedef struct TBigLong
{
    digit *digits;
    int digitsCount;
    bool sign;
} TBigLong;

typedef struct TInt
{
    bool isLong;
    union
    {
        llong longData;
        TBigLong bigLongData;
    };
} TInt;

typedef struct TFloat
{
    TInt intPart;
    ldouble floatPart;
} TFloat;


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
        TInt intNum;
        TFloat floatNum;
        TString string;
        TBool boolLit;
        TNone none;
    };

} TLiteral;