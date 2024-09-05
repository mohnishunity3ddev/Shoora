#include <defines.h>
#include <platform/platform.h>
#include <stdio.h>

namespace shu::interp
{
    struct shox_lexer
    {
        b32 hadError = false;
        void read_file(const char *path);
        void error(i32 line, const char *message);
        void report(i32 line, const char *where, const char *message);
    };

    enum shox_token_type
    {
        INVALID,

        LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
        COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

        BANG, BANG_EQUAL,
        EQUAL, EQUAL_EQUAL,
        GREATER, GREATER_EQUAL,
        LESS, LESS_EQUAL,

        IDENTIFIER, STRING, NUMBER,

        AND, CLASS, ELSE, _FALSE, FUN, FOR, IF, NIL, OR,
        PRINT, RETURN, SUPER, THIS, _TRUE, VAR, WHILE,

        END_OF_FILE
    };

    struct shox_token
    {
        shox_token_type type;
        char lexeme[128];
        i32 line;
        shox_token() : line(-1), type(shox_token_type::INVALID), lexeme() {}
        shox_token(shox_token_type type, const char *lexeme, i32 line);
        char *to_string();

      private:
        void token_string(char c[64]);
    };

    struct shox_scanner
    {
      private:
        static const i32 maxTokenCount = 2048;

        char *source;

        i32 start = 0, current = 0, line = 1;

        shox_token tokens[maxTokenCount];
        i32 tokenCount = 0;
        void add_token(const shox_token &token);
        b32 is_at_end();
        void scan_token();

      public:
        shox_scanner(const char *source);

        const shox_token *scan_tokens();

        void scanner_free();
    };
}