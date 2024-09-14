#include "shox.h"

namespace shu::interp
{
    static shox_token_type
    GetTokenType(const char *TokenString)
    {
        shox_token_type Result = shox_token_type::INVALID;

        if      (StringCompare(TokenString, "and"))    Result = shox_token_type::AND;
        else if (StringCompare(TokenString, "class"))  Result = shox_token_type::CLASS;
        else if (StringCompare(TokenString, "else"))   Result = shox_token_type::ELSE;
        else if (StringCompare(TokenString, "false"))  Result = shox_token_type::_FALSE;
        else if (StringCompare(TokenString, "for"))    Result = shox_token_type::FOR;
        else if (StringCompare(TokenString, "fun"))    Result = shox_token_type::FUN;
        else if (StringCompare(TokenString, "if"))     Result = shox_token_type::IF;
        else if (StringCompare(TokenString, "nil"))    Result = shox_token_type::NIL;
        else if (StringCompare(TokenString, "or"))     Result = shox_token_type::OR;
        else if (StringCompare(TokenString, "print"))  Result = shox_token_type::PRINT;
        else if (StringCompare(TokenString, "return")) Result = shox_token_type::RETURN;
        else if (StringCompare(TokenString, "super"))  Result = shox_token_type::SUPER;
        else if (StringCompare(TokenString, "this"))   Result = shox_token_type::THIS;
        else if (StringCompare(TokenString, "true"))   Result = shox_token_type::_TRUE;
        else if (StringCompare(TokenString, "var"))    Result = shox_token_type::VAR;
        else if (StringCompare(TokenString, "while"))  Result = shox_token_type::WHILE;

        return Result;
    }

    shox_scanner::shox_scanner(const char *source)
    {
        this->source = (char *)source;
        ASSERT(this->source != nullptr);
    }

    void
    shox_scanner::AddToken(const shox_token &token)
    {
        ASSERT(tokenCount + 1 <= maxTokenCount);
        this->tokens[tokenCount++] = token;
    }

    void
    shox_scanner::AddToken(shox_token_type type)
    {
        char temp[128];
        SubString(this->source, this->start, this->current, temp);

        shox_token token(type, temp, this->line);
        this->AddToken(token);
    }

    void
    shox_scanner::AddToken(shox_token_type type, const char *lexemeStr)
    {
        shox_token token(type, lexemeStr, this->line);
        this->AddToken(token);
    }

    void
    shox_scanner::AddToken(shox_token_type type, const char *lexemeStr, const shox_token_data &data)
    {
        shox_token token(type, lexemeStr, data, this->line);
        this->AddToken(token);
    }

    b32 shox_scanner::IsAtEnd()
    {
        b32 result = current >= StringLen(source);
        return result;
    }

    char
    shox_scanner::Advance()
    {
        char result = this->source[this->current++];
        return result;
    }

    b32
    shox_scanner::Match(char expected)
    {
        if (this->IsAtEnd()) return false;
        if (CharAt(this->source, this->current) != expected) return false;

        ++this->current;
        return true;
    }

    char
    shox_scanner::Peek()
    {
        char result;
        if (IsAtEnd())
            result = '\0';
        else
            result = CharAt(this->source, this->current);

        return result;
    }

    char
    shox_scanner::PeekNext()
    {
        char result;
        if ((current + 1) >= StringLen(source))
            result = '\0';
        else
            result = CharAt(this->source, this->current + 1);

        return result;
    }

    void
    shox_scanner::ProcessString()
    {
        while (this->Peek() != '"' && !this->IsAtEnd())
        {
            if (Peek() == '\n') ++this->line;
            this->Advance();
        }

        if (IsAtEnd())
        {
            shox_lexer::Error(line, "Unterminated string.");
            return;
        }

        // consume the closing " of the string.
        Advance();

        // Trim the surrounding quotes.
        char string[128];
        // TODO: Don't need to create new memory to store this. I can just startIndex and endIndex which represents
        // the start and end of the lexeme directly in the source file.
        SubString(this->source, start + 1, current - 1, string);

        this->AddToken(shox_token_type::STRING, string);
    }

