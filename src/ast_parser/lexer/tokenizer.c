#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "system_tools.h"

#include "token_buffer.h"
#include "tokenizer.h"
#include "tokenizer_api.h"

#define is_pass_symbol(ch) ((ch) == '\r' || (ch) == '\t' || (ch) == ' ')
#define is_ident_or_kw_start_symbol(ch) (((ch) >= 'a' && (ch) <= 'z') || ((ch) >= 'A' && (ch) <= 'Z') || (ch) == '_')
#define is_digit(ch) ((ch) >= '0' && (ch) <= '9')
#define is_string_start_symbol(ch) (ch == '"' || ch == '\'' || ch == '`')

static void set_pos_tokenizer_error(TTokenizer *tokenizer, const char *errStart, const char *textMsg, TokenizerErrTypes type)
{
    tokenizer->tokError.textMsg = textMsg;
    tokenizer->tokError.withPos = true;
    tokenizer->tokError.pos = get_error_file_pos(errStart, tokenizer->curLine, tokenizer->lineno, tokenizer->end);

    tokenizer->isError = true;
}

static void set_invalid_token_error(TTokenizer *tokenizer, const char *errStart, const char *textMsg)
{
    set_pos_tokenizer_error(tokenizer, errStart, textMsg, INVALID_TOKEN);
}

static void set_base_tokenizer_error(TTokenizer *tokenizer, char *textMsg, TokenizerErrTypes type)
{
    tokenizer->tokError.textMsg = textMsg;
    tokenizer->tokError.withPos = false;

    tokenizer->isError = true;
}

void set_memory_error(TTokenizer *tokenizer)
{
    set_base_tokenizer_error(tokenizer, "Memory limit exceded", MEMORY_ERROR);
}

static int tgetc(TTokenizer *tokenizer, char *dest)
{
    if (tokenizer->cur >= tokenizer->end)
    {
        *dest = '\0';
        return EOF;
    }
    *dest = *tokenizer->cur;
    tokenizer->cur++;
    if (*dest == '\n')
    {
        tokenizer->lineno++;
        tokenizer->curLine = tokenizer->cur;
    }
    if (*dest == '\0')
    {
        // tokenizer->cur = tokenizer->end;
        return EOF;
    }
    return 0;
}

static const char *_get_line_before(TTokenizer *tokenizer)
{
    assert(tokenizer->lineno != 0);

    const char *cur = tokenizer->cur - 1;
    while (cur >= tokenizer->start)
    {
        if (*cur == '\n')
            return cur + 1;
        cur--;
    }
    return tokenizer->start;
}

static void tbackc(TTokenizer *tokenizer, char ch)
{
    tokenizer->cur--;
    if (tokenizer->cur == tokenizer->curLine - 1)
    {
        tokenizer->curLine = _get_line_before(tokenizer);
        tokenizer->lineno--;
    }
    assert(tokenizer->cur >= tokenizer->start);
    assert(*tokenizer->cur == ch);
}

static void tgets(TTokenizer *tokenizer, const char *s)
{
    char ch;
    while (*s != '\0')
    {
        int r = tgetc(tokenizer, &ch);
        assert(r != EOF && ch == *s);
        s++;
    }
}

static bool lookahead(TTokenizer *tokenizer, const char *check)
{
    bool result = true;

    const char *cur = check;
    char tok_ch;
    while (*cur != '\0')
    {
        int r = tgetc(tokenizer, &tok_ch);
        if (r == EOF || tok_ch != *cur)
        {
            result = false;
            if (r != EOF)
                tbackc(tokenizer, tok_ch);
            break;
        }
        cur++;
    }
    cur--;
    // tgetc(tokenizer, &tok_ch);???

    while (cur >= check)
    {
        tbackc(tokenizer, *cur);
        cur--;
    }
    return result;
}

static bool is_tokenizer_EOF(TTokenizer *tokenizer)
{
    char ch;
    int r = tgetc(tokenizer, &ch);
    if (r == EOF)
        return true;
    tbackc(tokenizer, ch);
    return ch == '\0' ? true : false;
}

static TToken make_EOF_token()
{
    TToken token;
    token.type = EOF_TOKEN;
    token.start = NULL;
    token.end = NULL;

    return token;
}

