#include <stdio.h>

#define _KPARSER_IMPLEMENTATION
#include "parser.h"

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
    "<<", P_ShiftLeft,
    ">>", P_ShiftRight,
    "(", P_OpenBrace,
    ")", P_CloseBrace,
    "[", P_OpenBracket,
    "]", P_CloseBracket,
    "+", P_Plus,
    "-", P_Minus,
    "*", P_Multiply,
    "/", P_Divide
};

static const char *script = "";

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
    Parser *parser = Parser_Init(script, plist);


    Punctuation_Destroy(plist);
    Parser_Destroy(parser);
}

int main(int argc, char* argv[]) 
{
    TestPunctuation();
    return 0;
}
