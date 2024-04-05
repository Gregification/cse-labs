// Unbalanced binary search tree header file for lab 4.

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

// These will have to change if data in node is more than just an int.
typedef int Key;
typedef Key Item;
#define key(A)      (A)
#define less(A, B)  (key(A) < key(B))
#define eq(A, B)    (key(A) == key(B))

typedef struct STnode* link;

struct STnode {
    Item item;  // Data for this node
    link l, r;  // left & right links
    int N;      // subtree size (counts only live nodes)
};

extern Item NULLitem;

//spaghetti
extern char **ptstr;

void STinit();          // Initialize tree with just a sentinel

void STterminate();     // frees all resources, STinit() must be called to use tree again.

Item STsearch(Key v);   // Find node for a key

Item STselect(int k);   // Treat tree as flattened into an ordered array

int STinvSelect(Key v); // Inverse of STselect

void STinsert(Item item);   // Insert an item.  No uniqueness check
link createSTnode();

//return true if tree is invalid
int STverifyProperties();   // Ensure that tree isn't damaged
int STverifyPropertiesHelper(link node, Key min, Key max);
int STcalcSTsize(link root);

void STprintTree();         // Dumps out tree
void STprintSubTree(link, int);

char* STserialize();        // Flattens current tree into a pre-order string
void STserializeHelper(link, char*);

void STdeserialize(char *str);  // Parses string to current tree
void STdeserializeHelper(char **, link);//pointer to string
Item getValFromStr(char **);

int getLive();  // Number of nodes in tree with active keys.