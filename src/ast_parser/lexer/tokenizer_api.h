#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "syntax_errors.h"
#include "system_tools.h"

#include "token.h"

typedef struct TTokenizer TTokenizer;

TTokenizer *tokenizer_from_file_data(TFileData fileData);
void delete_tokenizer(TTokenizer *tokenizer);

bool is_tokenizer_error(TTokenizer *tokenizer);
TTokenizerError get_tokenizer_error(TTokenizer *tokenizer);
void pass_tokenizer_error(TTokenizer *tokenizer);

TToken token_soft_read(TTokenizer *tokenizer);
void flush_used_tokens(TTokenizer *tokenizer);

int get_tokenizer_pos(TTokenizer *tokenizer);
void set_tokenizer_pos(TTokenizer *tokenizer, int pos);
int rewind_tokenizer_pos(TTokenizer *tokenizer);

size_t token_length(TToken token);
bool check_token_str(TToken token, const char *str);