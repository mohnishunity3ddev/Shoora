#include <defines.h>
#include <platform/platform.h>

namespace shu::interp
{
    struct shox_lexer
    {
        void ReadFile(const char *path);

        static void Report(i32 line, const char *where, const char *message);
        static void Error(i32 line, const char *message);

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

        char *ToString(char *str);
        void LogString() const noexcept;

      private:
        void TokenString(char c[64]) const noexcept;
    };

    struct shox_scanner
    {
      private:
        static const i32 maxTokenCount = 4096;

        char *source;
        i32 start = 0, current = 0, line = 1;
        shox_token tokens[maxTokenCount];
        i32 tokenCount = 0;
        b32 MultiLineCommentEncountered = false;

        void AddToken(const shox_token &token);
        void AddToken(shox_token_type type);
        void AddToken(shox_token_type type, const char *literal);
        void AddToken(shox_token_type type, const char *lexemeStr, const shox_token_data &data);

        // Are we at the end of the string?
        b32 IsAtEnd();
        // Scan the current potential token in the source string.
        void ScanToken();
        // Advance one character in the source string.
        char Advance();
        // Does the current unseen character in the source match the character sent in here.
        b32 DoesCurrentMatch(char expected);
        // Just see what the character is at the current pointer without consuming it.
        char Peek();
        // Just see what the character is at the current + 1 position without consuming it.
        char PeekNext();
        // Process string literals
        void ProcessString();
        // Process Number Literals
        void ProcessNumber();
        void ProcessIdentifierOrReserved();

      public:
        shox_scanner(const char *source);

        const shox_token *ScanTokens();
        i32 getTokenCount() { return tokenCount; }
    };
}