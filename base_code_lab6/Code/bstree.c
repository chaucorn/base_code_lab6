#include "bstree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

/* ---------------------- RBTSpecific ----------------------*/
void bstree_node_to_dot ( const BinarySearchTree * t , void * stream );
void testrotateleft ( BinarySearchTree * t) ;
void testrotateright ( BinarySearchTree *t );
BinarySearchTree* grandparent(BinarySearchTree* n);
BinarySearchTree* uncle(BinarySearchTree* n);
BinarySearchTree* fixredblack_insert_case1(BinarySearchTree* x);
BinarySearchTree* fixredblack_insert_case2(BinarySearchTree* x);
BinarySearchTree* fixredblack_insert_case2_left(BinarySearchTree* x);
BinarySearchTree* fixredblack_insert_case2_right(BinarySearchTree* x);

/*------------------------  BSTreeType  -----------------------------*/
typedef enum {red, black} NodeColor;
struct _bstree {
    BinarySearchTree* parent;
    BinarySearchTree* left;
    BinarySearchTree* right;
    int key;
    NodeColor color;
};
typedef const BinarySearchTree* (*AccessFunction) (const BinarySearchTree*);
typedef struct {
    AccessFunction left;
    AccessFunction right;
} ChildAccessor;

/*------------------------  BaseBSTree  -----------------------------*/

BinarySearchTree* bstree_create(void) {
    return NULL;
}

/* This constructor is private so that we can maintain the oredring invariant on
 * nodes. The only way to add nodes to the tree is with the bstree_add function
 * that ensures the invariant.
 */
BinarySearchTree* bstree_cons(BinarySearchTree* left, BinarySearchTree* right, int key) {
    BinarySearchTree* t = malloc(sizeof(struct _bstree));
    t->parent = NULL;
    t->left = left;
    t->right = right;
    t->key = key;
    t->color = red;

    if (t->left != NULL)
        t->left->parent = t;
    if (t->right != NULL)
        t->right->parent = t;
    return t;
}

void freenode(const BinarySearchTree* t, void* n) {
    (void)n;
    free((BinarySearchTree*)t);
}

void bstree_delete(ptrBinarySearchTree* t) {
    bstree_depth_postfix(*t, freenode, NULL);
    *t=NULL;
}

bool bstree_empty(const BinarySearchTree* t) {
    return t == NULL;
}

int bstree_key(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->key;
}

BinarySearchTree* bstree_left(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->left;
}

BinarySearchTree* bstree_right(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->right;
}

BinarySearchTree* bstree_parent(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->parent;
}

/*------------------------  BSTreeDictionary  -----------------------------*/

/* Obligation de passer l'arbre par référence pour pouvoir le modifier */
void bstree_add(ptrBinarySearchTree* t, int v) {
	if (*t == NULL){
        *t = bstree_cons(NULL, NULL, v);
    }else{
        BinarySearchTree* bn_tree = *t;
        if (v > bn_tree->key){
            bstree_add(&bn_tree->right, v);
            bn_tree->right->parent = *t;
        }else{
            bstree_add(&bn_tree->left, v);
            bn_tree->left->parent = *t;
        } 
    }
}

const BinarySearchTree* bstree_search(const BinarySearchTree* t, int v) {
    if (t != NULL){
        int key = t->key;
        if (v < key){
            return bstree_search(t->left, v);
        }
        else if (v > key){
            return bstree_search(t->right, v);
        }
        else{
            return t;
        }
    }
    return NULL;
}

const BinarySearchTree* bigger_node(const BinarySearchTree* node1, const BinarySearchTree* node2){
    if (node1&&node2){
        int key1 = node1->key;
        int key2 = node2->key;
        return (key1>key2)?node1:node2;
    }else{
        return (node1)?node1:node2;
    }    
}

const BinarySearchTree* smaller_node(const BinarySearchTree* node1, const BinarySearchTree* node2){
    if (node1&&node2){
        int key1 = node1->key;
        int key2 = node2->key;
        return (key1<key2)?node1:node2;
    }else{
        return (node1)?node1:node2;
    }    
}

const BinarySearchTree* accessLeft(const BinarySearchTree* x){
    const BinarySearchTree* predecessor_child =  NULL;
    if (x->left != NULL){
        predecessor_child = x->left;
        while (predecessor_child->right != NULL){
            predecessor_child = predecessor_child->right;
        }
        
    }
    return predecessor_child;
    
}


