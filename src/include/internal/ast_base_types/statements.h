#pragma once

#define BASE_STATETEMENTS_SIZE 8

typedef struct TNode TNode;

enum StatementsTypes
{
    FUNC_RUN_STMT // temp
};

typedef struct TFuncRunStmt
{
    TNode* ident;
    TNode* args;
} TFuncRunStmt;

typedef struct TStatement
{
    enum StatementsTypes type;
    union
    {
        TFuncRunStmt funcRun;
    };

} TStatement;
