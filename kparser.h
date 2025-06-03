/*
 * This file is part of the single-file header parser library
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

/*
Description:
    This is a thread-safe (provided you don't access the buffer being 
    processed) parser. It includes fundamentals to parse a piece of text with 
    established punc_t etc.
NOTE:
    This is not unicode-compliant just yet. Feel free to submit a PR if you
    need it.

Usage:
    #define_KPARSER_IMPLEMENTATION // do in only one file
    #include "parser.h"

Allocations:
    Allocations happen when:
        - punc_t list (sizeof(punc_t) + initial 
          16x sizeof(punc_t))
        - initializing the parser_t (sizeof parser_t)
        - each token (sizeof(token_t) + string length)

If you want to provide your own assert/malloc/free define before including:
    _KASSERT
    _KMALLOC 
        -> then define _KMALLOC, _KREALLOC and _KFREE for each mem op

================================================================================

Changelog

v0.1.1 - Initial public release.

================================================================================
*/

#ifndef _KPARSER_H_
#define _KPARSER_H_

#include <stdint.h>
#include <string.h>

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
    int         id;
    int         len; 
} punc_t;

typedef struct {
    punc_t      *items;
    intmax_t     capacity;
    intmax_t     count;
} punc_list_t;

typedef struct {
    int          id;
    char        *token;
    intmax_t     len;
    intmax_t     line;   // line in buffer
    intmax_t     offset; // in buffer
} token_t;

typedef struct {
    token_t     *items;
    intmax_t     capacity;
    intmax_t     count;
} token_list_t;

typedef struct {
    int                  options;
    const char          *buffer;
    const punc_list_t   *punctuation;
    intmax_t             buffer_size;
    token_list_t         tokens;
    intmax_t             current_token;
} parser_t;

// parser_t options
// parse single quote slices as a whole token
#define P_ACCEPT_SINGLEQUOTES 0x01  
// parse double quoted slices as a whole token
#define P_ACCEPT_DOUBLEQUOTES 0x02  

//
// punc_t/Delimiter management
//
punc_list_t        *punc_init();
// will calculate the length of the punc_t as well
void                punc_add(punc_list_t *list, const char *token, int id);
void                punc_destroy(punc_list_t *list);

//
// Parsing 
//
parser_t           *parser_init(const char *buffer, const punc_list_t *punctuation, int options);
void                parser_destroy(parser_t *parser);


const token_t       parser_get_token(parser_t *parser);   // return the current token and progress the cursor
void                parser_unget_token(parser_t *parser); // reset to the previous token
const token_t       parser_peek_token(parser_t *parser);  // peek the next token, but don't move the cursor (-2 if EOF)
intmax_t            parser_get_line(parser_t *parser);   // get the current line in the script
int                 parser_is_punctuation(parser_t *parser, intmax_t start_offset);
#ifdef __cplusplus
};
#endif // __cplusplus

#endif // _KPARSER_H

#ifdef _KPARSER_IMPLEMENTATION
// return -1 if not, *index* of the punc_t if it is
int parser_is_punctuation(parser_t *parser, intmax_t start_offset) 
{
    intmax_t offset = start_offset;
    int punc_t = -1;
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
                punc_t = k;
                break;
            }
        }
    } 
    
    return punc_t;
}

punc_list_t *punc_init()
{
    punc_list_t * list = (punc_list_t*)_KMALLOC(sizeof(punc_list_t));
    if (list) {
        list->count = 0;
        list->capacity = 16;
        list->items = (punc_t*)_KMALLOC(sizeof(punc_t) * list->capacity);
        return list;
    }

    return nullptr;
}

void punc_add(punc_list_t *list, const char *token, int id) 
{
    _KASSERT(list != nullptr);

    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->items = (punc_t*) _KREALLOC(list->items, sizeof(punc_t) * list->capacity);
    }

    list->items[list->count++] = (punc_t) {
        .id = id,
        .p = token,
        .len = strlen(token)
    } ;
}

void punc_destroy(punc_list_t *list)
{
    if (list) {
        if (list->items) {
            _KFREE(list->items);
        }

        _KFREE(list);
    }
}

