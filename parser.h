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
#include <string.h>

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
    int len; 
    int id;
} Punctuation;

typedef struct {
    Punctuation *items;
    int64_t capacity;
    int64_t count;
} PunctuationList;

typedef struct {
    int id;
    char *token;
    int64_t len;
    int64_t line;   // line in buffer
    int64_t offset; // in buffer
} Token;

typedef struct {
    Token * items;
    int64_t capacity;
    int64_t count;
} TokenList;

typedef struct {
    int                      options;
    const char              *buffer;
    const PunctuationList   *punctuation;
    int64_t                  buffer_size;
    TokenList                tokens;
    int64_t                  current_token;
} Parser;

#define P_ACCEPT_SINGLEQUOTES 0x01  // parse single quote slices as a whole token
#define P_ACCEPT_DOUBLEQUOTES 0x02  // parse double quoted slices as a whole token

//
// Punctuation/Delimiter management
//
PunctuationList *Punctuation_Init();
// will calculate the length of the punctuation as well
void             Punctuation_Add(PunctuationList *list, const char *token, int id);
void             Punctuation_Destroy(PunctuationList *list);

//
// Parsing 
//
Parser          *Parser_Init(const char *buffer, const PunctuationList *punctuation, int options);
void             Parser_Destroy(Parser *parser);

int64_t          Parser_Parse(Parser *parser);      // move the cursor to the next token, returns line position (-1 if EOF)
void             Parser_UngetToken(Parser *parser); // reset to the previous token
const char      *Parser_GetToken(Parser *parser);   // return the string representation of the current token
int              Parser_GetTokenId(Parser *parser); // return -1 if a token, 0+ if its a punctuation
int              Parser_PeekToken(Parser *parser);  // peek the next token, but don't move the cursor
int64_t          Parser_GetLine(Parser *parser);    // get the current line in the script
int              Parser_IsPunctuation(Parser *parser, int64_t start_offset);
#ifdef __cplusplus
};
#endif // __cplusplus

#ifdef _KPARSER_IMPLEMENTATION

// return -1 if not, *index* of the punctuation if it is
int Parser_IsPunctuation(Parser *parser, int64_t start_offset) 
{
    int64_t offset = start_offset;
    int punctuation = -1;
    int found_punc = 0;
    int p_index = 0;

    for (int k = 0; k < parser->punctuation->count; k++) {
        // first char match
        if (parser->buffer[offset] == parser->punctuation->items[k].p[p_index]) {
            found_punc = 1;
            // match remaining chars
            while (++offset < parser->buffer_size && ++p_index < parser->punctuation->items[k].len) {
                if (parser->buffer[offset] != parser->punctuation->items[k].p[p_index]) {
                    found_punc = 0;
                    offset = start_offset; // reverse
                    p_index = 0;
                    break;
                }
            }

            if (found_punc) {
                punctuation = k;
                break;
            }
        }
    } 
    
    return punctuation;
}

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
        .p = token,
        .len = strlen(token)
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

