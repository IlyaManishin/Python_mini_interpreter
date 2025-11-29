#include <assert.h>
#include <string.h>

#include "token_buffer.h"
#include "tokenizer.h"
#include "tokenizer_api.h"

TTokenizer *tokenizer_from_file_data(TFileData fileData)
{
    assert(fileData.str != NULL);

    TTokenBuffer *tokBuffer = get_token_buf();
    if (tokBuffer == NULL)
        return NULL;

    TTokenizer *tokenizer = (TTokenizer *)malloc(sizeof(TTokenizer));
    if (tokenizer == NULL)
    {
        delete_token_buf(tokBuffer);
        return NULL;
    }

    tokenizer->start = fileData.str;
    tokenizer->cur = fileData.str;
    tokenizer->curLine = fileData.str;
    tokenizer->lineno = 0;
    tokenizer->end = fileData.str + fileData.dataSize;

    tokenizer->state = NEW_LINE_STATE;
    tokenizer->newIndent = 0;
    tokenizer->curIndent = 0;

    tokenizer->curBufPos = 0;
    tokenizer->tokensBuf = tokBuffer;

    tokenizer->isError = false;
    tokenizer->tokError.textMsg = NULL;

    return tokenizer;
}

void delete_tokenizer(TTokenizer *tokenizer)
{
    free(tokenizer->tokensBuf);
    free(tokenizer);
}

bool is_tokenizer_error(TTokenizer *tokenizer)
{
    return tokenizer->isError;
}

void pass_tokenizer_error(TTokenizer *tokenizer)
{
    tokenizer->isError = false;
}

TTokenizerError get_tokenizer_error(TTokenizer *tokenizer)
{
    return tokenizer->tokError;
}

size_t token_strlen(TToken token)
{
    if (token.end <= token.start)
        return 0;
    size_t res = token.end - token.start;
    return res;
}

bool check_token_str(TToken token, const char *str)
{
    size_t tokenLength = token_strlen(token);
    size_t strLength = nsu_strnlen(str, tokenLength + 1);
    if (strLength != tokenLength)
        return false;

    return strncmp(token.start, str, strLength) == 0;
}

TToken token_soft_read(TTokenizer *tokenizer)
{
    TToken token;
    if (get_token_from_buf(tokenizer->tokensBuf, &token, tokenizer->curBufPos))
    {
        tokenizer->curBufPos++;
        return token;
    }

    token = read_new_token(tokenizer);
    if (token.type == EOF_TOKEN || token.type == ERROR_TOKEN)
    {
        return token;
    }

    // append only correct tokens
    int r = append_token_to_buf(tokenizer->tokensBuf, token);
    tokenizer->curBufPos++;
    if (!r)
    {
        set_memory_error(tokenizer);
        TToken errorToken = make_error_token(tokenizer);
        return errorToken;
    }
    return token;
}

/**clear used tokens between left and curBufPos forever */
void flush_used_tokens(TTokenizer *tokenizer)
{
    pop_tokens_from_buf(tokenizer->tokensBuf, tokenizer->curBufPos);
    rewind_tokenizer_pos(tokenizer);
}

// TToken strong_token_read(TTokenizer *tokenizer)
// {
//     TToken token;
//     if (pop_token_from_buf(tokenizer->tokensBuf, &token))
//     {
//         return token;
//     }
//     token = read_new_token(tokenizer);
//     return token;
// }

int get_tokenizer_pos(TTokenizer *tokenizer)
{
    return tokenizer->curBufPos;
}

void set_tokenizer_pos(TTokenizer *tokenizer, int pos)
{
    assert(0 <= pos && pos <= get_buf_length(tokenizer->tokensBuf));
    tokenizer->curBufPos = pos;
}

int rewind_tokenizer_pos(TTokenizer *tokenizer)
{
    tokenizer->curBufPos = 0;
    return 0;
}