    void
    shox_scanner::ProcessNumber()
    {
        // Keep advancing till you see a number
        while ( IsDigit(Peek()) )
            Advance();

        // Is this a decimal number
        if (Peek() == '.' && IsDigit(PeekNext()))
        {
            // consume the . (decimal point)
            Advance();

            // Consume the rest of the number
            while (IsDigit(Peek()))
                Advance();
        }

        char numStr[128];
        // TODO: Don't need to create new memory to store this. I can just startIndex and endIndex which represents
        // the start and end of the lexeme directly in the source file.
        SubString(this->source, start, current, numStr);
        shox_token_data data = {.dData = StringToDouble(numStr)};
        this->AddToken(shox_token_type::NUMBER, numStr, data);
    }

    void
    shox_scanner::ProcessIdentifierOrReserved()
    {
        while (IsAlphaNumeric(Peek()))
            Advance();


        char t[128];
        SubString(source, start, current, t);

        shox_token_type type = GetTokenType(t);
        if (type == INVALID)
            type = shox_token_type::IDENTIFIER;

        this->AddToken(type);
    }

    const shox_token *
    shox_scanner::ScanTokens()
    {
        while (!IsAtEnd())
        {
            start = current;
            ScanToken();
        }

        AddToken(shox_token(shox_token_type::END_OF_FILE, "", line));
        return tokens;
    }

    void
    shox_scanner::ScanToken()
    {
        char c = Advance();
        switch(c)
        {
            case '(': { this->AddToken(shox_token_type::LEFT_PAREN); } break;
            case ')': { this->AddToken(shox_token_type::RIGHT_PAREN); } break;
            case '{': { this->AddToken(shox_token_type::LEFT_BRACE); } break;
            case '}': { this->AddToken(shox_token_type::RIGHT_BRACE); } break;
            case ',': { this->AddToken(shox_token_type::COMMA); } break;
            case '.': { this->AddToken(shox_token_type::DOT); } break;
            case '-': { this->AddToken(shox_token_type::MINUS); } break;
            case '+': { this->AddToken(shox_token_type::PLUS); } break;
            case ';': { this->AddToken(shox_token_type::SEMICOLON); } break;
            case '*': { this->AddToken(shox_token_type::STAR); } break;

            case '!': { this->AddToken(Match('=') ? BANG_EQUAL : BANG); } break;
            case '=': { this->AddToken(Match('=') ? EQUAL_EQUAL : EQUAL); } break;
            case '<': { this->AddToken(Match('=') ? LESS_EQUAL : LESS); } break;
            case '>': { this->AddToken(Match('=') ? GREATER_EQUAL : GREATER); } break;

            case '/':
            {
                // Is it a single line comment?
                if (Match('/'))
                {
                    // A comment goes till the end of the current line.
                    while(Peek() != '\n' && !IsAtEnd())
                    {
                        // Just advance without processing the character returned by advance.
                        Advance();
                    }
                }
                else
                {
                    AddToken(shox_token_type::SLASH);
                }
            } break;

            // Ignore whitespaces
            case ' ':
            case '\r':
            case '\t': break;
            case '\n': { this->line++; } break;

            // Strings
            case '"': { ProcessString(); } break;

            // reserved identifiers
            case 'o':
            {
                if (Match('r'))
                {
                    this->AddToken(shox_token_type::OR);
                }
            }

            default:
            {
                if (IsDigit(c))
                {
                    ProcessNumber();
                }
                else if (IsAlpha(c))
                {
                    ProcessIdentifierOrReserved();
                }
                else
                {
                    shox_lexer::Error(this->line, "Unexpected Character.");
                }
            } break;
        }
    }
} // namespace shu::interp