#if SHU_CUSTOM_IMPLEMENTATION

#include <memory.h>
#include <platform/platform.h>

u32 Stack[4096] = {0};
u32 *StackPtr = Stack;

typedef struct
{
    char *generator;
    char *version;
} gltf_asset;

typedef enum
{
    TokenType_Unknown,

    TokenType_JsonObject,
    TokenType_JsonArray,

    TokenType_String,
    TokenType_Numeric,
    TokenType_Boolean,

    TokenType_EOF
} json_token_type;

typedef struct parser
{
    char *At;

    i32 ActiveTokenIndex;
    i32 TokenCount;
} parser;

typedef struct token
{
    json_token_type TokenType;

    char *Text;
    i32 TextLength = -1;

    i32 ParentTokenIndex = -1;
    i32 ChildCount = -1;
} token;

inline b32
IsWhiteSpace(char *At)
{
    b32 Result = false;
    if (*At == ' ' || *At == '\t')
    {
        Result = true;
    }

    return Result;
}

inline b32
IsNumber(char *At)
{
    b32 Result = false;
    if (*At >= '0' && *At <= '9')
    {
        Result = true;
    }

    return Result;
}

inline b32
IsLineEnd(char *At)
{
    b32 Result = false;
    if (*At == '\r' || *At == '\n')
    {
        Result = true;
    }

    return Result;
}

inline void
SkipSpaces(parser *Parser)
{
    while (1)
    {
        if (IsWhiteSpace(Parser->At) || IsLineEnd(Parser->At))
        {
            ++Parser->At;
        }
        else
        {
            break;
        }
    }
}

token
GetNewJsonToken(parser *Parser)
{
    token Token;
    Token.TokenType = TokenType_Unknown;
    Token.ParentTokenIndex = -1;
    Token.ChildCount = 0;

    Token.Text = Parser->At;

    ++Parser->TokenCount;
    return Token;
}

void
ParseJson(parser *Parser, token *Tokens)
{
    while (Parser->At[0] != '\0')
    {

        SkipSpaces(Parser);
        char C = Parser->At[0];

        switch (C)
        {
        case '\0':
        {
            return;
        }
        break;

        case '{':
        case '[':
        {
            token Token = GetNewJsonToken(Parser);
            Token.TokenType = (C == '{') ? TokenType_JsonObject : TokenType_JsonArray;
            Token.ParentTokenIndex = Parser->ActiveTokenIndex;
            if (Token.ParentTokenIndex >= 0)
            {
                ++Tokens[Token.ParentTokenIndex].ChildCount;
            }

            Parser->ActiveTokenIndex = Parser->TokenCount - 1;
            ++Parser->At;
            Tokens[Parser->TokenCount - 1] = Token;
        }
        break;

        case '}':
        case ']':
        {
            json_token_type TypeToCheck = C == '}' ? TokenType_JsonObject : TokenType_JsonArray;
            token *Token = &Tokens[Parser->TokenCount - 1];

            while (Token->TextLength != -1)
            {
                Token = &Tokens[Token->ParentTokenIndex];
            }
            ASSERT(Token->TokenType == TypeToCheck);

            ++Parser->At;
            Token->TextLength = Parser->At - Token->Text;
            Parser->ActiveTokenIndex = Token->ParentTokenIndex;
            // ParseJson(Parser, Tokens);
        }
        break;

        case '\"':
        {
            ++Parser->At;
            token Token = GetNewJsonToken(Parser);
            Token.TokenType = TokenType_String;

            while (Parser->At[0] != '\"')
            {
                ++Parser->At;
            }

            Token.ParentTokenIndex = Parser->ActiveTokenIndex;
            if (Token.ParentTokenIndex >= 0)
            {
                ++Tokens[Token.ParentTokenIndex].ChildCount;
            }

            Token.TextLength = Parser->At - Token.Text;
            Token.TokenType = TokenType_String;
            ++Parser->At;
            Tokens[Parser->TokenCount - 1] = Token;
        }
        break;

        case ':':
        {
            Parser->ActiveTokenIndex = Parser->TokenCount - 1;
            ++Parser->At;
            // ParseJson(Parser, Tokens);
        }
        break;

        case ',':
        {
            if (Tokens[Parser->ActiveTokenIndex].TokenType != TokenType_JsonObject &&
                Tokens[Parser->ActiveTokenIndex].TokenType != TokenType_JsonArray)
            {
                Parser->ActiveTokenIndex = Tokens[Parser->ActiveTokenIndex].ParentTokenIndex;
            }
            ++Parser->At;
            // ParseJson(Parser, Tokens);
        }
        break;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':
        case '-':
        {
            token Token = GetNewJsonToken(Parser);
            Token.Text = Parser->At++;

            while (IsNumber(Parser->At) || *Parser->At == '.')
            {
                ++Parser->At;
            }

            Token.ParentTokenIndex = Parser->ActiveTokenIndex;
            if (Token.ParentTokenIndex >= 0)
            {
                ++Tokens[Token.ParentTokenIndex].ChildCount;
            }

            Token.TextLength = Parser->At - Token.Text;
            Token.TokenType = TokenType_Numeric;

            Tokens[Parser->TokenCount - 1] = Token;
        }
        break;

        case 't':
        {
            if (Parser->At[1] == 'r' && Parser->At[2] == 'u' && Parser->At[3] == 'e')
            {
                token Token = GetNewJsonToken(Parser);
                Parser->At += 4;

                Token.ParentTokenIndex = Parser->ActiveTokenIndex;
                if (Token.ParentTokenIndex >= 0)
                {
                    ++Tokens[Token.ParentTokenIndex].ChildCount;
                }

                Token.TextLength = Parser->At - Token.Text;
                Token.TokenType = TokenType_Boolean;
                Tokens[Parser->TokenCount - 1] = Token;
            }
        }
        break;

        case 'f':
        {
            if (Parser->At[1] == 'a' && Parser->At[2] == 'l' && Parser->At[3] == 's' && Parser->At[4] == 'e')
            {
                token Token = GetNewJsonToken(Parser);
                Parser->At += 5;
                Token.ParentTokenIndex = Parser->ActiveTokenIndex;
                if (Token.ParentTokenIndex >= 0)
                {
                    ++Tokens[Token.ParentTokenIndex].ChildCount;
                }

                Token.TextLength = Parser->At - Token.Text;
                Token.TokenType = TokenType_Boolean;
                Tokens[Parser->TokenCount - 1] = Token;
            }
        }
        break;

            SHU_INVALID_DEFAULT
        }
    }
}

