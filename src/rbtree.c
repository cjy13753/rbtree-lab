#include "rbtree.h"

#include <stdlib.h>

void rb_delete_fixup(rbtree *t, node_t *x);
int inorder_toarray(const rbtree *t, key_t *arr, node_t *root, int ticket);
void rb_transplant(rbtree *t, node_t *u, node_t *v);
node_t *tree_minimum(rbtree *t, node_t *root);
void free_nodes_in_postorder(rbtree *t, node_t *root);
node_t *binary_search(const rbtree *t, node_t *node, key_t key);
node_t *new_node(rbtree *t, key_t key, color_t color);
node_t *bst_insert(rbtree *t, node_t *root, node_t *node_to_insert);
void left_rotate(rbtree *t, node_t *pivot);
void right_rotate(rbtree *t, node_t *pivot);
void rb_insert_fixup(rbtree *t, node_t *node_to_insert);

/*
  1. Implementation 요구되는 functions
*/
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
  rb_insert_fixup(t, node_to_insert);
  
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

int rbtree_erase(rbtree *t, node_t *node_to_delete) {
  // 이해의 편의를 위해 Introduction to algorithm에 나오는 pseudo code의 변수명과 일부러 다르게 수정했다.
  // (1) y는 node_to_delete를 대체할 node이고,
  // (2) y_child는 y의 자식 노드로서 y를 대체할 node이다.
  // 위 두 가지 사항을 명심하면 아래 코드가 매끄럽게 논리적으로 연결되는 것을 이해할 수 있다.
  node_t *y;
  color_t y_original_color;
  node_t *y_child;
  
  // node_to_delete의 왼쪽 자식이 nil인 경우
  // 즉 (1) 자식 node가 아예 없거나, (2) 오른쪽 자식만 있는 경우
  if (node_to_delete->left == t->nil) {
    y = node_to_delete; // y가 node_to_delete를 가리키게 함으로써 대체한 것으로 간주한다.
    y_original_color = y->color;
    y_child = y->right;
    rb_transplant(t, y, y_child);
  }
  // node_to_delete가 왼쪽 자식만 있는 경우
  else if (node_to_delete->right == t->nil) {
    y = node_to_delete; // y가 node_to_delete를 가리키게 함으로써 대체한 것으로 간주한다.
    y_original_color = y->color;
    y_child = y->left;
    rb_transplant(t, y, y_child);
  }
  // node_to_delete가 양쪽 자식 모두 가진 경우
  else {
    y = tree_minimum(t, node_to_delete->right); // node_to_delete의 inorder successor를 y로 지정한다.
    y_original_color = y->color;
    y_child = y->right;
    // y가 node_to_delete의 direct right child인 경우
    if (y->parent == node_to_delete) {
      // y_child가 nil인 경우 fixup에서 문제 생길 수 있는 것에 대비. y_child가 non-nil child면 필요 없음.
      // fixup 때 sibling이 중요 변수여서, y_child의 부모를 참조해야 하는데, y_child가 nil이면 부모가 NULL일 것이라서 문제
      y_child->parent = y; 
    }
    // y가 node_to_delete의 direct right child가 아닌 경우
    else {
      rb_transplant(t, y, y_child);
      y->right = node_to_delete->right; // y의 우측 child가 변경됨
      y->right->parent = y;
    }
    // 직전까지의 코드가 y가 node_to_delete의 right subtree를 계승받는 작업이었다면
    // 아래의 코드는 y가 node_to_delete의 색깔과 부모를 계승받고, left subtree를 계승받는 작업
    rb_transplant(t, node_to_delete, y); // transplant가 부모를 계승받는 작업이다
    y->left = node_to_delete->left;
    y->left->parent = y;
    y->color = node_to_delete->color;
  }
  free(node_to_delete); // 부모, 좌, 우 연결고리를 잃어버린 node_to_delete을 free해주기
  if (y_original_color == RBTREE_BLACK) { //y_original_color가 red면 black height에 영향을 안 주지만, black이면 문제가 생길 수 있기 때문
    rb_delete_fixup(t, y_child);
  }
  return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  inorder_toarray(t, arr, t->root, 0);
  return 0;
}


/* 
  2. helper functions below 
*/

int inorder_toarray(const rbtree *t, key_t *arr, node_t *root, int ticket) {
  if (root == t->nil) {
    return ticket;
  }

  ticket = inorder_toarray(t, arr, root->left, ticket);
  *(arr + ticket) = root->key;
  ticket = inorder_toarray(t, arr, root->right, ++ticket);
  return ticket;
}

