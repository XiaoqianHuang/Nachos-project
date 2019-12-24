

#ifndef LRUCACHE
#define LRUCACHE
#include "../lib/list.h"
#include "../machine/translate.h"
#include <map>

struct Node {
  Node *next;
  Node *prev;
  TranslationEntry* entry;
  int LRUTime;
  Node(TranslationEntry *entry, int LRUTime) {
    this->entry = entry;
    this->LRUTime = LRUTime;
    next = prev = NULL;
  }

  void update(TranslationEntry *entry, int LRUTime) {
    this->entry = entry;
    this->LRUTime = LRUTime;
  }
};

class LRUCache {
public:
  LRUCache(int cap);
  TranslationEntry* set(int LRUtime, TranslationEntry* entry);
  Node* oldestNode();
  Node* removeNode(Node *node);
  Node* appendNode(Node *node);

  Node *head;
  Node *tail;
  map<TranslationEntry*, Node*> *nodeMap;
  int capibility;
  int size;
};

#endif // !LRUCACHE


