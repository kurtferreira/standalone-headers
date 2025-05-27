/*
 * This file is part of the single-file header lexer library
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
- we need to define compile-time vs runtime stuff (@TODO)

*/