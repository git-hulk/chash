#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "chash.h"


long int_hash_func(void *key) {
    return (long)general_int_hash_func((long)key);
}

int int_compare_func(void *key1, void *key2) {
    return (long) key1 - (long) key2;
}

void int_hash_test() {
    struct chashtable *int_cht = chash_init(64, int_compare_func, int_hash_func);

    long i, count = 1000;
    for(i = 0; i < count; i++) {
        chash_add(int_cht, (void*)i, (void*)i);
    }
    assert(chash_size(int_cht) == count);
    for(i = 0; i < count; i++) {
        long v = (long) chash_get(int_cht, (void*)i);
        assert(v == i);
    }

    for(i = 0; i < count; i++) {
        chash_set(int_cht, (void*)i, (void*)(i + 1));
    }
    assert(chash_size(int_cht) == count);
    for(i = 0; i < count; i++) {
        long v = (long) chash_get(int_cht, (void*)i);
        assert(v == i+1);
    }

    for(i = 0; i < count; i++) {
        chash_del(int_cht, (void*)i);
    }
    assert(chash_size(int_cht) == 0);

    chash_destroy(int_cht);
}

long str_hash_func(void *key) {
    return general_str_hash_func(key, strlen((char*)key));
}

int str_compare_func(void *key1, void *key2) {
    char *k1 = (char *) key1;
    char *k2 = (char *) key2;
    if(strlen(k1) == strlen(k2) 
        && !strncmp(k1, k2, strlen(k1))) {
        return 0;
    } else {
        return -1;
    }
}

void *str_dup_key_func(void *key) {
    return strdup((char*)key);
}
void *str_dup_value_func(void * value) {
    return strdup((char*)value);
}

void str_free_key_func(void *key) {
    free((char*)key);
}

void str_free_value_func(void *value) {
    free((char*)value);
}

void str_hash_test() {
    char key_buf[64];
    char val_buf[64];
    struct chashtable *str_cht;
    str_cht = chash_init(32, str_compare_func, str_hash_func);
    chash_set_dup_func(str_cht, str_dup_key_func, str_dup_value_func);
    chash_set_free_func(str_cht, str_free_key_func, str_free_value_func);

    long i, count = 2000;
    for(i = 0; i < count; i++) {
        snprintf(key_buf, 64, "_chash_key_%ld", i);
        snprintf(val_buf, 64, "_chash_val_%ld", i);
        chash_add(str_cht, key_buf, val_buf);
    }
    assert(chash_size(str_cht) == count);

    for(i = 0; i < count; i++) {
        snprintf(key_buf, 64, "_chash_key_%ld", i);
        snprintf(val_buf, 64, "_chash_val_%ld", i);
        char *v = (char*) chash_get(str_cht, key_buf);
        assert(!strcmp(v, val_buf));
    }

    for(i = 0; i < count; i++) {
        snprintf(key_buf, 64, "_chash_key_%ld", i);
        snprintf(val_buf, 64, "_chash_val_%ld", i + 1);
        chash_set(str_cht, key_buf, val_buf);
    }
    assert(chash_size(str_cht) == count);

    for(i = 0; i < count; i++) {
        snprintf(key_buf, 64, "_chash_key_%ld", i);
        snprintf(val_buf, 64, "_chash_val_%ld", i + 1);
        char *v = (char*) chash_get(str_cht, key_buf);
        assert(!strcmp(v, val_buf));
    }

    for(i = count-1; i >= 0; i--) {
        snprintf(key_buf, 64, "_chash_key_%ld", i);
        chash_del(str_cht, key_buf);
    }
    assert(chash_size(str_cht) == 0);

    chash_destroy(str_cht);
}

int main(int argc, char **argv) {
    int_hash_test();
    str_hash_test();
    printf("========== UNIT TEST SUCC. =========\n");
    return 0;
}
