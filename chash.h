/*
 * ***************************************************************
 * chash.h 
 * author by @git-hulk at 2015-09-22 
 * Copyright (C) 2015 Inc.
 * *************************************************************
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _CHASH_H_
#define _CHASH_H_
#include <stdint.h>

typedef long (*hash_func) (void *key);
typedef int (*compare_func) (void *key1, void *key2);
typedef void (*free_key_func) (void *key);
typedef void* (*dup_key_func) (void *key);
typedef void (*free_value_func) (void *value);
typedef void* (*dup_value_func) (void *value);

struct bucket {
    void *key;
    void *value;
    struct bucket *next;
};

struct chashtable {
    uint64_t size;
    uint64_t used; 
    struct bucket **buckets;
    hash_func hash_func;
    compare_func comp_func;
    free_key_func fk_func;
    free_value_func fv_func;
    dup_key_func dk_func;
    dup_value_func dv_func;
};

struct chashtable *chash_init(int buckets, compare_func cfunc, hash_func hfunc);
void set_free_func(struct chashtable *cht, free_key_func fk_func, free_value_func fv_func);
void set_dup_func(struct chashtable *cht, dup_key_func dk_func, dup_value_func dv_func);

void *chash_get(struct chashtable *cht, void *key);
int chash_del(struct chashtable *cht, void *key); 
int chash_add(struct chashtable *cht, void *key, void *value);
int chash_set(struct chashtable *cht, void *key, void *value);

#endif