static TToken make_token(TTokenizer *tokenizer, TokenTypes type)
{
    TToken token;
    token.type = type;
    token.start = tokenizer->start;
    token.end = tokenizer->cur;

    token.col = tokenizer->start - tokenizer->curLine; // need to test
    if (token.col < 0)
        token.col = 0;
    token.lineno = tokenizer->lineno;

    return token;
}

TToken make_error_token(TTokenizer *tokenizer)
{
    return make_token(tokenizer, ERROR_TOKEN);
}

static int read_new_line_indent(TTokenizer *tokenizer)
{
    int whiteSpaceCount = 0;
    char ch;
    while (tgetc(tokenizer, &ch) != EOF)
    {
        if (ch == ' ')
        {
            whiteSpaceCount++;
        }
        else if (ch == '\t')
        {
            whiteSpaceCount += WHITESPACE_IN_TAB;
        }
        else
        {
            tbackc(tokenizer, ch);
            break;
        }
    }
    return whiteSpaceCount;
}

static bool try_to_tgets(TTokenizer *tokenizer, const char *s)
{
    bool isValid = false;

    if (lookahead(tokenizer, s))
    {
        tgets(tokenizer, s);
        isValid = true;
    }
    return isValid;
}
static TToken read_keyword_token(TTokenizer *tokenizer)
{
    char first;
    tgetc(tokenizer, &first);

    TokenTypes type = ERROR_TOKEN;

    switch (first)
    {
    case 'r':
        if (try_to_tgets(tokenizer, "eturn"))
            type = RETURN_KW;
        break;
    case 'i':
        if (try_to_tgets(tokenizer, "f"))
            type = IF_KW;
        else if (try_to_tgets(tokenizer, "mport"))
            type = IMPORT_KW;
        else if (try_to_tgets(tokenizer, "n"))
            type = IN_KW;
        break;
    case 'd':
        if (try_to_tgets(tokenizer, "ef"))
            type = DEF_KW;
        break;
    case 'e':
        if (try_to_tgets(tokenizer, "lse"))
            type = ELSE_KW;
        else if (try_to_tgets(tokenizer, "lif"))
            type = ELIF_KW;
        break;
    case 'f':
        if (try_to_tgets(tokenizer, "or"))
            type = FOR_KW;
        else if (try_to_tgets(tokenizer, "rom"))
            type = FROM_KW;
        break;
    case 'b':
        if (try_to_tgets(tokenizer, "reak"))
            type = BREAK_KW;
        break;
    case 'c':
        if (try_to_tgets(tokenizer, "ontinue"))
            type = CONTINUE_KW;
        break;
    case 'a':
        if (try_to_tgets(tokenizer, "nd"))
            type = AND_KW;
        break;
    case 'o':
        if (try_to_tgets(tokenizer, "r"))
            type = OR_KW;
        break;
    case 'n':
        if (try_to_tgets(tokenizer, "ot"))
            type = NOT_KW;
        break;
    case 'w':
        if (try_to_tgets(tokenizer, "hile"))
            type = WHILE_KW;
        break;
    case 'p':
        if (try_to_tgets(tokenizer, "ass"))
            type = PASS_KW;
        break;
    case 'T':
        if (try_to_tgets(tokenizer, "rue"))
            type = TRUE_KW;
        break;
    case 'F':
        if (try_to_tgets(tokenizer, "alse"))
            type = FALSE_KW;
        break;
    case 'N':
        if (try_to_tgets(tokenizer, "one"))
            type = NONE_KW;
        break;
    }

    if (type == ERROR_TOKEN)
    {
        tbackc(tokenizer, first);
        return make_error_token(tokenizer);
    }

    TToken result = make_token(tokenizer, type);
    return result;
}

static TToken _read_one_char_operation_token(TTokenizer *tokenizer, char ch)
{
    switch (ch)
    {
    case '+':
        return make_token(tokenizer, PLUS);
    case '-':
        return make_token(tokenizer, MINUS);
    case '*':
        return make_token(tokenizer, MULTY);
    case '/':
        return make_token(tokenizer, DIVIS);
    case '%':
        return make_token(tokenizer, PERCENT);
    case '=':
        return make_token(tokenizer, ASSIGN);
    case '<':
        return make_token(tokenizer, LT);
    case '>':
        return make_token(tokenizer, GT);
    case '(':
        return make_token(tokenizer, LPAREN);
    case ')':
        return make_token(tokenizer, RPAREN);
    case '{':
        return make_token(tokenizer, LBRACE);
    case '}':
        return make_token(tokenizer, RBRACE);
    case '[':
        return make_token(tokenizer, LBRACKET);
    case ']':
        return make_token(tokenizer, RBRACKET);
    case ',':
        return make_token(tokenizer, COMMA);
    case '.':
        return make_token(tokenizer, DOT);
    case ':':
        return make_token(tokenizer, COLON);
    default:
        return make_error_token(tokenizer);
    }
}

