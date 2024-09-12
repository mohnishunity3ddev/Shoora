#include <defines.h>
#include <platform/platform.h>

namespace shu::interp
{
    struct shox_lexer
    {
        void read_file(const char *path);

        static void report(i32 line, const char *where, const char *message);
        static void error(i32 line, const char *message);

      private:
        static b32 hadError;
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

    struct shox_token_data
    {
        union
        {
            f64 dData;
            b32 bData;
        };
    };

    struct shox_token
    {
        shox_token() : line(-1), type(shox_token_type::INVALID), lexeme() { data.dData = 0; }

        shox_token(shox_token_type type, const char *lexeme, i32 line);
        shox_token(shox_token_type type, const char *lexeme, const shox_token_data &data, i32 line);

        shox_token_type type;
        char lexeme[128];
        i32 line;
        shox_token_data data;

        char *to_string(char *str);
        void log_string();

      private:
        void token_string(char c[64]);
    };

    struct shox_scanner
    {
      private:
        static const i32 maxTokenCount = 4096;

        char *source;
        i32 start = 0, current = 0, line = 1;
        shox_token tokens[maxTokenCount];
        i32 tokenCount = 0;

        void add_token(const shox_token &token);
        void add_token(shox_token_type type);
        void add_token(shox_token_type type, const char *literal);
        void add_token(shox_token_type type, const char *lexemeStr, const shox_token_data &data);

        // Are we at the end of the string?
        b32 is_at_end();
        // Scan the current potential token in the source string.
        void scan_token();
        // Advance one character in the source string.
        char advance();
        // Does the current unseen character in the source match the character sent in here.
        b32 match(char expected);
        // Just see what the character is and consume it
        char peek();
        // Just see what the character is and consume it
        char peek_next();
        // Process string literals
        void process_string();
        // Process Number Literals
        void process_number();
        void process_identifier();

      public:
        shox_scanner(const char *source);

        const shox_token *scan_tokens();

        void scanner_free();
    };
}