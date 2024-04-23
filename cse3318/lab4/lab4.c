/**
 * cse3318-003 lab4
 * george boone
 * 1002055713
 * 
 * compile on OMEGA, runs using the file redirect thing "<"
 * executable is "lab4.out"
    $ gcc -c lab4.c -o l4.o; gcc -c lab4driver.c -o l4d.o; gcc -o lab4.out l4.o l4d.o;
    $ ./lab4.out < a.dat

    table print format
    parent nodes on left, child nodes on right

    [item, subtree-size]    
                |_[left node]
                |       |_[left of left node]
                |       |_[end]             <--- sentinal node
                |_[right node]
                |       |_[end]
                |       |_[right of right node]
*/

#include "lab4.h"

#define DEBUG_PARSER 0
#define isSentinal(node_ptr) (node_ptr->N < 0)

int NULLitem = INT_MIN;
link sentinel;

//spaghetti code
char **ptstr;

void STinit(){          // Initialize tree with just a sentinel
    sentinel = createSTnode();
    sentinel->N = -1;
}

link createSTnode(){
    link node = malloc(sizeof(struct STnode));
    struct STnode tmp = {
            .r = sentinel,
            .l = sentinel,
            .item = NULLitem
        };
    memcpy(node, &tmp, sizeof(struct STnode));
    return node;
}

void STterminate(){
    //lazy
    //find a leaf -> delete that leaf -> repeat

    while(!isSentinal(sentinel->l)){
        link 
            former = sentinel,
            node = sentinel->l;

        while(!isSentinal(node)){
            int 
                leftValid = !isSentinal(node->l),
                rightValid = !isSentinal(node->r);

            if(!leftValid && !rightValid){
                if(node == former->l)
                    former->l = sentinel;
                else former->r = sentinel;

                free(node);
                break;
            }

            former = node;
            if(leftValid)
                node = node->l;
            else
                node = node->r;
        }
    }

    free(sentinel);
    free((*ptstr));
    exit(0);
}

Item STsearch(Key v){   // Find node for a key
    link ret = sentinel->l;

    while(!isSentinal(ret)){
        if(eq(ret->item, v)) 
            return ret->item;

        int isLess = less(ret->item, v);

        if(!isSentinal(ret->l) && isLess)
            ret = ret->l;
        else if(!isSentinal(ret->r) && !isLess)
            ret = ret->r;
        else break;
    }

    return NULLitem;
}

Item STselect(int k){   // Treat tree as flattened into an ordered array
    link node = sentinel->l;

    while(!isSentinal(node)){
        int apparentRank = (isSentinal(node->l) ? 0 : node->l->N) + 1; //[left ST count] + 1
        if(apparentRank == k) return node->item;
        
        if(k < apparentRank){
            node = node->l;
        }else {
            node = node->r;
            k -= apparentRank;
        }
    }

    return NULLitem;
}

int STinvSelect(Key v){ // Inverse of STselect
    //??? whar
}

void STinsert(Item item){     // Insert an item.  No uniqueness check
    link 
        node    = sentinel->l,
        former  = sentinel,
        new     = createSTnode();

    int isLess;

    new->N      = 1;
    new->item   = item;

    //find the leaf that it should get put to
    while(!isSentinal(node)){
        former = node;
        node->N++;

        isLess = less(item, node->item);
        if(isLess)  node = node->l;
        else        node = node->r;
    }

    if(isLess)
        former->l = new;
    else
        former->r = new;
}

//return true if tree is invalid
int STverifyProperties(){    // Ensure that tree isn't damaged
    return STverifyPropertiesHelper(sentinel->l, INT_MIN, INT_MAX);
}
int STverifyPropertiesHelper(link node, Key min, Key max){
    if(isSentinal(node))
        return 0;

    if(node->item < min || node->item > max)
        return 1;

    int size = 1;
    if(!isSentinal(node->l)) size += node->l->N;
    if(!isSentinal(node->r)) size += node->r->N;

    if(size != node->N) return 1;

    return 
        STverifyPropertiesHelper(node->l, min, node->item) ||
        STverifyPropertiesHelper(node->r, node->item, max);
}
int STcalcSTsize(link root){
    if(isSentinal(root)) return 0;

    int size = 1;
    size += STcalcSTsize(root->l);
    size += STcalcSTsize(root->r);
    root->N = size;

    return size;
}

