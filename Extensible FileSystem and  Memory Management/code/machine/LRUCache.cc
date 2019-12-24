#include "LRUCache.h"

LRUCache::LRUCache(int cap) {
  capibility = cap;
  head = tail = NULL;
  nodeMap = new map< TranslationEntry*, Node*>();
} // constructor

TranslationEntry* 
LRUCache::set(int LRUtime, TranslationEntry* entry) {
  TranslationEntry* removedValue = NULL;
  Node *node = NULL;
  //Node *currentNode = getNode(value);
  if (nodeMap->find(entry) != nodeMap->end()) { // if the key is already in the map, update value and put the node to the end
    node = (*nodeMap)[entry];
    node->LRUTime = LRUtime;
    removeNode(node);
    //tranList->Remove(currentNode);
    //currentNode->LRUTime = LRUtime;
  }
  else if (size < capibility) {
    node = new Node(entry, LRUtime);
  }
  else { // oversize
    node = tail; //select the least recent used node. 
    removedValue = tail->entry;
    removeNode(node);
    node->update(entry, LRUtime);
  }

  appendNode(node);// put new entry into the end of the list

  return removedValue; // return the deleted value
}

Node*
LRUCache::oldestNode() {
  return tail;
}

Node* 
LRUCache::removeNode(Node *node) {
  nodeMap->erase(node->entry);
  size--;
  if (node->prev != NULL) {
    node->prev->next = node->next;
  }
  if (node->next != NULL) {
    node->next->prev = node->prev;
  }
  if (node == head) {
    head = head->next;
  }
  if (node == tail) {
    tail = tail->prev;
  }
  node->next = NULL;
  node->prev = NULL;

  return node;
}

Node* 
LRUCache::appendNode(Node *node) {
  (*nodeMap)[node->entry] = node;
  size++;
  if (head == NULL) {
    head = node;
    tail = node;
  }
  else {
    node->next = head;
    head->prev = node;
    head = node;
  }
  return node;
}