#pragma once

#include "ast_parser_api.h"
#include "data_arena.h"

#include "../lexer/tokenizer_api.h"
#include "../parser.h"

TNode *file_rule(TAstParser *p);

bool lookahead(TAstParser *p, TokenTypes checkType);
bool check_next_token(TAstParser *p, TokenTypes type);

TNode *init_array_node(TDataArena *arena, NodeTypes itemType, TNode** nodes, int length);
TNode *init_empty_array_node(TDataArena *arena, NodeTypes itemType);

TNode *init_unary_op_node(TDataArena *arena, UnOpTypes opType, TNode* child);
TNode *init_bin_op_node(TDataArena *arena, BinOpTypes opType, TNode *left, TNode *right);
TNode *init_run_func_node(TDataArena *arena, TNode *funcIdent, TNode *args);

TNode *init_string_node(TDataArena *arena, TToken identToken);
TNode *init_number_node(TDataArena *arena, TToken identToken);
TNode *init_ident_node(TDataArena *arena, TToken identToken);

TToken read_token(TAstParser *p);
bool is_bool_ident(TToken ident);
bool is_ident_token(TToken token);