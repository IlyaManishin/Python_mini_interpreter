#pragma once

typedef struct TNode TNode;

typedef enum OpTypes
{
    UNARY_OP,
    BIN_OP
} OpTypes;

typedef enum UnOpTypes
{
    UN_OP_PASS,
    UNO_MINUS
} UnOpTypes;

typedef enum BinOpTypes
{
    BIN_OP_PASS,

    BIN_OP_SUM, // +
    BIN_OP_SUB, // -
    BIN_OP_MUL, // *
    BIN_OP_DIV, // /

    BIN_OP_ASSIGN, // =
    UN_OP_POW,     // **
} BinOpTypes;

typedef struct TUnOperation
{
    UnOpTypes type;
    TNode *value;
} TUnOperation;

typedef struct TBinOperation
{
    BinOpTypes type;
    TNode *left;
    TNode *right;
} TBinOperation;

typedef struct TOperation
{
    OpTypes type;
    union
    {
        TUnOperation unOp;
        TBinOperation binOp;
    };

} TOperation;