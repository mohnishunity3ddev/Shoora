#include "shox.h"

namespace shu::interp
{
    shox_token::shox_token(shox_token_type type, const char *lexeme, i32 line)
    {
        this->type = type;
        this->line = line;
        SHU_MEMCOPY(lexeme, this->lexeme, 128);
    }

    char *
    shox_token::to_string()
    {
        char Buffer[4096];
        const char *Format = "Type: %d(%s);  Lexeme: %s.\n";
        char t[64];
        token_string(t);
        i32 len = Platform_GenerateString(Buffer, ARRAY_SIZE(Buffer), Format, type, t, lexeme);

        i32 size = ALIGN32(len);
        void *memory = malloc(size);
        SHU_MEMCOPY(Buffer, memory, len);
        return (char *)memory;
    }

    void
    shox_token::token_string(char c[64])
    {
        switch(type)
        {
            case INVALID:       { SHU_MEMCOPY("INVALID", c, StringLen("INVALID")); } break;
            case LEFT_PAREN:    { SHU_MEMCOPY("LEFT_PAREN", c, StringLen("LEFT_PAREN")); } break;
            case RIGHT_PAREN:   { SHU_MEMCOPY("RIGHT_PAREN", c, StringLen("RIGHT_PAREN"));} break;
            case LEFT_BRACE:    { SHU_MEMCOPY("LEFT_BRACE", c, StringLen("LEFT_BRACE"));} break;
            case RIGHT_BRACE:   { SHU_MEMCOPY("RIGHT_BRACE", c, StringLen("RIGHT_BRACE"));} break;
            case COMMA:         { SHU_MEMCOPY("COMMA", c, StringLen("COMMA"));} break;
            case DOT:           { SHU_MEMCOPY("DOT", c, StringLen("DOT"));} break;
            case MINUS:         { SHU_MEMCOPY("MINUS", c, StringLen("MINUS"));} break;
            case PLUS:          { SHU_MEMCOPY("PLUS", c, StringLen("PLUS"));} break;
            case SEMICOLON:     { SHU_MEMCOPY("SEMICOLON", c, StringLen("SEMICOLON"));} break;
            case SLASH:         { SHU_MEMCOPY("SLASH", c, StringLen("SLASH"));} break;
            case STAR:          { SHU_MEMCOPY("STAR", c, StringLen("STAR"));} break;
            case BANG:          { SHU_MEMCOPY("BANG", c, StringLen("BANG"));} break;
            case BANG_EQUAL:    { SHU_MEMCOPY("BANG_EQUAL", c, StringLen("BANG_EQUAL"));} break;
            case EQUAL:         { SHU_MEMCOPY("EQUAL", c, StringLen("EQUAL"));} break;
            case EQUAL_EQUAL:   { SHU_MEMCOPY("EQUAL_EQUAL", c, StringLen("EQUAL_EQUAL"));} break;
            case GREATER:       { SHU_MEMCOPY("GREATER", c, StringLen("GREATER"));} break;
            case GREATER_EQUAL: { SHU_MEMCOPY("GREATER_EQUAL", c, StringLen("GREATER_EQUAL"));} break;
            case LESS:          { SHU_MEMCOPY("LESS", c, StringLen("LESS"));} break;
            case LESS_EQUAL:    { SHU_MEMCOPY("LESS_EQUAL", c, StringLen("LESS_EQUAL"));} break;
            case IDENTIFIER:    { SHU_MEMCOPY("IDENTIFIER", c, StringLen("IDENTIFIER"));} break;
            case STRING:        { SHU_MEMCOPY("STRING", c, StringLen("STRING"));} break;
            case NUMBER:        { SHU_MEMCOPY("NUMBER", c, StringLen("NUMBER"));} break;
            case AND:           { SHU_MEMCOPY("AND", c, StringLen("AND"));} break;
            case CLASS:         { SHU_MEMCOPY("CLASS", c, StringLen("CLASS"));} break;
            case ELSE:          { SHU_MEMCOPY("ELSE", c, StringLen("ELSE"));} break;
            case _FALSE:        { SHU_MEMCOPY("_FALSE", c, StringLen("_FALSE"));} break;
            case FUN:           { SHU_MEMCOPY("FUN", c, StringLen("FUN"));} break;
            case FOR:           { SHU_MEMCOPY("FOR", c, StringLen("FOR"));} break;
            case IF:            { SHU_MEMCOPY("IF", c, StringLen("IF"));} break;
            case NIL:           { SHU_MEMCOPY("NIL", c, StringLen("NIL"));} break;
            case OR:            { SHU_MEMCOPY("OR", c, StringLen("OR"));} break;
            case PRINT:         { SHU_MEMCOPY("PRINT", c, StringLen("PRINT"));} break;
            case RETURN:        { SHU_MEMCOPY("RETURN", c, StringLen("RETURN"));} break;
            case SUPER:         { SHU_MEMCOPY("SUPER", c, StringLen("SUPER"));} break;
            case THIS:          { SHU_MEMCOPY("THIS", c, StringLen("THIS"));} break;
            case _TRUE:         { SHU_MEMCOPY("_TRUE", c, StringLen("_TRUE"));} break;
            case VAR:           { SHU_MEMCOPY("VAR", c, StringLen("VAR"));} break;
            case WHILE:         { SHU_MEMCOPY("WHILE", c, StringLen("WHILE"));} break;
            case END_OF_FILE:   { SHU_MEMCOPY("END_OF_FILE", c, StringLen("END_OF_FILE"));} break;
        }
    }
}