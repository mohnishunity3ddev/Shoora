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

    b32 shox_scanner::is_at_end()
    {
        b32 result = current >= StringLen(source) - 1;
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

    void
    shox_scanner::scan_token()
    {

    }

} // namespace shu::interp