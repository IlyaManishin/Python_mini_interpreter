#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast_parser_api.h"
#include "ast_types.h"

#include "../parser.h"
#include "ast.h"

#define RULE_PREFIX()         \
    if (is_critical_error(p)) \
        return NULL;

#define HANDLE_NULL_NODE(node)    \
    if (node == NULL)             \
    {                             \
        set_memory_crit_error(p); \
        return NULL;              \
    }

#define DONE_RULE_POSTFIX(returnValue) \
    done:                              \
    flush_used_tokens(p->tokenizer);   \
    return returnValue;

static TNode *statements_rule(TAstParser *p);
static TNode *statement_rule(TAstParser *p);
static TNode *assign_rule(TAstParser *p);
static TNode *func_run_rule(TAstParser *p);
static TNode *read_args_rule(TAstParser *p);
static TNode *expr_rule(TAstParser *p);
static TNode *term_rule(TAstParser *p);
static TNode *factor_rule(TAstParser *p);
static TNode *unsigned_factor_rule(TAstParser *p);
static TNode *pow_rule(TAstParser *p);
static TNode *primary_rule(TAstParser *p);
static TNode *number_rule(TAstParser *p);
static TNode *string_rule(TAstParser *p);
static TNode *ident_rule(TAstParser *p);

// if we can read rule => flush tokens

TNode *file_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    rewind_tokenizer_pos(p->tokenizer);
    return statements_rule(p);
}

static TNode *statements_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    TDataArena *arena = get_parser_arena(p);

    size_t length = 0;
    size_t capacity = BASE_STATETEMENTS_SIZE;
    TNode **statements = (TNode **)arena_malloc(arena, capacity * sizeof(TNode *));

    TNode *statement = NULL;
    while ((statement = statement_rule(p)))
    {
        if (length == capacity)
        {
            size_t newCapacity = 2 * capacity;
            TNode **newStatements = (TNode **)arena_realloc(arena, statements, capacity * sizeof(TNode *), newCapacity * sizeof(TNode *));
            HANDLE_NULL_NODE(newStatements);

            statements = newStatements;
            capacity = newCapacity;
        }
        statements[length++] = statement;
    }
    if (length == 0)
    {
        arena_free(arena, statements);
        return NULL;
    }
    TNode *statementsNode = init_array_node(arena, STATEMENT_TYPE, statements, length);
    HANDLE_NULL_NODE(statementsNode)

    flush_used_tokens(p->tokenizer);
    return statementsNode;
}

static TNode *statement_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    int curPos = get_tokenizer_pos(p->tokenizer);

    TNode *statement;
    if ((statement = assign_rule(p)))
    {
        goto done;
    }
    if ((statement = func_run_rule(p)))
    {
        goto done;
    }
    set_tokenizer_pos(p->tokenizer, curPos);
    return NULL;

    DONE_RULE_POSTFIX(statement)
}

static TNode *assign_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    TDataArena *arena = get_parser_arena(p);
    int pos = get_tokenizer_pos(p->tokenizer);

    TNode *assign = NULL;
    TNode *ident, *expr;

    if ((ident = ident_rule(p)) &&
        lookahead(p, ASSIGN) &&
        (expr = expr_rule(p)))
    {
        assign = init_bin_op_node(arena, BIN_OP_ASSIGN, ident, expr);
        HANDLE_NULL_NODE(assign);
        goto done;
    }
    set_tokenizer_pos(p->tokenizer, pos);
    return NULL;

    DONE_RULE_POSTFIX(assign)
}

static TNode *func_run_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;
    TDataArena *arena = get_parser_arena(p);
    int pos = get_tokenizer_pos(p->tokenizer);

    TNode *funcRun = NULL;
    TNode *funcIdent, *args;
    if ((funcIdent = ident_rule(p)) &&
        lookahead(p, LBRACE) &&
        (args = read_args_rule(p)) &&
        lookahead(p, RBRACE))
    {
        TNode *funcRun = init_run_func_node(arena, funcIdent, args);
        HANDLE_NULL_NODE(funcRun);
        goto done;
    }

    set_tokenizer_pos(p->tokenizer, pos);
    return NULL;

    DONE_RULE_POSTFIX(funcRun)
}

static TNode *read_args_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    return NULL;
}

static TNode *expr_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;
    return NULL;
}

static TNode *term_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;
    return NULL;
}

static TNode *factor_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;
    return NULL;
}

static TNode *unsigned_factor_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;
    return NULL;
}

static TNode *pow_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;
    return NULL;
}

static TNode *primary_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;
    return NULL;
}

static TNode *number_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;
    return NULL;
}

static TNode *string_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;
    return NULL;
}

static TNode *ident_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    TDataArena *arena = get_parser_arena(p);
    int pos = get_tokenizer_pos(p->tokenizer);

    TToken token = read_token(p);
    if (!is_ident_token(token))
    {
        set_tokenizer_pos(p->tokenizer, pos);
        return NULL;
    }
    TNode *ident = init_ident_node(arena, token);
    HANDLE_NULL_NODE(ident)

    DONE_RULE_POSTFIX(ident)
}
