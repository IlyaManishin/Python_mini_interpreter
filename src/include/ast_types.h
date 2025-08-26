#pragma once

#include "data_arena.h"

#include "internal/ast_base_types/expressions.h"
#include "internal/ast_base_types/literals.h"
#include "internal/ast_base_types/operations.h"
#include "internal/ast_base_types/statements.h"

#define BASE_NODES_ARRAY_SIZE 16

typedef enum NodeTypes
{
    NODES_ARRAY_TYPE,

    LITERAL_TYPE,
    OP_TYPE,
    STATEMENT_TYPE,
    EXPRESSION_TYPE,
    IDENT_TYPE

} NodeTypes;

typedef struct TNodeArray
{
    size_t length;
    TNode **nodes;
} TNodeArray;

typedef struct TIdent
{
    char *str;
    size_t length;
} TIdent;

typedef struct TNode
{
    NodeTypes type;
    union
    {
        TNodeArray array;

        TOperation op;
        TLiteral literal;
        TStatement statement;
        TExpr expr;
        TIdent ident;
    } nodeValue;

    int lineno;
    int col;
} TNode;

typedef struct TAst
{
    TDataArena *astArena;
    TNode *first;
} TAst;