/*
 * ***************************************************************
 * chash.c
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

#include <stdio.h>
#include <stdlib.h>
#include "chash.h"

#define MAX_REHASH_RATIO 2
#define MIN_CHASH_SIZE 32

static void check_rehash(struct chashtable *cht);

//============ hash function from redis ============
unsigned int general_int_hash_func(unsigned int key) {
    key += ~(key << 15); 
    key ^=  (key >> 10); 
    key +=  (key << 3);
    key ^=  (key >> 6);
    key += ~(key << 11); 
    key ^=  (key >> 16); 
    return key; 
}

static uint32_t chash_function_seed = 5381;
unsigned int general_str_hash_func(const void *key, int len) {
    /* 'm' and 'r' are mixing constants generated offline.
     They're not really 'magic', they just happen to work well.  */
    uint32_t seed = chash_function_seed;
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    /* Initialize the hash to a 'random' value */
    uint32_t h = seed ^ len;

    /* Mix 4 bytes at a time into the hash */
    const unsigned char *data = (const unsigned char *)key;

    while(len >= 4) {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    /* Handle the last few bytes of the input array  */
    switch(len) {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0]; h *= m;
    };

    /* Do a few final mixes of the hash to ensure the last few
     * bytes are well-incorporated. */
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return (unsigned int)h;
}

struct chashtable *chash_init(int size, compare_func cfunc,  hash_func hfunc) {
    if(!cfunc || !hfunc) return NULL;
     
    size = size > MIN_CHASH_SIZE ? size : MIN_CHASH_SIZE;

    struct chashtable *cht = malloc(sizeof(*cht));
    if( ! cht) return NULL;

    cht->used = 0;
    cht->size = size;
    cht->hash_func = hfunc;
    cht->comp_func = cfunc;

    cht->fk_func = NULL;
    cht->fv_func = NULL;
    cht->dk_func = NULL;
    cht->dv_func = NULL;

    cht->buckets = malloc(size * sizeof(void *));
    if( ! cht->buckets) {
        free(cht);
        return NULL;
    }
    int i;
    for(i = 0; i < size; i++) {
        cht->buckets[i] = NULL;
    }

    return cht;
}

void chash_set_free_func(struct chashtable *cht, 
    free_key_func fk_func, free_value_func fv_func) {
    cht->fk_func = fk_func;
    cht->fv_func = fv_func;
}

void chash_set_dup_func(struct chashtable *cht, 
    dup_key_func dk_func, dup_value_func dv_func) {
    cht->dk_func = dk_func;
    cht->dv_func = dv_func;
}

void chash_destroy(struct chashtable *cht) {
    if( ! cht ) return;
    
    int i;
    struct bucket *curr, *next;
    for(i = 0; i < cht->size; i++) {
        curr = cht->buckets[i];
        while(curr) {
            next = curr->next; 
            free(curr);
            curr = next;
        }
    }

    free(cht->buckets);
    free(cht);
}

uint64_t chash_size(struct chashtable *cht) {
    return cht->used;
}

void *chash_get(struct chashtable *cht, void *key) {
    if( ! cht ) return NULL;

    uint64_t ind;
    struct bucket *curr;

    ind = cht->hash_func(key) & (cht->size - 1);
    for(curr = cht->buckets[ind]; curr; curr = curr->next) {
        if(!cht->comp_func(curr->key, key)) {
            return curr->value;
        }
    }

    return NULL;
}

int chash_del(struct chashtable *cht, void *key) {
    if( ! cht ) return 0;

    uint64_t ind;
    struct bucket *curr, *prev;
    ind = cht->hash_func(key) & (cht->size - 1);
    for(curr = cht->buckets[ind], prev = curr; curr; curr = curr->next) {
        if(! cht->comp_func(curr->key, key)) {
            if(curr == cht->buckets[ind]) {
                cht->buckets[ind] = curr->next;
            } else {
                prev->next = curr->next;
            }

            if(cht->fk_func) {
                cht->fk_func(curr->key);
            }
            if(cht->fv_func) {
                cht->fv_func(curr->value);
            }
            free(curr);

            cht->used--;
            return 1;
        }
        prev = curr;
    }

    return 0;
}

static int _chash_add_internel(struct chashtable *cht, void *key, void *value, int force) {
    if( ! cht ) return 0;

    uint64_t ind;
    struct bucket *curr;

    check_rehash(cht);

    ind = cht->hash_func(key) & (cht->size - 1);
    for(curr = cht->buckets[ind]; curr; curr = curr->next) {
        if(!cht->comp_func(curr->key, key)) {
            if(!force) return 0;

            if(cht->fv_func) {
                cht->fv_func(curr->value);
            }
            if(cht->dv_func) {
                curr->value = cht->dv_func(value);
            } else {
                curr->value = value;
            }

            return 1;
        }
    }
    
    struct bucket *new_bucket = malloc(sizeof(*new_bucket));
    if(cht->dk_func) {
        new_bucket->key = cht->dk_func(key);
    } else {
        new_bucket->key = key;
    }
    if(cht->dv_func) {
        new_bucket->value = cht->dv_func(value);
    } else {
        new_bucket->value = value;
    }
    
    // insert into hashtable
    new_bucket->next = cht->buckets[ind];
    cht->buckets[ind] = new_bucket;
    cht->used++;

    return 1;
}

int chash_add(struct chashtable *cht, void *key, void *value) {
    return _chash_add_internel(cht, key, value, 0);
}

int chash_set(struct chashtable *cht, void *key, void *value) {
    return _chash_add_internel(cht, key, value, 1);
}

static void check_rehash(struct chashtable *cht) {
    if(cht->used / cht->size < MAX_REHASH_RATIO) {
        return;
    }

    int new_size = cht->size * 2;
    struct chashtable*new_cht;
    new_cht = chash_init(new_size, cht->comp_func, cht->hash_func);
    if(!new_cht) {
        return;
    }
    chash_set_dup_func(new_cht, cht->dk_func, cht->dv_func);
    chash_set_free_func(new_cht, cht->fk_func, cht->fv_func);

    int i; 
    uint64_t ind;
    struct bucket *curr, *next;
    for(i = 0; i < cht->size; i++)  {
        curr = cht->buckets[i]; 
        while(curr) {
            next = curr->next;
            ind = new_cht->hash_func(curr->key) & (new_size - 1); 
            curr->next = new_cht->buckets[ind];
            new_cht->buckets[ind] = curr;
            new_cht->used++;
            curr = next;
        }
    }
    
    // replace new buckets to old chashtbale.
    struct bucket **tmp_buckets;
    cht->size = new_size;
    tmp_buckets = cht->buckets;
    cht->buckets = new_cht->buckets;
    new_cht->buckets = tmp_buckets;

    free(new_cht->buckets);
    free(new_cht);
}
