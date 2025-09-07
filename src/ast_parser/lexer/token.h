#pragma once

typedef enum TokenTypes
{
    ERROR_TOKEN, // on error

    IDENT,
    NUMBER,
    STRING,

    RETURN_KW,   // return
    IF_KW,       // if
    ELSE_KW,     // else
    ELIF_KW,     // elif
    FOR_KW,      // for
    BREAK_KW,    // break
    CONTINUE_KW, // continue
    TRUE_KW,     // True
    NONE_KW,     // None
    FALSE_KW,    // False
    AND_KW,      // and
    OR_KW,       // or
    NOT_KW,      // not

    WHILE_KW, // while
    IN_KW,    // in
    PASS_KW,  // pass
    DEF_KW,   // def
    // CLASS_KW,  // class
    IMPORT_KW, // import
    FROM_KW,   // from
    // AS_KW,     // as

    PLUS,           // +
    MINUS,          // -
    MULTY,          // *
    DIVIS,          // /
    POW,            // **
    DOUBLESLASH,    // //
    PERCENT,        // %
    EQ,             // ==
    NEQ,            // !=
    LT,             // <
    GT,             // >
    LEQ,            // <=
    GEQ,            // >=
    ASSIGN,         // =
    PLUS_ASSIGN,    // +=
    MINUS_ASSIGN,   // -=
    STAR_ASSIGN,    // *=
    SLASH_ASSIGN,   // /=
    PERCENT_ASSIGN, // %=

    LPAREN,   // (
    RPAREN,   // )
    LBRACE,   // {
    RBRACE,   // }
    LBRACKET, // [
    RBRACKET, // ]
    COMMA,    // ,
    DOT,      // .
    COLON,    // :

    INDENT,
    DEDENT,
    NEWLINE,
    EOF_TOKEN
} TokenTypes;

typedef struct TToken
{
    TokenTypes type;

    const char *start;
    const char *end; // for ident only?
    int lineno;
    int col;
} TToken;