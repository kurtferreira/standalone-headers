/*
================================================================================

    A simple string handler to compensate for some shortfalls in C.

    Borrowed liberally from gingerBill
        (https://github.com/gingerBill/gb/blob/master/gb_string.h).

================================================================================
*/
#ifndef _KSTR_H_
#define _KSTR_H_

#include "string.h"
#include "types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    size_t len;
    size_t cap;
} fstr_header_t;

#define FSTR_HEADER(s) ((fstr_header_t *)s - 1)

#ifndef _NO_MALLOC // define to unset this to your own allocator
    #define FMALLOC(x) malloc(x)
    #define FFREE(x) free(x)
#endif

static inline char* fstr_create(char *init) {
    size_t len = strlen(init);
    size_t hdrlen = sizeof(fstr_header_t);

    void* allocated = FMALLOC(hdrlen + len + 1);

    char *str = (char*) allocated + hdrlen;
    fstr_header_t *hdr = FSTR_HEADER(str);
    hdr->len = len;
    hdr->cap = len;

    memcpy(str, init, len);
    str[len] = '\0';

    return str;
}

static inline char* fstr_create_empty(size_t cap) {
    size_t hdrlen = sizeof(fstr_header_t);

    void* allocated = FMALLOC(hdrlen + cap + 1);

    char *str = (char*) allocated + hdrlen;
    fstr_header_t *hdr = FSTR_HEADER(str);
    hdr->len = 0;
    hdr->cap = cap;

    str[0] = '\0';

    return str;
}

static inline void fstr_destroy(char* str) {
    fstr_header_t *hdr = FSTR_HEADER(str);

    if(hdr != null_t)
        FFREE(hdr);
}

static inline size_t fstr_len(char* str) {
    return FSTR_HEADER(str)->len;
}

static inline size_t fstr_cap(char* str) {
    return FSTR_HEADER(str)->cap;
}

static inline void fstr_clear(char* str) {
    FSTR_HEADER(str)->len = 0;
    str[0] = '\0';
}

static inline char * fstr_realloc(char* str, size_t cap) {
    assert(cap > 0);

    fstr_header_t *hdr = FSTR_HEADER(str);
    if(cap < hdr->cap) {
        hdr->len = cap;
        str[hdr->len] = '\0';
        return str;
    }

    char* strnew = fstr_create_empty(cap);
    memcpy(strnew, str, hdr->len);
    FSTR_HEADER(strnew)->len = hdr->len;
    strnew[hdr->len] = '\0';

    fstr_destroy(str);

    return strnew;
}

// will reallocate if necessary
static inline void* fstr_append(char* dst, char* src) {
    fstr_header_t *hdr = FSTR_HEADER(dst);
    size_t len = strlen(src);
    if(hdr->cap < hdr->len + len) {
        printf("too small, reallocating...\n");
        dst = fstr_realloc(dst, hdr->len + len);
        hdr = FSTR_HEADER(dst);
    }

    memcpy(dst + hdr->len, src, len);
    hdr->len += len;

    dst[hdr->len] = '\0';
    return dst;
}

static inline bool_t fstr_has_capacity(char* str, size_t len) {
    if(FSTR_HEADER(str)->cap > len) {
        return True;
    }
    return False;
}

static inline void fstr_trim_left(char* str) {
    fstr_header_t *hdr = FSTR_HEADER(str);
    size_t index = 0;

    while(str[index] == ' ') {
        index++;
    }

    hdr->len -= index;
    memcpy(str, str + index, hdr->len);
    str[hdr->len] = '\0';
}

static inline void fstr_trim_right(char* str) {
    fstr_header_t *hdr = FSTR_HEADER(str);
    size_t index = hdr->len - 1;

    while(index >= 0 && str[index] == ' ') {
        index--;
    }

    hdr->len = index + 1;
    str[hdr->len] = '\0';
}

static inline void fstr_trim(char* str) {
    fstr_trim_left(str);
    fstr_trim_right(str);
}

#ifdef __cplusplus
}
#endif

#endif
