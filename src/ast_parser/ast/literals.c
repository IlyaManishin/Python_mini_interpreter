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

// bool is_const_ident(TToken ident)
// {
//     assert(ident.type == IDENT);
//     if (check_token_str(ident, "False") || check_token_str(ident, "True") || check_token_str(ident, "None"))
//         return true;
//     return false;
// }

static TNode *get_literal_node(TDataArena *arena, LiteralTypes type)
{
    TNode *literalNode = get_node(arena, LITERAL_TYPE);
    if (literalNode == NULL)
        return NULL;

    TLiteral *asLiteral = &literalNode->nodeValue.literal;
    asLiteral->type = type;

    return literalNode;
}

TNode *init_string_node(TDataArena *arena, TToken strToken)
{
    assert(strToken.type == STRING);
    return NULL;
    // size_t length;
    // char *str = copy_token_str(arena, strToken, &length);
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
    end++;

    *lengthDest = end - numberStr;
    return numberStr;
}

char *get_float_part_pos(char *numberStr, size_t length)
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

ldouble float_part_to_double(char *floatPos, size_t floatLength)
{
    char *curDigit = floatPos + 1;
    ldouble res = 0;
    ldouble multyPl = 0.1;
    for (int i = 0; i < floatLength; i++)
    {
        res += to_int(*curDigit) * multyPl;
        multyPl /= 10;
    }
    return res;
}

void parse_int_part(TInt *dest, char *intS, size_t intLength)
{
    if (intLength > MAX_INT_DIGITS_COUNT)
    {

        // TLong *longData = &asNumber->longData;
        // longData->digits = (digit *)numberS;
        // longData->digitsCount = sLength;
        // longData->sign = true;
    }
    else
    {
    }
}

TNode *init_number_node(TDataArena *arena, TToken numberToken)
{
    assert(numberToken.type == NUMBER);

    TNode *numberNode = get_literal_node(arena, NUMBER_L);
    if (numberNode == NULL)
        return NULL;
    TNumber *asNumber = &numberNode->nodeValue.literal.number;

    size_t sLength;
    char *numberStr = extract_number_from_token(arena, numberToken, &sLength);
    if (numberStr == NULL)
    {
        delete_node(arena, numberNode);
        return NULL;
    }

    char *floatPos = get_float_part_pos(numberStr, sLength);
    if (floatPos != NULL)
    {
        TFloat *asFloat = &asNumber->floatNum;

        asNumber->type = FLOAT_NUMBER;
        size_t intLength = floatPos - numberStr;
        size_t floatLength = sLength - intLength;

        asFloat->floatPart = float_part_to_double(floatPos, floatLength);
        parse_int_part((TInt *)asFloat, numberStr, intLength);
    }
    else
    {
        size_t intLength = sLength;
        asNumber->type = INT_NUMBER;

        parse_int_part(&asNumber->intNum, numberStr, intLength);
    }

    // fill int data
    return NULL;
}

TNode *init_bool_node(TDataArena *arena, bool value)
{
    TNode *boolNode = get_literal_node(arena, BOOL_L);
    if (boolNode == NULL)
        return NULL;

    TBool *asBool = &boolNode->nodeValue.literal.boolLit;
    asBool->value = value;
    return boolNode;
}

TNode *init_none_node(TDataArena *arena)
{
    TNode *noneNode = get_literal_node(arena, NONE_L);
    if (noneNode == NULL)
        return NULL;
    return noneNode;
}
