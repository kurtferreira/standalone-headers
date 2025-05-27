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

- we need to define error/stack tracing
    Lexer_SetTracer(void (*cb)(const char *msg, const char *file, maxint_t line, maxint_t offset));
- we need to identify file scopes
    Lexer_DefineScript(const char *identifier);
    Lexer_DefineScope(Lexer_Script *script, const char *scope);
- we want to define some abritrary grammar that a script(s) should obey
    Lexer_ExpectMatch(p_open_brace, p_close_brace, "Found open bracket with no closing bracket");
- we want to define what crap is (comments) so ignore
    Lexer_DefineIgnore(p_singleline_comment, P_Newline);
    Lexer_DefineIgnore(p_multiline_comment_open, p_multiline_comment_close);
- we need to define atomics
    Lexer_DefineAtomic(?)
    Lexer_DefineVariable(?)
    Lexer_DefineConstant(?)
    Lexer_DefineKeyword(?)
    Lexer_DefineOperation(?)
    Lexer_DefineScopeStart(?)
    Lexer_DefineScopeEnd(?)
- we need to define compile-time vs runtime stuff (@TODO)
*/

#ifndef _KLEXER_H_
#define _KLEXER_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define LEXER_UNKNOWN           (1<<0) // Simply, unknown
#define LEXER_DIGIT             (1<<1) // A number based representation
#define LEXER_ALPHANUMERIC      (1<<2) // An alphanumerical representation

typedef void (*Tracer)(const char *msg, const char *file, const char *scope, intmax_t line, intmax_t offset);
typedef struct {
    Tracer tracer;
} Lexer;

Lexer   *Lexer_Init(const char *buffer);
void     Lexer_Destroy(Lexer *lexer);
void     Lexer_SetTracer(Lexer *lexer, Tracer tracer);

#ifdef __cplusplus
};
#endif // __cplusplus


#endif // _KLEXER_H_

#ifdef _KLEXER_IMPLEMENTATION

Lexer *Lexer_Init(const char *buffer)
{
    return nullptr;
}

void Lexer_Destroy(Lexer *lexer)
{
    if (lexer) {
        _KFREE(lexer);
    }
}

void Lexer_SetTracer(Lexer *lexer, Tracer tracer)
{
    _KASSERT(lexer);
    lexer->tracer = tracer;
}

#endif // _KLEXER_IMPLEMENTATION