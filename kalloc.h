/*
 * This file is part of Clibs
 *
 * Copyright (C) 2025 Kurt Ferreira
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 */

/// Usage:
/// #define USE_KALLOC              // this will use memory tracking (not defining will use malloc/calloc/free)
/// #define KALLOC_IMPLEMENTATION   // Define this in a single file
/// #include "kalloc.h"

#ifndef KALLOC_H
#define KALLOC_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void*   kmem_alloc(size_t bytes, const char *file, const char *func, size_t line);
void*   kmem_calloc(size_t num_items, size_t bytes, const char *file, const char *func, size_t line);
bool    kmem_leaks();
void    kmem_print_leaks();
void    kmem_free(void *ptr, const char *file, size_t line);

#ifdef USE_KALLOC
    #define __alloc(x)      kmem_alloc(x, __FILE__, __func__, __LINE__)
    #define __calloc(x,y)   kmem_calloc(x, y, __FILE__, __func__, __LINE__)
    #define __free(x)       kmem_free(x, __FILE__, __func__, __LINE__)
    #define __leaks()       kmem_leaks()
    #define __print_leaks() kmem_print_leaks()
#else
    #define __alloc(x)      malloc(x)
    #define __calloc(x,y)   calloc(x,y)
    #define __free(x)       if(x) { free(x); }
    #define __leaks()       0
    #define __print_leaks() 0
#endif

#ifdef __cplusplus
};
#endif

#endif //KALLOC_H

#ifdef KALLOC_IMPLEMENTATION

void _kmem_append(void* ptr, size_t size, const char *file, const char *func, size_t line);
char * _kmem_bytes(size_t size);

typedef struct {
    void *ptr;
    size_t size;
    bool freed;
    const char *file;
    const char *func;
    size_t line;
} kmem_allocation_t;

struct {
    kmem_allocation_t *items;
    size_t capacity;
    size_t count;
} kmem_allocation_state = {
    .items = nullptr,
    .count =  0,
    .capacity = -1
};

inline char * _kmem_bytes(size_t size)
{
    char *buffer = malloc(128);

    const char *sizes[] = { "B", "KB", "MB", "GB", "TB", "PB" };
    double len = (double)size;
    int order = 0;

    while (len >= 1024 && order < (int)(sizeof(sizes) / sizeof(*sizes)) - 1) {
        order++;
        len /= 1024;
    }

    snprintf(buffer, 128, "%.2f %s", len, sizes[order]);

    return buffer;
}
inline void _kmem_append(void* ptr, size_t size, const char * file, const char * func, size_t line)
{
    if ( kmem_allocation_state.capacity == -1 ) {
        kmem_allocation_state.capacity = 256;
        kmem_allocation_state.items = (void*) malloc(kmem_allocation_state.capacity * sizeof(void *));
    }

    if ( kmem_allocation_state.capacity < kmem_allocation_state.count ) {
        kmem_allocation_state.capacity *= 2;
        kmem_allocation_state.items = realloc(kmem_allocation_state.items, kmem_allocation_state.capacity);
    }

    kmem_allocation_state.items[kmem_allocation_state.count++] = (kmem_allocation_t) {
        .ptr = ptr,
        .size = size,
        .freed = false,
        .file = file,
        .line = line,
        .func = func

    };
}

inline void *kmem_alloc(size_t size, const char *file, const char *func, size_t line)
{
    void *ptr = malloc(size);

    if ( ptr ) {
        _kmem_append(ptr, size, file, func, line);

        return ptr;
    }

    return nullptr;
}

inline void *kmem_calloc(size_t num_items, size_t size, const char *file, const char *func, size_t line)
{
    void *ptr = calloc(num_items, size);

    if ( ptr ) {
        _kmem_append(ptr, size, file, func, line);
        return ptr;
    }
    return nullptr;
}

inline void kmem_free(void *ptr, const char *file, size_t line)
{
    if ( ptr ) {
        for ( size_t i = 0; i < kmem_allocation_state.count; i++ ) {

            if ( kmem_allocation_state.items[i].ptr == ptr && !kmem_allocation_state.items[i].freed ) {
                kmem_allocation_state.items[i].freed = true;
                kmem_allocation_state.items[i].ptr = nullptr;
                kmem_allocation_state.items[i].file = "";
                kmem_allocation_state.items[i].func = "";
                kmem_allocation_state.items[i].line = 0;
                kmem_allocation_state.count--;
                break;
            }
        }

        free(ptr);
    }
}

inline void kmem_print_leaks()
{
    size_t count = 0;
    size_t allocated = 0;
    printf("\nMemory Leaks:\n");

    for ( size_t i = 0; i < kmem_allocation_state.count; i++ ) {
        if ( !kmem_allocation_state.items[i].freed ) {
            char * b = _kmem_bytes(kmem_allocation_state.items[i].size);
            printf("- %s (%s on line %zu): Leak at 0x%zu (size %s))\n",
                kmem_allocation_state.items[i].file,
                kmem_allocation_state.items[i].func,
                kmem_allocation_state.items[i].line,
                (size_t)kmem_allocation_state.items[i].ptr,
                b);
            allocated += kmem_allocation_state.items[i].size;
            count++;
            free(b);
        }
    }
    printf("\n--------------------------------------------------\n");
    printf("Total allocations not freed: %zu\n", count);
    char *b = _kmem_bytes(allocated);
    printf("Total memory not freed: %s\n", b);
    free(b);
    printf("--------------------------------------------------\n");
}

inline bool kmem_leaks()
{
    if ( kmem_allocation_state.count > 0 ) {
        return true;
    }

    return false;
}

#endif

