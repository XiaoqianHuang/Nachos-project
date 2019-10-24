#include "rb_tree_nachos.h"
/*

Node::Node(Thread *data) {
  this->data = data;
  color = RED;
  left = right = parent = NULL;
}

RBTree::RBTree() {
  root = NULL;
}

int RBTree::getColor(Node *&node) {
  if (node == NULL)
    return BLACK;

  return node->color;
}

void RBTree::setColor(Node *&node, int color) {
  if (node == NULL)
    return;

  node->color = color;
}

Node* RBTree::insertBST(Node *&root, Node *&ptr) {
  if (root == NULL)
    return ptr;

  if (ptr->data->getVirtualRunTime() < root->data->getVirtualRunTime()) {
    root->left = insertBST(root->left, ptr);
    root->left->parent = root;
  }
  else if (ptr->data->getVirtualRunTime() > root->data->getVirtualRunTime()) {
    root->right = insertBST(root->right, ptr);
    root->right->parent = root;
  }

  return root;
}

void RBTree::insertValue(Thread* n) {
  Node *node = new Node(n);
  root = insertBST(root, node);
  fixInsertRBTree(node);
}

void RBTree::rotateLeft(Node *&ptr) {
  Node *right_child = ptr->right;
  ptr->right = right_child->left;

  if (ptr->right != NULL)
    ptr->right->parent = ptr;

  right_child->parent = ptr->parent;

  if (ptr->parent == NULL)
    root = right_child;
  else if (ptr == ptr->parent->left)
    ptr->parent->left = right_child;
  else
    ptr->parent->right = right_child;

  right_child->left = ptr;
  ptr->parent = right_child;
}

void RBTree::rotateRight(Node *&ptr) {
  Node *left_child = ptr->left;
  ptr->left = left_child->right;

  if (ptr->left != NULL)
    ptr->left->parent = ptr;

  left_child->parent = ptr->parent;

  if (ptr->parent == NULL)
    root = left_child;
  else if (ptr == ptr->parent->left)
    ptr->parent->left = left_child;
  else
    ptr->parent->right = left_child;

  left_child->right = ptr;
  ptr->parent = left_child;
}

void RBTree::fixInsertRBTree(Node *&ptr) {
  Node *parent = NULL;
  Node *grandparent = NULL;
  while (ptr != root && getColor(ptr) == RED && getColor(ptr->parent) == RED) {
    parent = ptr->parent;
    grandparent = parent->parent;
    if (parent == grandparent->left) {
      Node *uncle = grandparent->right;
      if (getColor(uncle) == RED) {
        setColor(uncle, BLACK);
        setColor(parent, BLACK);
        setColor(grandparent, RED);
        ptr = grandparent;
      }
      else {
        if (ptr == parent->right) {
          rotateLeft(parent);
          ptr = parent;
          parent = ptr->parent;
        }
        rotateRight(grandparent);
        swap(parent->color, grandparent->color);
        ptr = parent;
      }
    }
    else {
      Node *uncle = grandparent->left;
      if (getColor(uncle) == RED) {
        setColor(uncle, BLACK);
        setColor(parent, BLACK);
        setColor(grandparent, RED);
        ptr = grandparent;
      }
      else {
        if (ptr == parent->left) {
          rotateRight(parent);
          ptr = parent;
          parent = ptr->parent;
        }
        rotateLeft(grandparent);
        swap(parent->color, grandparent->color);
        ptr = parent;
      }
    }
  }
  setColor(root, BLACK);
}

void RBTree::fixDeleteRBTree(Node *&node) {
  if (node == NULL)
    return;

  if (node == root) {
    root = NULL;
    return;
  }

  if (getColor(node) == RED || getColor(node->left) == RED || getColor(node->right) == RED) {
    Node *child = node->left != NULL ? node->left : node->right;

    if (node == node->parent->left) {
      node->parent->left = child;
      if (child != NULL)
        child->parent = node->parent;
      setColor(child, BLACK);
      delete (node);
    }
    else {
      node->parent->right = child;
      if (child != NULL)
        child->parent = node->parent;
      setColor(child, BLACK);
      delete (node);
    }
  }
  else {
    Node *sibling = NULL;
    Node *parent = NULL;
    Node *ptr = node;
    setColor(ptr, DOUBLE_BLACK);
    while (ptr != root && getColor(ptr) == DOUBLE_BLACK) {
      parent = ptr->parent;
      if (ptr == parent->left) {
        sibling = parent->right;
        if (getColor(sibling) == RED) {
          setColor(sibling, BLACK);
          setColor(parent, RED);
          rotateLeft(parent);
        }
        else {
          if (getColor(sibling->left) == BLACK && getColor(sibling->right) == BLACK) {
            setColor(sibling, RED);
            if (getColor(parent) == RED)
              setColor(parent, BLACK);
            else
              setColor(parent, DOUBLE_BLACK);
            ptr = parent;
          }
          else {
            if (getColor(sibling->right) == BLACK) {
              setColor(sibling->left, BLACK);
              setColor(sibling, RED);
              rotateRight(sibling);
              sibling = parent->right;
            }
            setColor(sibling, parent->color);
            setColor(parent, BLACK);
            setColor(sibling->right, BLACK);
            rotateLeft(parent);
            break;
          }
        }
      }
      else {
        sibling = parent->left;
        if (getColor(sibling) == RED) {
          setColor(sibling, BLACK);
          setColor(parent, RED);
          rotateRight(parent);
        }
        else {
          if (getColor(sibling->left) == BLACK && getColor(sibling->right) == BLACK) {
            setColor(sibling, RED);
            if (getColor(parent) == RED)
              setColor(parent, BLACK);
            else
              setColor(parent, DOUBLE_BLACK);
            ptr = parent;
          }
          else {
            if (getColor(sibling->left) == BLACK) {
              setColor(sibling->right, BLACK);
              setColor(sibling, RED);
              rotateLeft(sibling);
              sibling = parent->left;
            }
            setColor(sibling, parent->color);
            setColor(parent, BLACK);
            setColor(sibling->left, BLACK);
            rotateRight(parent);
            break;
          }
        }
      }
    }
    if (node == node->parent->left)
      node->parent->left = NULL;
    else
      node->parent->right = NULL;
    delete(node);
    setColor(root, BLACK);
  }
}

Node* RBTree::deleteBST(Node *&root, Thread* data) {
  if (root == NULL)
    return root;

  if (data->getVirtualRunTime() < root->data->getVirtualRunTime())
    return deleteBST(root->left, data);

  if (data->getVirtualRunTime() > root->data->getVirtualRunTime())
    return deleteBST(root->right, data);

  if (root->left == NULL || root->right == NULL)
    return root;

  Node *temp = minValueNode(root->right);
  root->data = temp->data;
  return deleteBST(root->right, temp->data);
}

void RBTree::deleteValue(Thread* data) {
  Node *node = deleteBST(root, data);
  fixDeleteRBTree(node);
}

void RBTree::inorderBST(Node *&ptr) {
  if (ptr == NULL)
    return;

  inorderBST(ptr->left);
  cout << "[" << ptr->data->getName() << " at " << ptr->data->getVirtualRunTime() << "(" << NodeNames[ptr->color] << ")]";
  inorderBST(ptr->right);
}

void RBTree::inorder() {
  inorderBST(root);
}

void RBTree::preorderBST(Node *&ptr) {
  if (ptr == NULL)
    return;

  cout << ptr->data << " " << ptr->color << endl;
  preorderBST(ptr->left);
  preorderBST(ptr->right);
}

void RBTree::preorder() {
  preorderBST(root);
  cout << "-------" << endl;
}

Node *RBTree::minValueNode(Node *&node) {

  Node *ptr = node;

  while (ptr->left != NULL)
    ptr = ptr->left;

  return ptr;
}

Node * RBTree::minValueNode(){
  minValueNode(root);
}

Node* RBTree::maxValueNode(Node *&node) {
  Node *ptr = node;

  while (ptr->right != NULL)
    ptr = ptr->right;

  return ptr;
}

int RBTree::getBlackHeight(Node *node) {
  int blackheight = 0;
  while (node != NULL) {
    if (getColor(node) == BLACK)
      blackheight++;
    node = node->left;
  }
  return blackheight;
}

void RBTree::merge(RBTree rbTree2) {
  Thread* temp;
  Node *c, *temp_ptr;
  Node *root1 = root;
  Node *root2 = rbTree2.root;
  int initialblackheight1 = getBlackHeight(root1);
  int initialblackheight2 = getBlackHeight(root2);
  if (initialblackheight1 > initialblackheight2) {
    c = maxValueNode(root1);
    temp = c->data;
    deleteValue(c->data);
    root1 = root;
  }
  else if (initialblackheight2 > initialblackheight1) {
    c = minValueNode(root2);
    temp = c->data;
    rbTree2.deleteValue(c->data);
    root2 = rbTree2.root;
  }
  else {
    c = minValueNode(root2);
    temp = c->data;
    rbTree2.deleteValue(c->data);
    root2 = rbTree2.root;
    if (initialblackheight1 != getBlackHeight(root2)) {
      rbTree2.insertValue(c->data);
      root2 = rbTree2.root;
      c = maxValueNode(root1);
      temp = c->data;
      deleteValue(c->data);
      root1 = root;
    }
  }
  setColor(c, RED);
  int finalblackheight1 = getBlackHeight(root1);
  int finalblackheight2 = getBlackHeight(root2);
  if (finalblackheight1 == finalblackheight2) {
    c->left = root1;
    root1->parent = c;
    c->right = root2;
    root2->parent = c;
    setColor(c, BLACK);
    c->data = temp;
    root = c;
  }
  else if (finalblackheight2 > finalblackheight1) {
    Node *ptr = root2;
    while (finalblackheight1 != getBlackHeight(ptr)) {
      temp_ptr = ptr;
      ptr = ptr->left;
    }
    Node *ptr_parent;
    if (ptr == NULL)
      ptr_parent = temp_ptr;
    else
      ptr_parent = ptr->parent;
    c->left = root1;
    if (root1 != NULL)
      root1->parent = c;
    c->right = ptr;
    if (ptr != NULL)
      ptr->parent = c;
    ptr_parent->left = c;
    c->parent = ptr_parent;
    if (getColor(ptr_parent) == RED) {
      fixInsertRBTree(c);
    }
    else if (getColor(ptr) == RED) {
      fixInsertRBTree(ptr);
    }
    c->data = temp;
    root = root2;
  }
  else {
    Node *ptr = root1;
    while (finalblackheight2 != getBlackHeight(ptr)) {
      ptr = ptr->right;
    }
    Node *ptr_parent = ptr->parent;
    c->right = root2;
    root2->parent = c;
    c->left = ptr;
    ptr->parent = c;
    ptr_parent->right = c;
    c->parent = ptr_parent;
    if (getColor(ptr_parent) == RED) {
      fixInsertRBTree(c);
    }
    else if (getColor(ptr) == RED) {
      fixInsertRBTree(ptr);
    }
    c->data = temp;
    root = root1;
  }
  return;
}

bool RBTree::isEmpty() {
  if (root == NULL) {
    return TRUE;
  }
  return FALSE;
}

*/

void RBTree::printTree() { // list
  if (root == NULL) {
    return;
  }
  List<Node*> *rbqueue = new List<Node*>();
  rbqueue->Append(root);
  while (rbqueue->NumInList() != 0) {
    int size = rbqueue->NumInList();
    for (int i = 0; i < size; i++) {
      Node *n = rbqueue->Front(); // remove front
      if (n != NULL) {
        if (n->left != NULL) {
          rbqueue->Append(n->left);
        }
        if (n->right != NULL) {
          rbqueue->Append(n->right);
        }
        printf("[%s at %.2f(%s)] ", n->data->getName(), n->data->getVirtualRunTime(), NodeNames[n->color]);
      }
      rbqueue->RemoveFront();
    }

    printf("\n");
  }

}

Node*
RBTree::getANode() {
  if (minValueNode() != root) {
    return minValueNode();
  }
  else {
    return maxValueNode(root);
  }
}