parser_t *parser_init(const char *buffer, const punc_list_t *punctuation, int options) 
{
    _KASSERT(punctuation);

    parser_t *p = (parser_t*) _KMALLOC(sizeof(parser_t));
    if (p) {
        p->options = options;
        p->buffer = buffer;
        p->buffer_size = strlen(buffer);
        p->punctuation = punctuation;
        p->tokens.capacity = 255;
        p->tokens.count = 0;
        p->tokens.items = (token_t*)_KMALLOC(sizeof(token_t) * p->tokens.capacity);
        
        intmax_t current_line = 0, begin_token = 0, end_token = 0;
        token_t token;
        
        // Parse the entire buffer
        for (intmax_t i = 0; i < p->buffer_size; i++) {
            while (p->buffer[i] == ' ' && i < p->buffer_size) {
                i++; // skip starting whitespace
            }

            if (p->buffer[i] == '\r' && i < p->buffer_size) {
                i++; // skip \r
            }

            if (p->buffer[i] == '\n') {
                current_line++;
            }

            // are we starting any punctuations
            int is_punc = parser_is_punctuation(p, i);
            if (is_punc == -1) {
                // gobble up until we hit whitespace or another punc_t
                intmax_t start_offset = i;
                intmax_t end_offset = start_offset + 1;
                intmax_t start_line = current_line;

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

                        is_punc = parser_is_punctuation(p, i);
                        
                        // delimited by punc_t, reverse so we can track next round
                        if (is_punc > -1) {
                            is_punc = -1;
                            i -= p->punctuation->items[is_punc].len + 1; 
                            break;
                        }

                        // we've reached a whitespace break
                        if (p->buffer[i] == '\n' || p->buffer[i] == '\t' || p->buffer[i] == ' ') break;
                    
                        end_offset++;
                        i++;
                    }
                }
                
                // normal token
                if (is_punc < 0) {
                    token.id = -1;
                    token.line = start_line;
                    token.offset = start_offset;
                    token.len = (end_offset - start_offset) - 1;
                    token.token = (char*)_KMALLOC(token.len);
                    
                    strncpy(token.token, buffer + start_offset, token.len);
                    token.token[token.len] = '\0';
                }
            } 

            if (is_punc >= 0) {
                token.id = p->punctuation->items[is_punc].id;
                token.len = p->punctuation->items[is_punc].len;
                token.offset = i;
                token.line = current_line;
                token.token = (char*) _KMALLOC(token.len);
                strcpy(token.token, p->punctuation->items[is_punc].p);

                if (token.len > 1) {
                    // increment by the full punc_t size
                    i += token.len; // -1 because we've already got the first character
                }
            } else {
                if (token.len == 0) continue; // empty tokens
            }

            // Add the token (expand array size if necessary)
            if (p->tokens.count >= p->tokens.capacity) {
                p->tokens.capacity *= 2;
                p->tokens.items = (token_t*) _KREALLOC(p->tokens.items, p->tokens.capacity);
            }
 
            p->tokens.items[p->tokens.count++] = token;
        }

        return p;
    }

    return nullptr;
}

void parser_destroy(parser_t *parser) 
{
    _KASSERT(parser);
    
    if (parser->tokens.items) {
        for (int i = 0; i < parser->tokens.count; i++) {
            _KFREE(parser->tokens.items[i].token);
        }
        _KFREE(parser->tokens.items);
    }

    _KFREE(parser);
    parser = nullptr;
}


const token_t parser_get_token(parser_t *parser)
{
    _KASSERT(parser);

    if (parser->current_token < parser->tokens.count) {
        return parser->tokens.items[parser->current_token++];
    }

    const token_t eof_token = {
        .id = -2,
        .len = 3,
        .line = parser->tokens.items[parser->current_token-1].line+1,
        .offset = parser->tokens.items[parser->current_token-1].offset + parser->tokens.items[parser->current_token-1].len,
        .token = nullptr
    };

    return eof_token;
}

void parser_unget_token(parser_t *parser)
{
    _KASSERT(parser);
    if (parser->current_token > 0) {
        --parser->current_token;
    }
}

const token_t parser_peek_token(parser_t *parser)
{
    _KASSERT(parser);

    if (parser->current_token < parser->tokens.count) {
        return parser->tokens.items[parser->current_token + 1];
    }

    const token_t eof_token = {
        .id = -2,
        .len = 3,
        .line = parser->tokens.items[parser->current_token-1].line+1,
        .offset = parser->tokens.items[parser->current_token-1].offset + parser->tokens.items[parser->current_token-1].len,
        .token = nullptr
    };

    return eof_token;
}

intmax_t parser_get_line(parser_t *parser)
{
    _KASSERT(parser);
    
    return parser->tokens.items[parser->current_token].line;
}

#endif // _KPARSER_IMPLEMENTATION