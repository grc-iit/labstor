//
// Created by lukemartinlogan on 6/7/21.
//

#include <labstor/interfaces/stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <dlfcn.h>

/**
 * PROTOTYPES
 * */

#define WRAPPER_FUN(x) x
#define REAL_FUN(x) __real_##x
#define FNAME_TYPE(T, fname, ...) T (*)(__VA_ARGS__)
#define FORWARD_DECL(T, fname, ...) \
    T (*REAL_FUN(fname))(__VA_ARGS__) = NULL;
#define GETFUN(T, fname, ...) \
    if(!REAL_FUN(fname)) { REAL_FUN(fname) = (FNAME_TYPE(T, fname, __VA_ARGS__))dlsym(RTLD_NEXT, #fname); }


FORWARD_DECL(size_t, fwrite, const void *ptr, size_t size, size_t nmemb, FILE *stream)
FORWARD_DECL(size_t, fread, void *ptr, size_t size, size_t nmemb, FILE *stream)
FORWARD_DECL(int, fputc, int c, FILE *stream)
FORWARD_DECL(int, putw, int w, FILE *stream)
FORWARD_DECL(int, fgetc, FILE *stream)
FORWARD_DECL(int, getw, FILE *stream)

/**
 * STDIO FUNCTIONS
 * */

size_t WRAPPER_FUN(fwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return REAL_FUN(fwrite)(ptr, size, nmemb, stream);
}

size_t WRAPPER_FUN(fread)(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return REAL_FUN(fread)(ptr, size, nmemb, stream);
}

int WRAPPER_FUN(fputc)(int c, FILE *stream) {
    return REAL_FUN(fputc)(c, stream);
}
int WRAPPER_FUN(putw)(int w, FILE *stream) {
    return REAL_FUN(putw)(w, stream);
}
int WRAPPER_FUN(fgetc)(FILE *stream) {
    return REAL_FUN(fgetc)(stream);
}
int WRAPPER_FUN(getw)(FILE *stream) {
    return REAL_FUN(getw)(stream);
}