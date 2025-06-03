/*
 * This file is part of the single-file (sort of) header lexer library
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
Objectives:
--------------------------------------------------------------------------------


[-] we need to identify file scopes
    Lexer_DefineScript(const char *identifier);
    Lexer_DefineScope(Lexer_Script *script, const char *scope);
[-] we want to define some abritrary grammar that a script(s) should obey
    Lexer_ExpectMatch(p_open_brace, p_close_brace, "Found open bracket with no closing bracket");
[-] we want to define what crap is (comments) so ignore
    Lexer_DefineIgnore(p_singleline_comment, P_Newline);
    Lexer_DefineIgnore(p_multiline_comment_open, p_multiline_comment_close);
[-] we need to define atomics
    Lexer_DefineAtomic(?)
    Lexer_DefineVariable(?)
    Lexer_DefineConstant(?)
    Lexer_DefineKeyword(?)
    Lexer_DefineOperation(?)
    Lexer_DefineScopeStart(?)
    Lexer_DefineScopeEnd(?)
[-] we need to define compile-time vs runtime stuff (@TODO)

[+] we need to define error/stack tracing
    Lexer_SetTracer(void (*cb)(const char *msg, const char *file, maxint_t line, maxint_t offset));
*/

#ifndef _KLEXER_H_
#define _KLEXER_H_

#include <stdint.h>
#include <stdlib.h>
#include "kparser.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define LEXER_UNKNOWN           (1<<0) // Simply, unknown
#define LEXER_DIGIT             (1<<1) // A number based representation
#define LEXER_ALPHANUMERIC      (1<<2) // An alphanumerical representation

typedef struct {
    char            *name;
    token_list_t    *variables;    
} scope_t;

typedef struct {
    scope_t         *items;
    intmax_t         capacity;
    intmax_t         count;
} scopt_list_t;

typedef struct {
    token_t          start_match;
    token_t          end_match;
} rule_t;

typedef struct {
    rule_t          *items;
    intmax_t         capacity;
    intmax_t         count;
} rule_list_t;

typedef struct {
    char            *filename;
    scopt_list_t     scopes;
} script_t;

typedef void (*tracer_t)(const char *msg, const char *file, const char *scope, intmax_t line, intmax_t offset);
typedef struct {
    tracer_t         tracer;
    scope_t          global_scope;
    parser_t        *parser;
} lexer_t;

lexer_t *lexer_init(const char *buffer);
void     lexer_destroy(lexer_t *lexer);
// Necessary to output any parsing/lexing errors
void     lexer_set_tracer(lexer_t *lexer, tracer_t tracer);
void     lexer_declare_rule(token_t start, token_t end, const char *error);


// Mainly used internally when parsing
void     lexer_parse_script(const char *identifier);
scope_t *lexer_parse_scope(lexer_t *lexer, const char *scope);

#ifdef __cplusplus
};
#endif // __cplusplus


#endif // _KLEXER_H_

#ifdef _KLEXER_IMPLEMENTATION

lexer_t *lexer_init(const char *buffer)
{
    return nullptr;
}

void lexer_destroy(lexer_t *lexer)
{
    if (lexer) {
        _KFREE(lexer);
    }
}

void lexer_set_tracer(lexer_t *lexer, tracer_t tracer)
{
    _KASSERT(lexer);
    lexer->tracer = tracer;
}

void lexer_declare_rule(token_t start, token_t end, const char *error)
{
    
}

void lexer_parse_script(const char *identifier)
{

}

scope_t *lexer_parse_scope(lexer_t *lexer, const char *scope)
{
    return nullptr;
}

#endif // _KLEXER_IMPLEMENTATION