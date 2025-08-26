#pragma once
#include "stdlib.h"

typedef struct TNode TNode;

typedef enum ExprTypes
{
    EXPR

} ExprTypes;

typedef struct TExpr
{
    TNode *op;
} TExpr;


