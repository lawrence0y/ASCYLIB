/*
 * File:
 * Author(s):
 *   Vasileios Trigonakis
 */

#include "intset.h"
#include "utils.h"

__thread ssmem_allocator_t* alloc;

node_t*
new_node(skey_t key, sval_t val, node_t* l, node_t* r, int initializing)
{
  node_t* node;
#if GC == 1
  if (likely(!initializing))		/* for initialization AND the coupling algorithm */
    {
      node = (node_t*) ssmem_alloc(alloc, sizeof(node_t));
    }
  else
    {
      node = (node_t*) ssalloc(sizeof(node_t));
    }
#else
  node = (node_t*) ssalloc(sizeof(node_t));
#endif
  
  if (node == NULL) 
    {
      perror("malloc @ new_node");
      exit(1);
    }

  node->key = key;
  node->val = val;
  node->left = l;
  node->right = r;
  node->lock.to_uint64 = 0;

#if defined(__tile__)
  /* on tilera you may have store reordering causing the pointer to a new node
     to become visible, before the contents of the node are visible */
  MEM_BARRIER;
#endif	/* __tile__ */

  return (node_t*) node;
}


intset_t* set_new()
{
  intset_t *set;

  if ((set = (intset_t *)ssalloc_aligned(CACHE_LINE_SIZE, sizeof(intset_t))) == NULL) 
    {
      perror("malloc");
      exit(1);
    }

  node_t* min = new_node(INT_MIN, 1, NULL, NULL, 1);
  node_t* max = new_node(INT_MAX, 1, NULL, NULL, 1);
  set->head = new_node(INT_MAX, 0, min, max, 1);
  MEM_BARRIER;
  return set;
}

void
node_delete(node_t *node) 
{
#if GC == 1
  ssmem_free(alloc, node);
#else
  /* ssfree(node); */
#endif
}

void
set_delete_l(intset_t *set)
{
  /* TODO: implement */
}

static int
node_size(node_t* n)
{
 if (n->leaf != 0)
    {
      return 1;
    }
  else
    {
      return node_size((node_t*) n->left) + node_size((node_t*) n->right);
    }
}

int 
set_size(intset_t* set)
{
  int size = node_size(set->head) - 2;
  return size;
}



	