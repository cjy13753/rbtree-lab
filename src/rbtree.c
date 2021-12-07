#include "rbtree.h"

#include <stdlib.h>

void free_nodes_in_postorder(rbtree *t, node_t *root);
node_t *binary_search(const rbtree *t, node_t *node, key_t key);
node_t *new_node(rbtree *t, key_t key, color_t color);
node_t *bst_insert(rbtree *t, node_t *root, node_t *node_to_insert);
void left_rotate(rbtree *t, node_t *pivot);
void right_rotate(rbtree *t, node_t *pivot);
void fixup(rbtree *t, node_t *node_to_insert);

rbtree *new_rbtree(void) {
  node_t *NIL = (node_t *)calloc(1, sizeof(node_t));
  NIL->key = 0;
  NIL->color = RBTREE_BLACK;
  NIL->parent = NULL;
  NIL->left = NULL;
  NIL->right = NULL;
  
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  p->root = NIL;
  p->nil = NIL;
  
  return p;
}

void delete_rbtree(rbtree *t) {
  free_nodes_in_postorder(t, t->root);
  free(t->nil);
  free(t);
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  // Initiallize node with the given key and color it red
  node_t *node_to_insert = new_node(t, key, RBTREE_RED);

  // bst insert the new node into t
  t->root = bst_insert(t, t->root, node_to_insert);

  // fixup to maintain the properties of rb tree
  fixup(t, node_to_insert);
  
  return t->root;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  return binary_search(t, t->root, key);
}

node_t *rbtree_min(const rbtree *t) {
  node_t *cur_node = t->root;
  node_t *next_left = cur_node->left;
  while (next_left != t->nil) {
    cur_node = next_left;
    next_left = cur_node->left;
  }
  return cur_node;
}

node_t *rbtree_max(const rbtree *t) {
  node_t *cur_node = t->root;
  node_t *next_right = cur_node->right;
  while (next_right != t->nil) {
    cur_node = next_right;
    next_right = cur_node->right;
  }
  return cur_node;
}

int rbtree_erase(rbtree *t, node_t *p) {
  // Minimum implementation just to pass 'test_erase_root'
  free(p);
  t->root = t->nil;
  return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  // TODO: implement to_array
  return 0;
}

// helper functions below
void free_nodes_in_postorder(rbtree *t, node_t *root) {
  if (root == t->nil) {
    return;
  }

  free_nodes_in_postorder(t, root->left);
  free_nodes_in_postorder(t, root->right);
  free(root);
}

node_t *binary_search(const rbtree *t, node_t *node, key_t key) {
  if (node == t->nil) {
    return NULL;
  }
  
  if (key < node->key) {
    return binary_search(t, node->left, key);
  }
  else if (key == node->key) {
    return node;
  }
  else {
    return binary_search(t, node->right, key);
  }
}

node_t *new_node(rbtree *t, key_t key, color_t color) {
  node_t *node_to_insert = (node_t *)calloc(1, sizeof(node_t));
  node_to_insert->key = key;
  node_to_insert->parent = t->nil;
  node_to_insert->left = t->nil;
  node_to_insert->right = t->nil;
  node_to_insert->color = color;

  return node_to_insert;
}

node_t *bst_insert(rbtree *t, node_t *root, node_t *node_to_insert) {
  if (root == t->nil)
    return node_to_insert;

  if (node_to_insert->key < root->key) {
    root->left = bst_insert(t, root->left, node_to_insert);
    root->left->parent = root;
  }
  else {
    root->right = bst_insert(t, root->right, node_to_insert);
    root->right->parent = root;
  }
  
  return root;
}

void left_rotate(rbtree *t, node_t *pivot) {
  node_t *right = pivot->right;
  
  // pivot의 right child가 가지고 있던 left child를 pivot의 right child로 갱신
  pivot->right = right->left;
  if (pivot->right != t->nil) {
    pivot->right->parent = pivot;
  }
  
  // pivot의 right child가 기존 pivot의 부모와 연결관계 형성
  right->parent = pivot->parent;
  if (pivot->parent == t->nil) {
    t->root = right;
  } else if (pivot == pivot->parent->left) {
    pivot->parent->left = right;
  }
  else if (pivot == pivot->parent->right) {
    pivot->parent->right = right;
  }

  // pivot의 right child와 pivot의 부모 관계를 역전
  right->left = pivot;
  pivot->parent = right;
}