char *
LoadFileContents(const char *Path)
{
    FILE *File = fopen(Path, "rb");

    if (File == nullptr)
    {
        LogInfo("Could not load file: %s\n", Path);
        return nullptr;
    }

    fseek(File, 0, SEEK_END);
    u64 FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);

    char *FileData = (char *)malloc(FileSize + 1);
    FileData[FileSize] = '\0';

    if (FileData == nullptr)
    {
        LogErrorUnformatted("Could not get memory!\n");
        return nullptr;
    }

    size_t ElementsRead = fread(FileData, 1, FileSize, File);

    fclose(File);
    File = nullptr;

    return FileData;
}

void
LoadMeshCustom(const char *Path)
{
    char *FileContents = LoadFileContents(Path);

    parser Parser = {};
    Parser.At = (char *)FileContents;
    Parser.ActiveTokenIndex = -1;
    Parser.TokenCount = 0;

    token *Tokens = (token *)malloc(1024 * 1024 * 1024);
    u32 TokenCount = 0;

    ParseJson(&Parser, Tokens);

    for (u32 Index = 0; Index < Parser.TokenCount; ++Index)
    {
        LogInfo("Token: %.*s\n", Tokens[Index].TextLength, Tokens[Index].Text);
    }

    gltf_asset asset = {};
    // GLTF Asset struct packing
    i32 i = 1;
    if (strncmp("asset", Tokens[i].Text, Tokens[i].TextLength) == 0)
    {
        i += 2;
        if (strncmp("generator", Tokens[i].Text, Tokens[i].TextLength) == 0)
        {
            token SrcToken = Tokens[i + 1];

            asset.generator = (char *)malloc(SrcToken.TextLength + 1);

            strncpy(asset.generator, SrcToken.Text, SrcToken.TextLength);
            asset.generator[SrcToken.TextLength] = '\0';
        }
        i += 2;
        if (strncmp("version", Tokens[i].Text, Tokens[i].TextLength) == 0)
        {
            token SrcToken = Tokens[i + 1];

            asset.version = (char *)malloc(SrcToken.TextLength + 1);
            strncpy(asset.version, SrcToken.Text, SrcToken.TextLength);
            asset.version[SrcToken.TextLength] = '\0';
        }
    }

    int x = 0;
    free(Tokens);
}

void
Free(void **Data)
{
    free((void *)*Data);
}
#endif