// rb_tree_nachos.h
// data structure for storing thread entities in ready queue

// ** The commented code is come from https://github.com/anandarao/Red-Black-Tree
/*
#pragma once
#ifndef RB_TREE_NACHOS_H
#define RB_TREE_NACHOS_H

#include "debug.h"
#include "list.h"
#include "thread.h"

static const char * NodeNames[] = { "R", "B", "DB" };

enum Color { RED, BLACK, DOUBLE_BLACK };

struct Node
{
  Thread* data;
  int color;
  Node *left, *right, *parent;

  explicit Node(Thread*);
};

class RBTree
{
private:
  Node *root;
public:
  void rotateLeft(Node *&); // left rotate the subtree
  void rotateRight(Node *&); // right rotate the subtree
  void fixInsertRBTree(Node *&); // insert adjust
  void fixDeleteRBTree(Node *&); // delete adjust
  void inorderBST(Node *&); // print the tree by in order sequence
  void preorderBST(Node *&);  // print the tree by pre order sequence
  int getColor(Node *&); // get the color
  void setColor(Node *&, int); // set the color
  Node *minValueNode(Node *&); //Find the node with minimum value
  Node *minValueNode(); // public
  Node *maxValueNode(Node *&); //Find the node with maximum value
  Node *getANode(); // get a node from the tree
  Node* insertBST(Node *&, Node *&); // insert node
  Node* deleteBST(Node *&, Thread*);  // delete node
  int getBlackHeight(Node *); // get black height of the tree
  bool isEmpty(); // if the tree is empty
  void printTree();

public:
  RBTree();
  void insertValue(Thread*);
  void deleteValue(Thread*);
  void merge(RBTree);
  void inorder();
  void preorder();
};


#endif //RED_BLACK_TREE_RBTREE_H
*/