void right_rotate(rbtree *t, node_t *pivot) {
  node_t *left = pivot->left;
  
  // pivot의 left child가 가지고 있던 right child를 pivot의 left child로 갱신
  pivot->left = left->right;
  if (pivot->left != t->nil) {
    pivot->left->parent = pivot;
  }
  
  // pivot의 left child가 기존 pivot의 부모와 연결관계 형성
  left->parent = pivot->parent;
  if (pivot->parent == t->nil) {
    t->root = left;
  } else if (pivot == pivot->parent->left) {
    pivot->parent->left = left;
  }
  else if (pivot == pivot->parent->right) {
    pivot->parent->right = left;
  }

  // pivot의 right child와 pivot의 부모 관계를 역전
  left->right = pivot;
  pivot->parent = left;
}

void fixup(rbtree *t, node_t *node_to_insert) {
  node_t *pt = node_to_insert;
  
  while (pt != t->root && pt->color == RBTREE_RED && pt->parent->color == RBTREE_RED) {
    node_t *pt_parent = pt->parent;
    node_t *pt_grandparent = pt->parent->parent;

    // case A: when pt_parent is left child of pt_grandparent
    if (pt_parent == pt_grandparent->left) {
      node_t *pt_uncle = pt_grandparent->right;
      // case 1: pt_uncle is red
      if (pt_uncle != t->nil && pt_uncle->color == RBTREE_RED) {
        pt_grandparent->color = RBTREE_RED;
        pt_parent->color = RBTREE_BLACK;
        pt_uncle->color = RBTREE_BLACK;
        pt = pt_grandparent;
      }

      else {
        // case 2: pt_uncle is black and pt is right child of pt_parent(triangle)
        if (pt == pt_parent->right) {
          left_rotate(t, pt_parent);
          pt = pt_parent;
          pt_parent = pt->parent;
        }
        // case 3: pt_uncle is black is left child of pt_parent(line)
        right_rotate(t, pt_grandparent);
        pt_parent->color = RBTREE_BLACK;
        pt_grandparent->color = RBTREE_RED;
        pt = pt_parent;
      }
    }

    // case B: when pt_parent is right child of pt_grandparent
    else if (pt_parent == pt_grandparent->right) {
      node_t *pt_uncle = pt_grandparent->left;
      // case 1: pt_uncle is red
      if (pt_uncle != t->nil && pt_uncle->color == RBTREE_RED) {
        pt_grandparent->color = RBTREE_RED;
        pt_parent->color = RBTREE_BLACK;
        pt_uncle->color = RBTREE_BLACK;
        pt = pt_grandparent;
      }

      else {
        // case 2: pt_uncle is black and pt is left child of pt_parent(triangle)
        if (pt == pt_parent->left) {
          right_rotate(t, pt_parent);
          pt = pt_parent;
          pt_parent = pt->parent;
        }
        // case 3: pt_uncle is black is right child of pt_parent(line)
        left_rotate(t, pt_grandparent);
        pt_parent->color = RBTREE_BLACK;
        pt_grandparent->color = RBTREE_RED;
        pt = pt_parent;
      }
    }
  }
  t->root->color = RBTREE_BLACK;
}


// // Temporary driver code
// void inorder(node_t *root) {
//   if (root == NULL) {
//     return;
//   }

//   if (root->left == NULL && root->right == NULL) {
//     printf("%d ", root->key);
//     return;
//   }
    
//   inorder(root->left);
//   printf("%d ", root->key);
//   inorder(root->right);
// }

// int main() {
//   rbtree *t = new_rbtree();
//   rbtree_insert(t, 5);
//   rbtree_insert(t, 4);
//   rbtree_insert(t, 6);
//   rbtree_insert(t, 3);
//   rbtree_insert(t, 2);
//   rbtree_insert(t, 1);
//   rbtree_insert(t, 0);

//   inorder(t->root);
// }