const BinarySearchTree* accessRight(const BinarySearchTree* x){
    const BinarySearchTree* successor_child =  NULL;
    if (x->left != NULL){
        successor_child = x->right;
        while (successor_child->left != NULL){
            successor_child = successor_child->left;
        }
        
    }
    return successor_child;
    
}


const BinarySearchTree* find_next(const BinarySearchTree* x, AccessFunction access){
    const BinarySearchTree* next_node=  access(x);
    return next_node;
    
}

const BinarySearchTree* bstree_successor(const BinarySearchTree* x) {
    assert(!bstree_empty(x));
    // Find upper successor
    const BinarySearchTree* successor_upper = NULL;
    const BinarySearchTree* successor_lower = NULL;
    if (x->parent!=NULL){
        // if node is the left child of its parent
        const BinarySearchTree* parent = x->parent;
        if (parent->left!=NULL&&parent->left ==x){
            successor_upper = x->parent;
        }else if(parent->parent!=NULL&&(parent->parent)->right == parent){
            successor_upper = parent->parent;
            }
        }
    // Find lower successor on the right
    ChildAccessor accessor = {.left = accessLeft, .right=accessRight};
    successor_lower = find_next(x, accessor.right);
    return smaller_node(successor_lower, successor_upper);
}


const BinarySearchTree* bstree_predecessor(const BinarySearchTree* x) {
    assert(!bstree_empty(x));
    // Find upper predecessor
    const BinarySearchTree* predecessor_upper = NULL;
    const BinarySearchTree* predecessor_lower = NULL;
    if (x->parent!=NULL){
        // if node is right child
        const BinarySearchTree* parent = x->parent;
        if (parent->left!=NULL&&parent->left ==x){
            if(parent->parent!=NULL&&(parent->parent)->right == parent){
                predecessor_upper = parent->parent;
            }
        }
        if (parent->right!=NULL&&parent->right == x){
            predecessor_upper = parent;
        }
        
    }
    // Find lower predecessor on the left
    ChildAccessor accessor = {.left = accessLeft, .right = accessRight};
    predecessor_lower = find_next(x, accessor.left);
    return bigger_node(predecessor_lower, predecessor_upper);
}

//void bstree_swap_nodes(ptrBinarySearchTree* tree, ptrBinarySearchTree from, ptrBinarySearchTree to)
void bstree_swap_nodes(ptrBinarySearchTree from, ptrBinarySearchTree to) {
    assert(!bstree_empty(*tree) && !bstree_empty(from) && !bstree_empty(to));
    int value_from = from->key;
    int value_to = to->key;
    from->key = value_to;
    to->key = value_from;
}

// t -> the tree to remove from, current -> the node to remove
//void bstree_remove_node(ptrBinarySearchTree* t, ptrBinarySearchTree current) 
void bstree_remove_node(ptrBinarySearchTree current) {
    // Cas 1 node has no child
    if (current->left == NULL&& current->right == NULL)
    {
        if (current->parent != NULL)
        {
            BinarySearchTree* parent = current->parent;
            parent->left = (parent->left == current)?  NULL : parent->left ;
            parent->right = (parent->right == current)? NULL :parent->right ;
        }
        free(current);
    }
    // Case 2: Node has two children
    else if (current->left != NULL&& current->right != NULL) {
        // Cast const type to node
        const BinarySearchTree* current_const = (const BinarySearchTree*) current;
        // Find successor
        BinarySearchTree* successor = (BinarySearchTree*) bstree_successor(current_const); 
        bstree_swap_nodes(current, successor);
        bstree_remove_node(successor);
        
    }
    // Case 3: only left child
    else if (current->left != NULL && current->right == NULL){
        if (current->parent != NULL)
        {
            BinarySearchTree* parent = current->parent;
            parent->left = (parent->left == current)?  current->left : parent->left ;
            parent->right = (parent->right == current)? current->left :parent->right ;
            current->left->parent = parent;
        }else{
            current->left->parent = NULL;
        }
        free(current);
        
    }
    // Case 4 only right child
    else if (current->right != NULL && current->left == NULL) {
        if (current->parent != NULL)
        {
            BinarySearchTree* parent = current->parent;
            parent->left = (parent->left == current)?  current->right : parent->left ;
            parent->right = (parent->right == current)? current->right :parent->right ;
            current->right->parent = parent;
        }else{
            current->right->parent = NULL;
        }
        free(current);
    }
}