// 부모 관계만 계승해준다. 양쪽 자식과의 관계는 별도로 계승작업을 해줘야 한다.
void rb_transplant(rbtree *t, node_t *node_to_transplant, node_t *replacement) {
  if (node_to_transplant->parent == t->nil) {
    t->root = replacement;
  }
  else if (node_to_transplant == node_to_transplant->parent->left) {
    node_to_transplant->parent->left = replacement;
  }
  else {
    node_to_transplant->parent->right = replacement;
  }
  replacement->parent = node_to_transplant->parent;
}

node_t *tree_minimum(rbtree *t, node_t *successor_node) {
  while (successor_node->left != t->nil) {
    successor_node = successor_node->left;
  }
  return successor_node;
}

// broken_node 변수는 Introduction to algorithm 교과서에서의 x 변수, sibling은 w이다.
// broken_node는 기본적으로 가지고 있는 색에 더해 black 색깔을 하나 더 가지고 있다고 가정한다.
// 만약 broken_node의 색은 color attribute가 red라면 black-red,
// color attribute가 black이라면 doubly-black이라고 가정한다.
// 위와 같은 사고방식은 Introduction to algorithm에서 소개한 방식으로, property 5가 깨지는 걸 property 1이 깨지는 것으로 치환하는 효과를 가진다.
// broken_node라고 명명한 이유는 property 1을 깨뜨리기 때문이다.
void rb_delete_fixup(rbtree *t, node_t *broken_node) {
  // broken_node가 doubly-black인 경우에만 아래의 case들을 진행한다.
  while (broken_node != t->root && broken_node->color == RBTREE_BLACK) {
    // Insertion 때와 비슷하게, broken_node가 parent의 좌측 child인지, 우측 child인지로 크게 경우를 나눈다.
    if (broken_node == broken_node->parent->left) {
      node_t *sibling = broken_node->parent->right;
      // case 1: sibling이 red인 경우
      // case 1이 종료된 이후 새롭게 지정된 sibling은 반드시 black이기 때문에 case 2, case 3, 혹은 case 4로 넘어간다.
      if (sibling->color == RBTREE_RED) {
        sibling->color = RBTREE_BLACK;
        broken_node->parent->color = RBTREE_RED;
        left_rotate(t, broken_node->parent);
        sibling = broken_node->parent->right;
      }
      // case 2: sibling이 black이면서, sibling의 모든 자식들이 black인 경우
      // 만약 case 1에서 case 2로 넘어왔다면 새로운 broken_node는 black-red이기 때문에 case 2 종료 이후 while loop이 종료된다.
      if (sibling->left->color == RBTREE_BLACK && sibling->right->color == RBTREE_BLACK) {
        sibling->color = RBTREE_RED;
        broken_node = broken_node->parent;
      }
      else {
        // case 3: sibling이 black이면서, sibling의 left child는 red, right child는 black인 경우
        // case 3가 종료되면 반드시 case 4로 넘어간 후 while loop이 종료된다.
        if (sibling->right->color == RBTREE_BLACK) {
          sibling->left->color = RBTREE_BLACK;
          sibling->color = RBTREE_RED;
          right_rotate(t, sibling);
          sibling = broken_node->parent->right;
        }
        // case 4: sibling이 black이면서, sibling의 left child는 모르고, right child는 red인 경우
        sibling->color = broken_node->parent->color;
        broken_node->parent->color = RBTREE_BLACK;
        sibling->right->color = RBTREE_BLACK;
        left_rotate(t, broken_node->parent);
        broken_node = t->root;
      }
    }
    // broken_node가 parent의 left-child일 때와 완전히 대칭적으로 반대이다.
    else {
      node_t *sibling = broken_node->parent->left;
      if (sibling->color == RBTREE_RED) {
        sibling->color = RBTREE_BLACK;
        broken_node->parent->color = RBTREE_RED;
        right_rotate(t, broken_node->parent);
        sibling = broken_node->parent->left;
      }
      if (sibling->right->color == RBTREE_BLACK && sibling->left->color == RBTREE_BLACK) {
        sibling->color = RBTREE_RED;
        broken_node = broken_node->parent;
      }
      else {
        if (sibling->left->color == RBTREE_BLACK) {
          sibling->right->color = RBTREE_BLACK;
          sibling->color = RBTREE_RED;
          left_rotate(t, sibling);
          sibling = broken_node->parent->left;
        }
        sibling->color = broken_node->parent->color;
        broken_node->parent->color = RBTREE_BLACK;
        sibling->left->color = RBTREE_BLACK;
        right_rotate(t, broken_node->parent);
        broken_node = t->root;
      }
    }
  }
  // x가 black-red인 상황에서, x를 simple black으로 만들어주면 property 1이 회복됨과 동시에 모든 rb tree property가 지켜지게 된다. 
  broken_node->color = RBTREE_BLACK;
}

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

void rb_insert_fixup(rbtree *t, node_t *node_to_insert) {
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
        // case 3가 진행되면 while loop는 반드시 종료된다.
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