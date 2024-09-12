#include "shox.h"

namespace shu::interp
{
    b32 shox_lexer::hadError = false;

    void
    shox_lexer::read_file(const char *path)
    {
        platform_read_file_result Result = Platform_ReadFile(path);
        
        shox_token s = shox_token(shox_token_type::LEFT_BRACE, "My Literal", 1244);

        s.log_string();
        int x = 0;
    }

    void
    shox_lexer::error(i32 line, const char *message)
    {
        report(line, "", message);
    }

    void
    shox_lexer::report(i32 line, const char *where, const char *message)
    {
        LogError("[line: %d] Error(%s): %s.\n", line, where, message);

        shox_lexer::hadError = true;
    }
}
