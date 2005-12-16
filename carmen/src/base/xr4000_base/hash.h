#ifndef __INC_hash_h
#define __INC_hash_h

#ifdef __cplusplus
extern "C" {
#endif

struct cache_node {
  struct cache_node *next;
  void *key;
  void *value;
};

typedef struct cache_node *node_ptr;

typedef struct cache {
  struct cache_node **node_table;
  unsigned int size;
  unsigned int used;
  unsigned int mask;
  unsigned int last_bucket;
  unsigned int (*hash_func)();
  int (*compare_func)();
} *cache_ptr;

typedef unsigned int (*hash_func_type)(); 
typedef int (*compare_func_type)(); 

void hash_add(struct cache **, void *, void *);
void *hash_change_key_value(struct cache *, void *, void *);
void hash_delete(struct cache *);
struct cache *hash_new(unsigned int, unsigned int (*)(), int (*)());
struct cache_node *hash_next(struct cache *, struct cache_node *);
void hash_remove(struct cache *, void *);
void *hash_value_for_key(struct cache *, void *);

#ifdef __cplusplus
}
#endif

#endif /* __INC_hash_h */
