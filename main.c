#include <stdio.h>

#define _KPARSER_IMPLEMENTATION
#include "kparser.h"

// Make sure to include multibyte punctuation before single-byte ones
// e.g. '<<' before '<'
typedef enum {
    P_ShiftLeft,
    P_ShiftRight,
    P_OpenBrace,
    P_CloseBrace,
    P_OpenBracket,
    P_CloseBracket,
    P_Plus,
    P_Minus,
    P_Multiply,
    P_Divide
} MyPunctuation;

static Punctuation punctuation[] = {
    "<<", P_ShiftLeft, 0,
    ">>", P_ShiftRight, 0,
    "(", P_OpenBrace, 0,
    ")", P_CloseBrace, 0,
    "[", P_OpenBracket, 0,
    "]", P_CloseBracket, 0,
    "+", P_Plus, 0,
    "-", P_Minus, 0,
    "*", P_Multiply, 0,
    "/", P_Divide, 0
};

static const char *script = "( hello\t a>>b world + dingles)\n"
                            "[and now]";

void TestPunctuation() 
{
    PunctuationList *plist = Punctuation_Init();
    //  Define the punctuation list
    for (int i = 0; i < (int)sizeof(punctuation) / sizeof(punctuation[0]); i++) {
        Punctuation_Add(plist, punctuation[i].p, punctuation[i].id);
    }
    for (int i = 0; i < plist->count; i++) {
        printf("Punctuation: \"%s\" (%i)\n", plist->items[i].p, plist->items[i].id);
    }

    // Parse a piece of string given the punctuation definition
    Parser *parser = Parser_Init(script, plist, P_ACCEPT_DOUBLEQUOTES|P_ACCEPT_SINGLEQUOTES);

    Token token = Parser_GetToken(parser);
    while (token.id != -2) {
        printf("Token (id:%i): [%s]\n", token.id, token.token);
        token = Parser_GetToken(parser);
    }


    Punctuation_Destroy(plist);
    Parser_Destroy(parser);
}

int main(int argc, char* argv[]) 
{
    TestPunctuation();
    return 0;
}
