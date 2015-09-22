#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "chash.h"

long int_hash_func(void *key) {
    return (long)general_int_hash_func((long)key);
}

int int_compare_func(void *key1, void *key2) {
    return (long) key1 - (long) key2;
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

int main(int argc, char **argv) {

    struct chashtable *int_cht;
    int_cht = chash_init(64, int_compare_func, int_hash_func);
    chash_add(int_cht, (void*)1, (void*)1);
    printf("int value: %ld\n", (long)chash_get(int_cht, (void*)1));
    chash_del(int_cht, (void*)1);
    assert(chash_size(int_cht) == 0);
    chash_destroy(int_cht);

    struct chashtable *str_cht;
    str_cht = chash_init(32, str_compare_func, str_hash_func);
    chash_set_dup_func(str_cht, str_dup_key_func, str_dup_value_func);
    chash_set_free_func(str_cht, str_free_key_func, str_free_value_func);

    chash_add(str_cht, "abc", "value");
    printf("str value: %s\n", chash_get(int_cht, "abc"));
    chash_del(int_cht, "abc");
    assert(chash_size(str_cht) == 0);
    chash_destroy(str_cht);
    
    return 0;
}
