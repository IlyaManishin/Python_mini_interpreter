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

#define HANDLE_INVALID_RULE(msg) //???

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
        funcRun = init_run_func_node(arena, funcIdent, args);
        HANDLE_NULL_NODE(funcRun);
        goto done;
    }

    set_tokenizer_pos(p->tokenizer, pos);
    return NULL;

    DONE_RULE_POSTFIX(funcRun)
}

/*Args: list of expr*/
static TNode *read_args_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    TDataArena *arena = get_parser_arena(p);
    TNode *res = NULL;

    // If next is RBRACE -> empty args (do not consume RBRACE here)
    if (check_next_token(p, RBRACE))
    {
        res = init_empty_array_node(arena, EXPRESSION_TYPE);
        HANDLE_NULL_NODE(res);
        goto done;
    }

    size_t length = 0;
    size_t capacity = BASE_ARGS_LIST_SIZE;
    TNode **args = (TNode **)arena_malloc(arena, capacity * sizeof(TNode *));
    if (args == NULL)
    {
        set_memory_crit_error(p);
        return NULL;
    }

    TNode *arg = NULL;
    while ((arg = expr_rule(p)))
    {
        if (length == capacity)
        {
            size_t newCapacity = 2 * capacity;
            TNode **newArgs = (TNode **)arena_realloc(arena, args, capacity * sizeof(TNode *), newCapacity * sizeof(TNode *));
            HANDLE_NULL_NODE(newArgs);
            args = newArgs;
            capacity = newCapacity;
        }
        args[length++] = arg;

        if (!lookahead(p, COMMA))
            break;
    }

    res = init_array_node(arena, EXPRESSION_TYPE, args, length);
    HANDLE_NULL_NODE(res);

    DONE_RULE_POSTFIX(res)
}

static TNode *expr_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    TDataArena *arena = get_parser_arena(p);

    TNode *left = term_rule(p);
    if (left == NULL)
        return NULL;

    while (1)
    {
        int pos = get_tokenizer_pos(p->tokenizer);
        if (lookahead(p, PLUS))
        {
            TNode *right = term_rule(p);
            if (right == NULL)
            {
                set_tokenizer_pos(p->tokenizer, pos);
                break;
            }
            left = init_bin_op_node(arena, BIN_OP_SUM, left, right);
            HANDLE_NULL_NODE(left);
        }
        else if (lookahead(p, MINUS))
        {
            TNode *right = term_rule(p);
            if (right == NULL)
            {
                set_tokenizer_pos(p->tokenizer, pos);
                break;
            }
            left = init_bin_op_node(arena, BIN_OP_SUB, left, right);
            HANDLE_NULL_NODE(left);
        }
        else
        {
            break;
        }
    }

    DONE_RULE_POSTFIX(left)
}

static TNode *term_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    TDataArena *arena = get_parser_arena(p);

    TNode *left = factor_rule(p);
    if (left == NULL)
        return NULL;

    while (1)
    {
        int pos = get_tokenizer_pos(p->tokenizer);
        if (lookahead(p, MULTY))
        {
            TNode *right = factor_rule(p);
            if (right == NULL)
            {
                set_tokenizer_pos(p->tokenizer, pos);
                break;
            }
            left = init_bin_op_node(arena, BIN_OP_MUL, left, right);
            HANDLE_NULL_NODE(left);
        }
        else if (lookahead(p, DIVIS))
        {
            TNode *right = factor_rule(p);
            if (right == NULL)
            {
                set_tokenizer_pos(p->tokenizer, pos);
                break;
            }
            left = init_bin_op_node(arena, BIN_OP_DIV, left, right);
            HANDLE_NULL_NODE(left);
        }
        else
        {
            break;
        }
    }

    DONE_RULE_POSTFIX(left)
}

static TNode *factor_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    TDataArena *arena = get_parser_arena(p);
    int pos = get_tokenizer_pos(p->tokenizer);

    TNode *res = NULL;

    if (lookahead(p, PLUS))
    {
        TNode *res = factor_rule(p);
        if (res == NULL)
        {
            // handle error
        }
        goto done;
    }

    set_tokenizer_pos(p->tokenizer, pos);
    if (lookahead(p, MINUS))
    {
        TNode *factor = factor_rule(p);
        if (factor == NULL)
        {
            // handle error
        }

        TNode *res = init_unary_op_node(arena, UNO_MINUS, factor);
        HANDLE_NULL_NODE(res);
        goto done;
    }

    set_tokenizer_pos(p->tokenizer, pos);
    if ((res = unsigned_factor_rule(p)))
    {
        goto done;
    }
    // handle error
    return NULL;

    DONE_RULE_POSTFIX(res)
}

static TNode *unsigned_factor_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    return pow_rule(p);
}

static TNode *pow_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    TDataArena *arena = get_parser_arena(p);

    TNode *left = primary_rule(p);
    if (left == NULL)
        return NULL;

    // right-associative: a ^ b ^ c => a ^ (b ^ c)
    int pos = get_tokenizer_pos(p->tokenizer);
    if (lookahead(p, MULTY) && lookahead(p, MULTY))
    {
        TNode *right = pow_rule(p);
        if (right == NULL)
        {
            set_tokenizer_pos(p->tokenizer, pos);
            return left;
        }
        TNode *node = init_bin_op_node(arena, BIN_OP_POW, left, right);
        HANDLE_NULL_NODE(node);
        goto done;
    }
    return NULL;

    DONE_RULE_POSTFIX(left)
}

static TNode *primary_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    int pos = get_tokenizer_pos(p->tokenizer);

    // parenthesized expression
    TNode *res = NULL;
    if (lookahead(p, LBRACE) &&
        (res = expr_rule(p)) &&
        lookahead(p, RBRACE))
    {
        goto done;
    }
    set_tokenizer_pos(p->tokenizer, pos);

    if ((res = number_rule(p)))
        goto done;

    if ((res = string_rule(p)))
        goto done;

    if ((res = ident_rule(p)))
        goto done;

    DONE_RULE_POSTFIX(res)
}

static TNode *number_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;

    TDataArena *arena = get_parser_arena(p);
    int pos = get_tokenizer_pos(p->tokenizer);

    TToken token = read_token(p);
    if (token.type != NUMBER)
    {
        set_tokenizer_pos(p->tokenizer, pos);
        return NULL;
    }
    TNode *res = init_number_node(arena, token);
    HANDLE_NULL_NODE(res)

    DONE_RULE_POSTFIX(res)
}

static TNode *string_rule(TAstParser *p)
{
    if (is_critical_error(p))
        return NULL;
    TDataArena *arena = get_parser_arena(p);
    int pos = get_tokenizer_pos(p->tokenizer);

    TToken token = read_token(p);
    if (token.type != STRING)
    {
        set_tokenizer_pos(p->tokenizer, pos);
        return NULL;
    }
    TNode *res = init_string_node(arena, token);
    HANDLE_NULL_NODE(res)

    DONE_RULE_POSTFIX(res)
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
