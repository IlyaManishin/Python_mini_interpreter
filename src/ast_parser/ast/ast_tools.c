#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ast_parser_api.h"
#include "ast_types.h"
#include "data_arena.h"

#include "../lexer/tokenizer_api.h"
#include "../parser.h"
#include "ast.h"

bool lookahead(TAstParser *p, TokenTypes checkType)
{
    TTokenizer *tok = p->tokenizer;

    while (1)
    {
        TToken token = token_soft_read(tok);
        if (token.type == EOF_TOKEN)
        {
            return false;
        }
        if (is_tokenizer_error(tok))
        {
            TTokenizerError error = get_tokenizer_error(tok);
            if (error.type == MEMORY_ERROR)
            {
                set_memory_crit_error(p);
                return false;
            }
            append_tokenizer_error(p, error);
            pass_tokenizer_error(tok);
            continue;
        }
        return token.type == checkType;
    }
    return false;
}

bool check_next_token(TAstParser *p, TokenTypes type)
{
    int pos = get_tokenizer_pos(p->tokenizer);
    int res = lookahead(p, type);
    set_tokenizer_pos(p->tokenizer, pos);

    return res;
}

TToken read_token(TAstParser *p)
{
    return token_soft_read(p->tokenizer);
}

static TNode *get_node(TDataArena *arena, NodeTypes type)
{
    TNode *node = (TNode *)arena_malloc(arena, sizeof(TNode));
    if (node == NULL)
        return NULL;
    node->type = type;
    return node;
}

TNode *init_array_node(TDataArena *arena, NodeTypes itemType, TNode **nodes, int length)
{
    TNode *node = get_node(arena, NODES_ARRAY_TYPE);
    if (node == NULL)
        return NULL;

    TNodeArray *asArray = &node->nodeValue.array;
    asArray->length = length;
    asArray->nodes = nodes;

    return node;
}

TNode *init_empty_array_node(TDataArena *arena, NodeTypes itemType)
{
    return init_array_node(arena, itemType, NULL, 0);
}

void delete_node(TDataArena *arena, TNode *node)
{
    arena_free(arena, node);
}

static char *copy_token_str(TDataArena *arena, TToken token, size_t *lengthDest)
{
    size_t length = token_strlen(token);
    assert(length != 0);

    char *res = (char *)arena_malloc(arena, length + 1);
    if (res == NULL)
        return NULL;

    memcpy(res, token.start, length);
    res[length] = '\0'; // need?
    if (lengthDest != NULL)
        *lengthDest = length;
    return res;
}

TNode *init_bin_op_node(TDataArena *arena, BinOpTypes opType, TNode *left, TNode *right)
{
    TNode *node = get_node(arena, OP_TYPE);
    if (node == NULL)
        return NULL;

    TBinOperation *asBinOp = &node->nodeValue.op.binOp;
    asBinOp->type = opType;
    asBinOp->left = left;
    asBinOp->right = right;

    return node;
}

TNode *init_unary_op_node(TDataArena *arena, UnOpTypes opType, TNode *child)
{
    TNode *node = get_node(arena, OP_TYPE);
    if (node == NULL)
        return NULL;

    TUnOperation *asUnOp = &node->nodeValue.op.unOp;
    asUnOp->type = opType;
    asUnOp->child = child;

    return node;
}

TNode *init_run_func_node(TDataArena *arena, TNode *funcIdent, TNode *args)
{
    TNode *node = get_node(arena, STATEMENT_TYPE);
    if (node == NULL)
        return NULL;
    TFuncRunStmt *asFuncRun = &node->nodeValue.statement.funcRun;
    asFuncRun->ident = funcIdent;
    asFuncRun->args = args;

    return node;
}

bool is_ident_token(TToken token)
{
    return token.type == IDENT && !is_bool_ident(token);
}

TNode *init_ident_node(TDataArena *arena, TToken identToken)
{
    assert(identToken.type == IDENT);
    TNode *node = get_node(arena, IDENT_TYPE);
    if (node == NULL)
        return NULL;

    size_t identLength;
    char *identName = copy_token_str(arena, identToken, &identLength);
    if (identName == NULL)
    {
        delete_node(arena, node);
        return NULL;
    }
    TIdent *asIdent = &node->nodeValue.ident;
    asIdent->str = identName;
    asIdent->length = identLength;

    return node;
}
TNode *init_string_node(TDataArena *arena, TToken identToken)
{
    return NULL;
}

TNode *init_number_node(TDataArena *arena, TToken identToken)
{
    return NULL;
}
