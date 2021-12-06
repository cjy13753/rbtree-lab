#include "rbtree.h"

#include <stdlib.h>

node_t *binary_search(node_t *node, key_t key);
node_t *new_node(key_t data);
node_t *bst_insert(node_t *root, node_t *node_to_insert);
void left_rotate(rbtree *t, node_t *pivot);
void right_rotate(rbtree *t, node_t *pivot);
void fixup(rbtree *t, node_t *node_to_insert);

rbtree *new_rbtree(void) {
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  p->root = NULL;
  p->nil = NULL;
  
  return p;
}

void delete_rbtree(rbtree *t) {
  // TODO: reclaim the tree nodes's memory
  free(t);
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  // Initiallize node with the given key and color it red
  node_t *node_to_insert = new_node(key);

  // bst insert the new node into t
  t->root = bst_insert(t->root, node_to_insert);

  // fixup to maintain the properties of rb tree
  fixup(t, node_to_insert);
  
  return t->root;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  return binary_search(t->root, key);
}

node_t *rbtree_min(const rbtree *t) {
  // TODO: implement find
  return t->root;
}

node_t *rbtree_max(const rbtree *t) {
  // TODO: implement find
  return t->root;
}

int rbtree_erase(rbtree *t, node_t *p) {
  // TODO: implement erase
  return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  // TODO: implement to_array
  return 0;
}

// helper functions below
node_t *binary_search(node_t *node, key_t key) {
  if (node == NULL) {
    return NULL;
  }
  
  if (key < node->key) {
    return binary_search(node->left, key);
  }
  else if (key == node->key) {
    return node;
  }
  else {
    return binary_search(node->right, key);
  }
}

node_t *new_node(key_t key) {
  node_t *node_to_insert = (node_t *)calloc(1, sizeof(node_t));
  node_to_insert->key = key;
  node_to_insert->parent = NULL;
  node_to_insert->left = NULL;
  node_to_insert->right = NULL;
  node_to_insert->color = RBTREE_RED;

  return node_to_insert;
}

node_t *bst_insert(node_t *root, node_t *node_to_insert) {
  if (root == NULL)
    return node_to_insert;

  if (node_to_insert->key < root->key) {
    root->left = bst_insert(root->left, node_to_insert);
    root->left->parent = root;
  }
  else {
    root->right = bst_insert(root->right, node_to_insert);
    root->right->parent = root;
  }
  
  return root;
}

void left_rotate(rbtree *t, node_t *pivot) {
  node_t *right = pivot->right;
  
  // pivot의 right child가 가지고 있던 left child를 pivot의 right child로 갱신
  pivot->right = right->left;
  if (pivot->right != NULL) {
    pivot->right->parent = pivot;
  }
  
  // pivot의 right child가 기존 pivot의 부모와 연결관계 형성
  right->parent = pivot->parent;
  if (pivot->parent == NULL) {
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
  if (pivot->left != NULL) {
    pivot->left->parent = pivot;
  }
  
  // pivot의 left child가 기존 pivot의 부모와 연결관계 형성
  left->parent = pivot->parent;
  if (pivot->parent == NULL) {
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
    node_t *pt_parent = node_to_insert->parent;
    node_t *pt_grandparent = pt->parent->parent;

    // case A: when pt_parent is left child of pt_grandparent
    if (pt_parent == pt_grandparent->left) {
      node_t *pt_uncle = pt_grandparent->right;
      // case 1: pt_uncle is red
      if (pt_uncle != NULL && pt_uncle->color == RBTREE_RED) {
        pt_grandparent->color = RBTREE_RED;
        pt_parent->color = RBTREE_BLACK;
        pt_uncle->color = RBTREE_BLACK;
        pt = pt_grandparent;
      }

      // case 2: pt_uncle is black and pt is right child of pt_parent(triangle)
      else if (pt == pt_parent->right) {
        left_rotate(t, pt_parent);
        pt = pt_parent;
        pt_parent = pt->parent;
      }
      // case 3: pt_uncle is black is left child of pt_parent(line)
      else {
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
      if (pt_uncle != NULL && pt_uncle->color == RBTREE_RED) {
        pt_grandparent->color = RBTREE_RED;
        pt_parent->color = RBTREE_BLACK;
        pt_uncle->color = RBTREE_BLACK;
        pt = pt_grandparent;
      }

      // case 2: pt_uncle is black and pt is left child of pt_parent(triangle)
      else if (pt == pt_parent->left) {
        right_rotate(t, pt_parent);
        pt = pt_parent;
        pt_parent = pt->parent;
      }
      // case 3: pt_uncle is black is right child of pt_parent(line)
      else {
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