static TToken _read_two_char_operation_token(TTokenizer *tokenizer, char first, char second)
{
    switch (first)
    {
    case '/':
        if (second == '/')
            return make_token(tokenizer, DOUBLESLASH);
        else if (second == '=')
            return make_token(tokenizer, SLASH_ASSIGN);
        break;

    case '*':
        if (second == '=')
            return make_token(tokenizer, STAR_ASSIGN);
        break;

    case '+':
        if (second == '=')
            return make_token(tokenizer, PLUS_ASSIGN);
        break;

    case '-':
        if (second == '=')
            return make_token(tokenizer, MINUS_ASSIGN);
        break;

    case '%':
        if (second == '=')
            return make_token(tokenizer, PERCENT_ASSIGN);
        break;

    case '=':
        if (second == '=')
            return make_token(tokenizer, EQ);
        break;

    case '!':
        if (second == '=')
            return make_token(tokenizer, NEQ);
        break;

    case '<':
        if (second == '=')
            return make_token(tokenizer, LEQ);
        break;

    case '>':
        if (second == '=')
            return make_token(tokenizer, GEQ);
        break;
    default:
        return make_error_token(tokenizer);
    }
    return make_error_token(tokenizer);
}

static TToken read_operation_token(TTokenizer *tokenizer)
{
    char first, second;
    tgetc(tokenizer, &first);
    int r2 = tgetc(tokenizer, &second);

    if (r2 != EOF)
    {
        TToken result = _read_two_char_operation_token(tokenizer, first, second);
        if (result.type != ERROR_TOKEN)
        {
            return result;
        }
        tbackc(tokenizer, second);
    }

    TToken result = _read_one_char_operation_token(tokenizer, first);
    if (result.type == ERROR_TOKEN)
        tbackc(tokenizer, first);
    return result;
}

static TToken read_ident_token(TTokenizer *tokenizer)
{
    char ch;
    bool isEOF = false;
    do
    {
        int r = tgetc(tokenizer, &ch);
        if (r == EOF)
        {
            isEOF = true;
            break;
        }
    } while (is_ident_or_kw_start_symbol(ch) || is_digit(ch));
    if (!isEOF)
        tbackc(tokenizer, ch);
    return make_token(tokenizer, IDENT);
}

static int _read_digits_part(TTokenizer *tokenizer)
{
    int count = 0;
    char ch;
    while (tgetc(tokenizer, &ch) != EOF)
    {
        if (ch == '_')
            continue;
        if (!is_digit(ch))
        {
            tbackc(tokenizer, ch);
            break;
        }
        count++;
    }
    return count;
}

static TToken read_number(TTokenizer *tokenizer)
{
    _read_digits_part(tokenizer);
    char after;
    int r = tgetc(tokenizer, &after);
    if (r != EOF && after == '.')
    {
        int digits_count = _read_digits_part(tokenizer);
        if (digits_count == 0)
        {
            set_invalid_token_error(tokenizer, tokenizer->cur, "Invalid number fraction");
            return make_error_token(tokenizer);
        }
        r = tgetc(tokenizer, &after);
    }
    if (r == EOF)
        return make_token(tokenizer, NUMBER);

    tbackc(tokenizer, after);
    if (is_ident_or_kw_start_symbol(after))
    {
        set_invalid_token_error(tokenizer, tokenizer->cur, "Unexpected symbol after number");
        return make_error_token(tokenizer);
    }
    return make_token(tokenizer, NUMBER);
}

