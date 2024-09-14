#include "shox.h"

namespace shu::interp
{
    b32 shox_lexer::hadError = false;

    void
    shox_lexer::ReadFile(const char *path)
    {
        platform_read_file_result Result = Platform_ReadFile(path);

        shox_scanner scanner((const char *)Result.Data);
        const shox_token *tokens = scanner.ScanTokens();
        for (i32 i = 0; i < scanner.getTokenCount(); ++i)
        {
            tokens[i].LogString();
        }

        int x = 0;
        Platform_FreeFileMemory(&Result);
    }

    void
    shox_lexer::Error(i32 line, const char *message)
    {
        Report(line, "", message);
    }

    void
    shox_lexer::Report(i32 line, const char *where, const char *message)
    {
        LogError("[line: %d] Error(%s): %s.\n", line, where, message);
        shox_lexer::hadError = true;
    }
}
