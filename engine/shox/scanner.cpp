#include "shox.h"

namespace shu::interp
{
    shox_scanner::shox_scanner(const char *source)
    {
        i32 size = ALIGN32(StringLen(source));
        this->source = (char *)malloc(size);
        SHU_MEMCOPY(source, this->source, size);
    }

    void
    shox_scanner::scanner_free()
    {
        if (this->source != nullptr)
        {
            free(this->source);
            this->source = nullptr;
        }
    }

    void
    shox_scanner::add_token(const shox_token &token)
    {
        ASSERT(tokenCount + 1 <= maxTokenCount);
        this->tokens[tokenCount++] = token;
    }

    void
    shox_scanner::add_token(shox_token_type type)
    {
        char temp[128];
        SubString(this->source, this->start, this->current, temp);

        shox_token token(type, temp, this->line);
        this->add_token(token);
    }

    void
    shox_scanner::add_token(shox_token_type type, const char *lexemeStr)
    {
        shox_token token(type, lexemeStr, this->line);
        this->add_token(token);
    }

    void
    shox_scanner::add_token(shox_token_type type, const char *lexemeStr, const shox_token_data &data)
    {
        shox_token token(type, lexemeStr, data, this->line);
        this->add_token(token);
    }

    b32 shox_scanner::is_at_end()
    {
        b32 result = current >= StringLen(source);
        return result;
    }

    const shox_token *
    shox_scanner::scan_tokens()
    {
        while(!is_at_end())
        {
            start = current;
            scan_token();
        }

        add_token(shox_token(shox_token_type::END_OF_FILE, "", line));
        return tokens;
    }

    char
    shox_scanner::advance()
    {
        char result = this->source[this->current++];
        return result;
    }

    b32
    shox_scanner::match(char expected)
    {
        if (this->is_at_end()) return false;
        if (CharAt(this->source, this->current) != expected) return false;

        ++this->current;
        return true;
    }

    char
    shox_scanner::peek()
    {
        char result;
        if (is_at_end())
            result = '\0';
        else
            result = CharAt(this->source, this->current);

        return result;
    }

    char
    shox_scanner::peek_next()
    {
        char result;
        if ((current + 1) >= StringLen(source))
            result = '\0';
        else
            result = CharAt(this->source, this->current + 1);

        return result;
    }

    void
    shox_scanner::process_string()
    {
        while (this->peek() != '"' && !this->is_at_end())
        {
            if (peek() == '\n') ++this->line;
            this->advance();
        }

        if (is_at_end())
        {
            shox_lexer::error(line, "Unterminated string.");
            return;
        }

        // consume the closing " of the string.
        advance();

        // Trim the surrounding quotes.
        char string[128];
        SubString(this->source, start + 1, current - 1, string);

        this->add_token(shox_token_type::STRING, string);
    }

    void
    shox_scanner::process_number()
    {
        // Keep advancing till you see a number
        while ( IsDigit(peek()) )
            advance();

        // Is this a decimal number
        if (peek() == '.' && IsDigit(peek_next()))
        {
            // consume the . (decimal point)
            advance();

            // Consume the rest of the number
            while (IsDigit(peek()))
                advance();
        }

        char numStr[128];
        SubString(this->source, start, current, numStr);
        shox_token_data data = {.dData = StringToDouble(numStr)};
        this->add_token(shox_token_type::NUMBER, numStr, data);
    }

    void
    shox_scanner::process_identifier()
    {
        while (IsAlphaNumeric(peek()))
            advance();

        
        shox_token_type type;
        this->add_token(type);
    }

    void
    shox_scanner::scan_token()
    {
        char c = advance();
        switch(c)
        {
            case '(': { this->add_token(shox_token_type::LEFT_PAREN); } break;
            case ')': { this->add_token(shox_token_type::RIGHT_PAREN); } break;
            case '{': { this->add_token(shox_token_type::LEFT_BRACE); } break;
            case '}': { this->add_token(shox_token_type::RIGHT_BRACE); } break;
            case ',': { this->add_token(shox_token_type::COMMA); } break;
            case '.': { this->add_token(shox_token_type::DOT); } break;
            case '-': { this->add_token(shox_token_type::MINUS); } break;
            case '+': { this->add_token(shox_token_type::PLUS); } break;
            case ';': { this->add_token(shox_token_type::SEMICOLON); } break;
            case '*': { this->add_token(shox_token_type::STAR); } break;

            case '!': { this->add_token(match('=') ? BANG_EQUAL : BANG); } break;
            case '=': { this->add_token(match('=') ? EQUAL_EQUAL : EQUAL); } break;
            case '<': { this->add_token(match('=') ? LESS_EQUAL : LESS); } break;
            case '>': { this->add_token(match('=') ? GREATER_EQUAL : GREATER); } break;

            case '/':
            {
                // Is it a single line comment?
                if (match('/'))
                {
                    // A comment goes till the end of the current line.
                    while(peek() != '\n' && !is_at_end())
                    {
                        // Just advance without processing the character returned by advance.
                        advance();
                    }
                }
                else
                {
                    add_token(shox_token_type::SLASH);
                }
            } break;

            // Ignore whitespaces
            case ' ':
            case '\r':
            case '\t': break;
            case '\n': { this->line++; } break;

            // Strings
            case '"': { process_string(); } break;

            // reserved identifiers
            case 'o':
            {
                if (match('r'))
                {
                    this->add_token(shox_token_type::OR);
                }
            }

            default:
            {
                if (IsDigit(c))
                {
                    process_number();
                }
                else if (IsAlpha(c))
                {
                    process_identifier();
                }
                else
                {
                    shox_lexer::error(this->line, "Unexpected Character.");
                }
            } break;
        }
    }

} // namespace shu::interp