#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ast_types.h"
#include "data_arena.h"
#include "system_tools.h"

#include "../lexer/tokenizer_api.h"
#include "../parser.h" //???
#include "ast.h"

#define to_int(digit) ((digit) - '0')

static TNode *init_literal_node(TDataArena *arena, LiteralTypes type)
{
    TNode *literalNode = get_node(arena, LITERAL_TYPE);
    if (literalNode == NULL)
        return NULL;

    TLiteral *asLiteral = &literalNode->nodeValue.literal;
    asLiteral->type = type;

    return literalNode;
}

static char *extract_string_from_token(TDataArena *arena, TToken strToken, size_t *lengthDest)
{
    size_t length;
    char *str = copy_token_str(arena, strToken, &length);
    if (str == NULL)
        return NULL;

    char *cur = str;
    char *last = str;
    bool isLastSlash = false;
    for (int i = 0; i < length; i++)
    {
        if (*cur == '\\')
        {
            if (isLastSlash)
            {
                *last++ = '\\';
            }
            isLastSlash = !isLastSlash;
        }
        else if (isLastSlash)
        {
            switch (*cur)
            {
            case 'n':
                *last++ = '\n';
                break;
            case 'r':
                *last++ = '\r';
                break;
            case 't':
                *last++ = '\t';
                break;
            default:
                *last++ = '\\';
                *last++ = *cur;
            }
            isLastSlash = false;
        }
        else
        {
            *last++ = *cur;
        }
        cur++;
    }
    *last++ = '\0';
    *lengthDest = last - str;

    return str;
}

TNode *init_string_node(TDataArena *arena, TToken strToken)
{
    assert(strToken.type == STRING);

    size_t strLength;
    char *str = extract_string_from_token(arena, strToken, &strLength);
    if (str == NULL)
        return NULL;

    TNode *strNode = init_literal_node(arena, STRING_L);
    if (strNode == NULL)
    {
        arena_free(arena, str);
        return NULL;
    }
    TString *asString = &strNode->nodeValue.literal.string;
    asString->data = str;
    asString->length = strLength;

    return strNode;
}

static char *extract_number_from_token(TDataArena *arena, TToken numberToken, size_t *lengthDest)
{
    size_t sLength;
    char *numberStr = copy_token_str(arena, numberToken, &sLength);
    if (numberStr == NULL)
        return NULL;

    char *cur = numberStr;
    char *end = numberStr;
    for (int i = 0; i < sLength; i++)
    {
        if (*cur != '_')
        {
            *end = *cur;
            end++;
        }
        cur++;
    }
    *end = '\0';

    *lengthDest = end - numberStr;
    return numberStr;
}

static char *get_float_point_pos(char *numberStr, size_t length)
{
    char *cur = numberStr;
    for (int i = 0; i < length; i++)
    {
        if (*cur == '.')
            return cur;
        cur++;
    }
    return NULL;
}

static ldouble float_part_to_double(char *pointPos, size_t floatLength)
{
    if (floatLength > FLOAT_DIGITS_LIMIT)
    {
        // set warning
    }
    char *curDigit = pointPos + 1;
    ldouble res = 0.0L;
    ldouble multyPl = 0.1L;
    for (int i = 0; i < floatLength; i++)
    {
        res += to_int(*curDigit) * multyPl;
        multyPl /= 10;
        curDigit++;
    }
    return res;
}

static void parse_int_part(TDataArena *arena, TInt *dest, char *intS, size_t intLength)
{
    while (intLength > 0)
    {
        if (*intS != '0')
            break;

        intS++;
        intLength--;
    }
    if (intLength == 0)
    {
        dest->isBigLong = false;
        dest->longData = 0ll;
        return;
    }

    if (intLength > MAX_LONG_DIGITS_COUNT)
    {
        digit *digits = (digit *)arena_malloc(arena, intLength * sizeof(digit));
        for (int i = 0; i < intLength; i++)
        {
            digits[i] = (digit)to_int(intS[i]);
        }
        dest->isBigLong = true;

        TBigLong *asBig = &dest->bigLongData;
        asBig->digits = digits;
        asBig->digitsCount = intLength;
        asBig->sign = false;
    }
    else
    {
        dest->isBigLong = false;
        dest->longData = atoll(intS);
    }
}

TNode *init_number_node(TDataArena *arena, TToken numberToken)
{
    assert(numberToken.type == NUMBER);

    size_t sLength;
    char *numberStr = extract_number_from_token(arena, numberToken, &sLength);
    if (numberStr == NULL)
        return NULL;

    TNode *res;
    char *pointPos = get_float_point_pos(numberStr, sLength);
    if (pointPos != NULL)
    {
        res = init_literal_node(arena, FLOAT_L);
        if (res == NULL)
            return NULL;

        TFloat *asFloat = &res->nodeValue.literal.floatNum;

        size_t intLength = pointPos - numberStr;
        size_t floatLength = sLength - intLength;

        asFloat->floatPart = float_part_to_double(pointPos, floatLength);
        *pointPos = '\0';
        parse_int_part(arena, (TInt *)asFloat, numberStr, intLength);
    }
    else
    {
        res = init_literal_node(arena, INT_L);
        if (res == NULL)
            return NULL;
        size_t intLength = sLength;

        TInt *asInt = &res->nodeValue.literal.intNum;
        parse_int_part(arena, asInt, numberStr, intLength);
    }
    arena_free(arena, numberStr);
    return res;
}

TNode *init_bool_node(TDataArena *arena, bool value)
{
    TNode *boolNode = init_literal_node(arena, BOOL_L);
    if (boolNode == NULL)
        return NULL;

    TBool *asBool = &boolNode->nodeValue.literal.boolLit;
    asBool->value = value;
    return boolNode;
}

TNode *init_none_node(TDataArena *arena)
{
    TNode *noneNode = init_literal_node(arena, NONE_L);
    if (noneNode == NULL)
        return NULL;
    return noneNode;
}