static TToken read_string_token(TTokenizer *tokenizer)
{
    char leftEdge;
    tgetc(tokenizer, &leftEdge);

    bool isEscaped = false;
    char ch;
    while (tgetc(tokenizer, &ch) != EOF)
    {
        if (ch == '\n')
        {
            tbackc(tokenizer, ch);
            goto unclosed_string_error;
        }
        if (ch == leftEdge && !isEscaped) // right edge == left edge but not \"
        {
            tbackc(tokenizer, ch);
            TToken stringToken = make_token(tokenizer, STRING);
            tgetc(tokenizer, &ch);
            return stringToken;
        }
        // else if (is_string_start_symbol(ch))
        // {
        //     set_invalid_token_error(tokenizer, tokenizer->cur - 1, "Invalid symbol inside string");
        //     return make_error_token(tokenizer);
        // }
        if (ch == '\\')
        {
            isEscaped = !isEscaped;
        }
        else
        {
            isEscaped = false;
        }
    }
unclosed_string_error:
    set_invalid_token_error(tokenizer, tokenizer->cur - 1, "Unclosed string given");
    TToken result = make_error_token(tokenizer);
    return result;
}

static void read_pass_symbols(TTokenizer *tokenizer)
{
    char ch;
    while (tgetc(tokenizer, &ch) != EOF)
    {
        if (!is_pass_symbol(ch))
        {
            tbackc(tokenizer, ch);
            break;
        }
    }
}
static void read_error_chars(TTokenizer *tokenizer)
{
    char ch;
    while (tgetc(tokenizer, &ch) != EOF)
    {
        if (is_ident_or_kw_start_symbol(ch) || is_digit(ch) || is_pass_symbol(ch))
        {
            tbackc(tokenizer, ch);
            break;
        }
    }
}

TToken read_new_token(TTokenizer *tokenizer)
{
    if (is_tokenizer_error(tokenizer))
    {
        return make_error_token(tokenizer);
    }
    if (tokenizer->state == EOF_STATE && tokenizer->curIndent == 0)
    {
        return make_EOF_token();
    }

    int newIndendWhitespaces;
    char ch;
    int r;

restart:
    if (tokenizer->state == NEW_LINE_STATE)
    {
        newIndendWhitespaces = read_new_line_indent(tokenizer); // read cur line intend
        r = tgetc(tokenizer, &ch);
        if (r == EOF)
        {
            goto EOF_state_set;
        }
        if (ch == '\n') // empty line
        {
            goto restart;
        }
        else
        {
            tbackc(tokenizer, ch);
            if (newIndendWhitespaces % WHITESPACE_IN_TAB != 0)
            {
                set_invalid_token_error(tokenizer, tokenizer->cur, "Invalid line indend");
                return make_error_token(tokenizer);
            }
            tokenizer->newIndent = newIndendWhitespaces / WHITESPACE_IN_TAB;
            tokenizer->state = INSIDE_LINE_STATE;
        }
    }
    read_pass_symbols(tokenizer);

EOF_state_set:
    if (is_tokenizer_EOF(tokenizer) && tokenizer->state != EOF_STATE)
    {
        tokenizer->state = EOF_STATE;
        tokenizer->newIndent = 0;
    }

    if (tokenizer->newIndent < tokenizer->curIndent)
    {
        tokenizer->curIndent--;
        return make_token(tokenizer, DEDENT);
    }
    else if (tokenizer->newIndent > tokenizer->curIndent)
    {
        tokenizer->curIndent++;
        return make_token(tokenizer, INDENT);
    }

    if (tokenizer->state == EOF_STATE)
        return make_EOF_token();

    tokenizer->start = tokenizer->cur;
    tgetc(tokenizer, &ch);
    // if (r == EOF || ch == '\0')
    //     return make_EOF_token();
    if (ch == '\n')
    {
        tokenizer->state = NEW_LINE_STATE;
        return make_token(tokenizer, NEWLINE);
    }
    tbackc(tokenizer, ch);

    if (is_ident_or_kw_start_symbol(ch))
    {
        TToken keywordToken = read_keyword_token(tokenizer);
        if (keywordToken.type != ERROR_TOKEN)
        {
            return keywordToken;
        }
        TToken identToken = read_ident_token(tokenizer);
        return identToken;
    }
    if (is_digit(ch))
    {
        TToken numberToken = read_number(tokenizer);
        return numberToken;
    }
    if (is_string_start_symbol(ch))
    {
        TToken stringToken = read_string_token(tokenizer);
        return stringToken;
    }

    TToken opToken = read_operation_token(tokenizer);
    if (opToken.type != ERROR_TOKEN)
        return opToken;

    set_invalid_token_error(tokenizer, tokenizer->cur, "Unexpected operation");
    read_error_chars(tokenizer);
    return make_error_token(tokenizer);
}