void bstree_remove(ptrBinarySearchTree* t, int v) {
    assert (!bstree_empty(*t));
    const ptrBinarySearchTree tree = *t;
    const ptrBinarySearchTree node = (const ptrBinarySearchTree) bstree_search(tree, v) ;
    if (node != NULL){
        ptrBinarySearchTree to_remove = (ptrBinarySearchTree) node; 
        bstree_remove_node(to_remove);
    }

}

/*------------------------  BSTreeVisitors  -----------------------------*/

void bstree_depth_prefix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if (t!=NULL)
    {
        //printf("%i ", t->key);
        f(t, environment);
        bstree_depth_prefix(t->left, f, environment);
        bstree_depth_prefix(t->right, f, environment);
    }
    
}

void bstree_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if (t!=NULL)
    {
        
        bstree_depth_infix(t->left, f, environment);
        f(t, environment);
        //printf("%i ", t->key);
        bstree_depth_infix(t->right, f, environment);
        
    }
}

void bstree_depth_postfix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if (t != NULL)
    {
        bstree_depth_postfix(t->left, f, environment);
        bstree_depth_postfix(t->right, f, environment);
        f(t, environment);
        //printf("%i ", t->key);
    }
    
}

void bstree_iterative_breadth(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    Queue* q = create_queue();
    if (t != NULL){
        queue_push(q, t);
    }
    while (!queue_empty(q)){
        const BinarySearchTree* node = queue_top(q);
        f(node, environment);
        queue_pop(q);
        if (node->left != NULL){
            queue_push(q, node->left);
        }
        if (node->right != NULL){
            queue_push(q, node->right);
        }
    }
    delete_queue(&q);
}

void bstree_iterative_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    bstree_depth_infix(t, f, environment);
}

/*------------------------  BSTreeIterator  -----------------------------*/

struct _BSTreeIterator {
    /* the collection the iterator is attached to */
    const BinarySearchTree* collection;
    /* the first element according to the iterator direction */
    const BinarySearchTree* (*begin)(const BinarySearchTree* );
    /* the current element pointed by the iterator */
    const BinarySearchTree* current;
    /* function that goes to the next element according to the iterator direction */
    const BinarySearchTree* (*next)(const BinarySearchTree* );
};

/* minimum element of the collection */
const BinarySearchTree* goto_min(const BinarySearchTree* e) {
	assert (e!=NULL);
    if (e->left == NULL){
        return e;
    }else{
        while (e->left!=NULL){
            e = e->left;
        }
        return e;
    }
}

/* maximum element of the collection */
const BinarySearchTree* goto_max(const BinarySearchTree* e) {
	assert (e!=NULL);
    if (e->right == NULL){
        return e;
    }else{
        while (e->right!=NULL){
            e = e->right;
        }
        return e;
    }
}

/* constructor */
BSTreeIterator* bstree_iterator_create(const BinarySearchTree* collection, IteratorDirection direction) {
	BSTreeIterator* iterator = malloc(sizeof(BSTreeIterator));
    iterator->collection = collection;
    if (direction == backward)
    {
        iterator->begin = goto_max;
        iterator->next = bstree_predecessor;
    }else{
        iterator->begin = goto_min;
        iterator->next = bstree_successor;
    }
    iterator->current = iterator->begin(collection);
    return iterator;
}


/* destructor */
void bstree_iterator_delete(ptrBSTreeIterator* i) {
    free(*i);
    *i = NULL;
}

BSTreeIterator* bstree_iterator_begin(BSTreeIterator* i) {
    i->current = i->begin(i->collection);
    return i;
}

bool bstree_iterator_end(const BSTreeIterator* i) {
    return i->current == NULL;
}

BSTreeIterator* bstree_iterator_next(BSTreeIterator* i) {
    i->current = i->next(i->current);
    return i;
}

const BinarySearchTree* bstree_iterator_value(const BSTreeIterator* i) {
    return i->current;
}

