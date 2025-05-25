/*
 * This file is part of the standalone Parser header file
 * 
 * Copyright (C) 2025 Kurt Ferreira
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 */

// Description:
// This is a thread-safe (provided you don't access the buffer being processed) parser.
// It includes fundamentals to parse a piece of text with established punctuation etc.
// NOTE:
//  This is not unicode-compliant just yet. Feel free to submit a PR if you
//  need it.

#ifndef _KPARSER_H_
#define _KPARSER_H_

#include <stdint.h>

#ifndef _KPARSER_IMPLEMENTATION 
#define _KPARSER_IMPLEMENTATION
#endif

#ifndef nullptr 
    #define nullptr (void*)NULL
#endif // nullptr 

#ifndef _KASSERT
    #include <assert.h>
    #define _KASSERT(x) assert(x)
#endif // _KASSERT
#ifndef _KMALLOC
    #include <stdlib.h>
    #define _KMALLOC(x) malloc(x)
    #define _KREALLOC(a,b) realloc(a,b)
    #define _KFREE(x) free(x)
#endif // _KMALLOC

#ifdef __cplusplus
extern "C" {
#endif
 
typedef struct {
    const char *p;
    int id;
} Punctuation;

typedef struct {
    Punctuation *items;
    int64_t capacity;
    int64_t count;
} PunctuationList;

typedef struct {
    const char *token;
    int id;
    int64_t line;   // line in buffer
    int64_t offset; // in buffer
} Token;

typedef struct {
    Token * items;
    int64_t capacity;
    int64_t count;
} TokenList;

typedef struct {
    const char      *buffer;
    int64_t          buffer_size;
    PunctuationList *punctuation;
    TokenList        tokens;
    int64_t          current;
} Parser;

PunctuationList *Punctuation_Init();
void             Punctuation_Add(PunctuationList *list, const char *token, int id);
void             Punctuation_Destroy(PunctuationList *list);

Parser          *Parser_Init(const char *buffer, const PunctuationList *punctuation);
void             Parser_Destroy(Parser *parser);

int64_t          Parser_Parse(Parser *parser);      // move the cursor to the next token, returns line position (-1 if EOF)
void             Parser_UngetToken(Parser *parser); // reset to the previous token
const char      *Parser_GetToken(Parser *parser);   // return the string representation of the current token
int              Parser_GetTokenId(Parser *parser); // return -1 if a token, 0+ if its a punctuation
int              Parser_PeekToken(Parser *parser);  // peek the next token, but don't move the cursor
int64_t          Parser_GetLine(Parser *parser);    // get the current line in the script

#ifdef __cplusplus
};
#endif // __cplusplus

#ifdef _KPARSER_IMPLEMENTATION

PunctuationList *Punctuation_Init()
{
    PunctuationList * list = (PunctuationList*)_KMALLOC(sizeof(PunctuationList));
    if (list) {
        list->count = 0;
        list->capacity = 16;
        list->items = (Punctuation*)_KMALLOC(sizeof(Punctuation) * list->capacity);
        return list;
    }

    return nullptr;
}

void Punctuation_Add(PunctuationList *list, const char *token, int id) 
{
    _KASSERT(list != nullptr);

    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->items = (Punctuation*) _KREALLOC(list->items, sizeof(Punctuation) * list->capacity);
    }

    list->items[list->count++] = (Punctuation) {
        .id = id,
        .p = token
    } ;
}

void Punctuation_Destroy(PunctuationList *list)
{
    if (list) {
        if (list->items) {
            _KFREE(list->items);
        }

        _KFREE(list);
    }
}

Parser *Parser_Init(const char *buffer, const PunctuationList *punctuation) 
{
    _KASSERT(punctuation);

    Parser *p = (Parser*) _KMALLOC(sizeof(Parser));
    if (p) {
        p->buffer = buffer;
        p->buffer_size = strlen(buffer);
        p->punctuation = punctuation;
        p->tokens.capacity = 255;
        p->tokens.count = 0;
        p->tokens.items = (Token*)_KMALLOC(sizeof(Token) * p->tokens.capacity);
        
        // Parse the entire buffer
        int64_t current_line = 0, begin_token = 0, end_token = 0;
        for (int64_t i = 0; i < p->buffer_size; i++) {
            if (p->buffer[i] == '\r' && i < p->buffer_size) i++; // skip \r
            if (p->buffer[i] == '\n') {
                current_line++;
            }

            while (p->buffer[i] == ' ' && i < p->buffer_size) i++; // skip whitespace

            // are we scanning any punctuations
            for (uint64_t k = 0; k < p->punctuation->count; k++) {
                if (p->buffer[i] == p->punctuation->items[k].p[0]) {

                }
            }

            // Add the token (expand array size if necessary)
            if (p->tokens.count >= p->tokens.capacity) {
                p->tokens.capacity *= 2;
                p->tokens.items = (Token*) _KREALLOC(p->tokens.items, p->tokens.capacity);
            }

            // Token t = {
            //     .id = -1,
            //     .line = current_line,
            //     .offset = i,
            //     .token = ""
            // };
            // p->tokens.items[p->tokens.count++] = t;
        }

        return p;
    }

    return nullptr;
}

void Parser_Destroy(Parser *parser) 
{
    _KASSERT(parser);
    
    if (parser->tokens.items) {
        _KFREE(parser->tokens.items);
    }

    _KFREE(parser);
    parser = nullptr;
}

int64_t Parser_Parse(Parser *parser)
{
    _KASSERT(parser);
    if (parser->current < parser->tokens.count) {
        parser->current++;
        return parser->tokens.items[parser->current].line;
    }

    return -1; // reached EOF
}

int Parser_GetTokenId(Parser *parser)
{
    _KASSERT(parser);

    return (const char *)parser->tokens.items[parser->current].id;
}

const char *Parser_GetToken(Parser *parser)
{
    _KASSERT(parser);

    return (const char *)parser->tokens.items[parser->current].token;
}

void Parser_UngetToken(Parser *parser)
{
    _KASSERT(parser);
    if (parser->current > 0) {
        --parser->current;
    }
}

int Parser_PeekToken(Parser *parser)
{
    _KASSERT(parser);

    if (parser->current <= parser->tokens.count) {
        return parser->tokens.items[parser->current + 1].id;
    }

    return -2;
}

int64_t Parser_GetLine(Parser *parser)
{
    _KASSERT(parser);
    
    return parser->tokens.items[parser->current].line;
}

#endif // _KPARSER_IMPLEMENTATION

#endif // _KPARSER_H