void STprintTree(){           // Dumps out tree
    STprintSubTree(sentinel->l, 0);
}

void STprintSubTree(link root, int indent){
    for(int i = 0; i < indent; i++)
        printf("%12s", "|");
    
    if(isSentinal(root)){
        puts("_[end]");
        return;
    }

    printf("_[%3d, %2d]\n", root->item, root->N);
    STprintSubTree(root->r, indent+1);
    STprintSubTree(root->l, indent+1);
}

char* STserialize(){    // Flattens current tree into a pre-order string
    if(isSentinal(sentinel->l))
        return "";
    
    char* ret = calloc(sentinel->l->N * 6, sizeof(char));//eyeballed the number, seems to work

    STserializeHelper(sentinel->l, ret);

    return ret;
}
void STserializeHelper(link node, char* str){
    if(isSentinal(node)){
        strcat(str, ".");
        return;
    }
    char arr[20];
    int l = sprintf(arr, "%d",node->item);
    strcat(str, node->item > 0 ? "+" : "");
    strcat(str, arr);
    STserializeHelper(node->l, str);
    STserializeHelper(node->r, str);
}

void STdeserialize(char *str){     // Parses string to current tree
    ptstr = &str;
    link root = createSTnode();
        root->item = getValFromStr(&str);
        root->N = 1;

    sentinel->l = root;
    STdeserializeHelper(&str, root);
    
    STcalcSTsize(root); //correct the subtree counts

    //testing
    if(strlen(str) != 0) 
        puts("\x1B[33m[WARNING] extra characters in input! \x1B[39m ");
    if(STverifyProperties())
        puts("\x1B[33m[WARNING] invalid tree (either node order or subtree size)\x1B[39m ");
}
void STdeserializeHelper(char **pts, link root){
    if(!root) return;

    //for each possible child
    for(int i = 0; i < 2; i++){
        switch((*pts)[0]){
            case '.'    : 
                (*pts) += 1;
                if(DEBUG_PARSER) puts("next");
                continue;
            case '+'    :
            case '-'    :
            case '\0'   :
                break;
            default: {
                printf("bad input \"%c\" in %s\n", (*pts)[0], *pts);
                STterminate();
            }
        }

        link new = createSTnode();
            new->N = 1;
            new->item = getValFromStr(pts);

        link* too;
        switch(i){
            case 0: too = &(root->l);  break;
            case 1: too = &(root->r);  break;

            default:{//uh oh
                if(DEBUG_PARSER) puts("deseralization explosion");
                STterminate();
                exit(1);//may mem leak the string
            }
        }

        *too = new;
        STdeserializeHelper(pts, *too);
    }
}
Item getValFromStr(char **pts){ //gets first value & incriments
    int off = 1;
    char* str = *pts;

    while(str[off] <= '9' && str[off] >= '0') off++;

    switch(str[off]){
        case '+'    :
        case '-'    :
        case '.'    :
        case '\0'   :
            break;
        default: {
            printf("bad input \"%c\" in %s\n", str[off], str);
            exit(1);
        }
    }

    char og = str[off];
    str[off] = '\0';
    int ret = atoi(str);
    str[off] = og;

    if(DEBUG_PARSER) printf("%d\n\t%s\n\t%s\n", ret, str, str+off);

    *pts += off;

    return ret;
}


int getLive(){  // Number of nodes in tree with active keys.
    if(isSentinal(sentinel->l))
        return 0;
    
    return sentinel->l->N;
}