/* ---------------------- RBTSpecific ----------------------*/
void bstree_node_to_dot ( const BinarySearchTree * t , void * stream ){
    FILE *file = (FILE *) stream;

    printf("%d ", bstree_key(t));
    const char *color = (t->color == red) ? "fillcolor=red" : "fillcolor=grey";
    
    /*
    fprintf(file, "\tn%d [label=\"{%d|{<left>|<right>}}\"];\n", 
        t->key, t->key);
    */
    fprintf(file, "\tn%d [style=filled, %s, label=\"{%d|{<left>|<right>}}\"];\n", bstree_key(t), color, bstree_key(t));


    if (bstree_left(t)) {
        fprintf(file, "\tn%d:left:c -> n%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(bstree_left(t)));
    } else {
        fprintf(file, "\tlnil%d [style=filled, fillcolor=grey, label=\"NIL\"];\n", bstree_key(t));
        fprintf(file, "\tn%d:left:c -> lnil%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(t));
    }
    if (bstree_right(t)) {
        fprintf(file, "\tn%d:right:c -> n%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(bstree_right(t)));
    } else {
        fprintf(file, "\trnil%d [style=filled, fillcolor=grey, label=\"NIL\"];\n", bstree_key(t));
        fprintf(file, "\tn%d:right:c -> rnil%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(t));
    }
}

void leftrotate(BinarySearchTree*x){
    assert(x!=NULL);
    if (x->right != NULL){
        // y is the right child of x
        BinarySearchTree* y = x->right;
        if (y->left != NULL)
        {
            x->right = y->left;
            y->left->parent = x;
        }
        y->left = x;
        // if x has parent
        if (x->parent!=NULL)
        {
            BinarySearchTree* parent_x = x->parent;
            if (parent_x->left != NULL && parent_x->left == x)
            {
                parent_x->left = y;
                y->parent = parent_x;
            }else if (parent_x->right != NULL && parent_x->right == x)
            {
                parent_x->right = y;
                y->parent = parent_x;
            }
            
        }else{
            x->parent = y;
            y->parent = NULL;
        } 
    }
    
}

/*-- Right Rotate : Dans la rotation droite, un nœud devient le fils droit du nœud qui ´etait son fils gauche. --*/
void rightrotate(BinarySearchTree*x){
    assert(x!= NULL);
    // If left child exists
    if (x->left != NULL)
    {
        // y is the left child of x
        BinarySearchTree* y = x->left;
        if (y->right != NULL){
            x->left = y->right;
            y->right = x;
        }
        if (x->parent != NULL){
            BinarySearchTree* parent_x = x->parent;
            if (parent_x->left != NULL&&parent_x->left == x){
                parent_x->left = y; 
            }else if (parent_x->right != NULL&&parent_x->right == x)
            {
                parent_x->right = y;
            }
            y->parent = parent_x;
        }else{
            x->parent = y;
            y->parent = NULL;
        }
    }
}

void testrotateleft(BinarySearchTree *t){
    leftrotate(t);
}
void testrotateright(BinarySearchTree *t){
    rightrotate(t);
}

BinarySearchTree* grandparent(BinarySearchTree* n){
    return n->parent->parent;
}

BinarySearchTree* uncle(BinarySearchTree* n){
    BinarySearchTree* parent = n->parent;
    BinarySearchTree* grandparent = n->parent->parent;
    if (grandparent->left!= NULL&&grandparent->left == parent)
    {
        return grandparent->right;
    }else if (grandparent->right!= NULL&&grandparent->right == parent)
    {
        return grandparent->left;
    }
    return NULL;

}

BinarySearchTree* fixredblack_insert(BinarySearchTree* x){
    // if the parent of x is the root of the tree ->color in black
    if (x->parent->parent == NULL)
    {
        x->parent->color = black;
        return x;
    }else{
        if(x->parent->color ==red){
            return fixredblack_insert_case1(x);
        }else{
            return x;
        }
    }
}

BinarySearchTree* fixredblack_insert_case1(BinarySearchTree* x){ // 
    // If x's uncle is red
    BinarySearchTree* x_uncle = uncle(x);
    if (x_uncle != NULL)
    {
        BinarySearchTree* x_grandparent = grandparent(x);
        if (x_uncle->color == red)
        {
            // color the parent and the uncle black, grandparent red
            x->parent->color = black;
            x_uncle->color = black;
            x_grandparent->color = red;
            fixredblack_insert(x_grandparent); 
        }else{
            // uncle's color is black
            return fixredblack_insert_case2(x);
        }
    }else{
        return x;
    }
}

BinarySearchTree* fixredblack_insert_case2(BinarySearchTree* x){

}