Parser *Parser_Init(const char *buffer, const PunctuationList *punctuation, int options) 
{
    _KASSERT(punctuation);

    Parser *p = (Parser*) _KMALLOC(sizeof(Parser));
    if (p) {
        p->options = options;
        p->buffer = buffer;
        p->buffer_size = strlen(buffer);
        p->punctuation = punctuation;
        p->tokens.capacity = 255;
        p->tokens.count = 0;
        p->tokens.items = (Token*)_KMALLOC(sizeof(Token) * p->tokens.capacity);
        
        int64_t current_line = 0, begin_token = 0, end_token = 0;
        Token token;
        
        // Parse the entire buffer
        for (int64_t i = 0; i < p->buffer_size; i++) {
            if (p->buffer[i] == '\r' && i < p->buffer_size) i++; // skip \r
            if (p->buffer[i] == '\n') {
                current_line++;
            }

            while (p->buffer[i] == ' ' && i < p->buffer_size) i++; // skip starting whitespace

            // are we starting any punctuations
            int is_punc = Parser_IsPunctuation(p, i);
            if (is_punc != -1) {
                // gobble up until we hit whitespace or another punctuation
                int64_t start_offset = i;
                int64_t end_offset = start_offset + 1;
                int64_t start_line = current_line;

                if (p->options & P_ACCEPT_DOUBLEQUOTES && p->buffer[i] == '"') {
                    while (i < p->buffer_size) {
                        int is_escaped = 0;
                        if (i < p->buffer_size-1 && p->buffer[i-1] == '\\') {
                            is_escaped = 1;
                        }
                        
                        if (is_escaped == 0 && p->buffer[++i] == '"') break;

                        end_offset++;
                    }
                } else if (p->options & P_ACCEPT_SINGLEQUOTES && p->buffer[i] == '\'') {
                    while (i < p->buffer_size) {
                        int is_escaped = 0;
                        if (i < p->buffer_size-1 && p->buffer[i-1] == '\\') {
                            is_escaped = 1;
                        }
                        
                        if (is_escaped == 0 && p->buffer[++i] == '\'') break;

                        end_offset++;
                    }
                } else { 
                    while (i < p->buffer_size) {
                        if (p->buffer[i] == '\r' && i < p->buffer_size) i++; // skip \r
                        if (p->buffer[i] == '\n') current_line++;

                        is_punc = Parser_IsPunctuation(p, i);
                        // if we hit punctuation or whitespace, halt at this token
                        if (
                            is_punc > -1 || 
                            p->buffer[i] == '\n' || 
                            p->buffer[i] == '\t' ||
                            p->buffer[i] == ' '
                        ) {
                            // break and possibly capture the punctuation
                            break;
                        }
                    
                        end_offset++;
                        i++;
                    }
                }

                // normal token
                if (is_punc < 0) {
                        token.id = -1;
                        token.line = start_line;
                        token.len = end_offset - start_offset;
                        token.offset = start_offset;
                        token.token = (char*)malloc(token.len);
                        strncpy(token.token, buffer + start_offset, token.len);
                        token.token[token.len] = '\0';
                }
            } 

            if (is_punc) {
                token.id = p->punctuation->items[is_punc].id;
                token.len = p->punctuation->items[is_punc].len;
                token.offset = i;
                token.line = current_line;
                token.token = (char*) malloc(token.len);
                strcpy(token.token, p->punctuation->items[is_punc].p);
            }

            // Add the token (expand array size if necessary)
            if (p->tokens.count >= p->tokens.capacity) {
                p->tokens.capacity *= 2;
                p->tokens.items = (Token*) _KREALLOC(p->tokens.items, p->tokens.capacity);
            }
 
            printf("Found punctuation, adding: %s\n", token.token);
            p->tokens.items[p->tokens.count++] = token;
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
    if (parser->current_token < parser->tokens.count) {
        parser->current_token++;
        return parser->tokens.items[parser->current_token].line;
    }

    return -1; // reached EOF
}

int Parser_GetTokenId(Parser *parser)
{
    _KASSERT(parser);

    return parser->tokens.items[parser->current_token].id;
}

const char *Parser_GetToken(Parser *parser)
{
    _KASSERT(parser);

    return (const char *)parser->tokens.items[parser->current_token].token;
}

void Parser_UngetToken(Parser *parser)
{
    _KASSERT(parser);
    if (parser->current_token > 0) {
        --parser->current_token;
    }
}

int Parser_PeekToken(Parser *parser)
{
    _KASSERT(parser);

    if (parser->current_token <= parser->tokens.count) {
        return parser->tokens.items[parser->current_token + 1].id;
    }

    return -2;
}

int64_t Parser_GetLine(Parser *parser)
{
    _KASSERT(parser);
    
    return parser->tokens.items[parser->current_token].line;
}

#endif // _KPARSER_IMPLEMENTATION

#endif // _KPARSER_H