/*
 *	Operating System Lab
 *	    Lab2 (Synchronization)
 *	    Student id :
 *	    Student name :
 *
 *   lab2_bst.c :
 *       - thread-safe bst code.
 *       - coarse-grained, fine-grained lock code
 *
 *   Implement thread-safe bst for coarse-grained version and fine-grained
 *version.
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/lab2_sync_types.h"

#define DEBUG
#define fg_remove

/* This is Global mutex variable*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * TODO
 *  Implement funtction which traverse BST in in-order
 *
 *  @param lab2_tree *tree  : bst to print in-order.
 *  @return                 : status (success or fail)
 */
int lab2_node_print_inorder(lab2_node *node) {
        if (!node) {
                printf("Tree is empty!\n");
                return -1;
        }

        if (node->left)
                lab2_node_print_inorder(node->left);
        printf("%d ", node->key);
        if (node->right)
                lab2_node_print_inorder(node->right);
        return 0;
}

/*
 * TODO
 *  Implement function which creates struct lab2_tree
 *  ( refer to the ./include/lab2_sync_types.h for structure lab2_tree )
 *
 *  @return                 : bst which you created in this function.
 */
lab2_tree *lab2_tree_create() {
        // You need to implement lab2_tree_create function.
        lab2_tree *tree = (lab2_tree *)malloc(sizeof(lab2_tree));
        tree->root = NULL;
        // pthread_mutex_init(&(tree->root->mutex), NULL);

        return tree;
}

/*
 * TODO
 *  Implement function which creates struct lab2_node
 *  ( refer to the ./include/lab2_sync_types.h for structure lab2_node )
 *
 *  @param int key          : bst node's key to creates
 *  @return                 : bst node which you created in this function.
 */
lab2_node *lab2_node_create(int key) {
        // You need to implement lab2_node_create function.
        lab2_node *node = (lab2_node *)malloc(sizeof(lab2_node));

        // pthread_mutex_init(&node->mutex, NULL);
        node->key = key;
        node->left = node->right = NULL;
        pthread_mutex_init(&(node->mutex), NULL);
        pthread_rwlock_init(&(node->rwlock), NULL);

        return node;
}

/*
 * TODO
 *  Implement a function which insert nodes from the BST.
 *
 *  @param lab2_tree *tree      : bst which you need to insert new node.
 *  @param lab2_node *new_node  : bst node which you need to insert.
 *  @return                 : satus (success or fail)
 */
int lab2_node_insert(lab2_tree *tree, lab2_node *new_node) {
        // You need to implement lab2_node_insert function.

        struct lab2_node *curNode = tree->root;
        struct lab2_node *parNode = NULL;

        while (curNode) {
                parNode = curNode;
                if (new_node->key == curNode->key)
                        return 1;
                if (new_node->key < curNode->key)
                        curNode = curNode->left;
                else
                        curNode = curNode->right;
        }

        if (!parNode)
                tree->root = new_node;
        else if (new_node->key < parNode->key)
                parNode->left = new_node;
        else
                parNode->right = new_node;

        return 0;
}

/*
 * TODO
 *  Implement a function which insert nodes from the BST in fine-garined manner.
 *
 *  @param lab2_tree *tree      : bst which you need to insert new node in
 * fine-grained manner.
 *  @param lab2_node *new_node  : bst node which you need to insert.
 *  @return                     : status (success or fail)
 */
int lab2_node_insert_fg(lab2_tree *tree, lab2_node *new_node) {
        // You need to implement lab2_node_insert_fg function.

        // pthread_mutex_lock(&mutex);
        lab2_node *curNode = tree->root;
        lab2_node *parNode = NULL;

        // printf("thread id : %ld , node key : %d\n", pthread_self(),
        //        new_node->key);

        while (curNode) {
                parNode = curNode;
                if (new_node->key == curNode->key) {
                        // pthread_mutex_unlock(&mutex);
                        return 1;
                }

                else if (new_node->key < curNode->key)
                        curNode = curNode->left;

                else
                        curNode = curNode->right;
        }

        if (!parNode) {
                tree->root = new_node;
                return 0;
        }

        pthread_mutex_t node_mutex = parNode->mutex;
        pthread_mutex_lock(&node_mutex);

        if (new_node->key < parNode->key)
                parNode->left = new_node;
        else
                parNode->right = new_node;

        // pthread_mutex_unlock(&mutex);
        pthread_mutex_unlock(&node_mutex);
        return 0;
}

/*
 * TODO
 *  Implement a function which insert nodes from the BST in coarse-garined
 * manner.
 *
 *  @param lab2_tree *tree      : bst which you need to insert new node in
 * coarse-grained manner.
 *  @param lab2_node *new_node  : bst node which you need to insert.
 *  @return                     : status (success or fail)
 */
int lab2_node_insert_cg(lab2_tree *tree, lab2_node *new_node) {
        // You need to implement lab2_node_insert_cg function.
        pthread_mutex_lock(&mutex);

        lab2_node *curNode = tree->root;
        lab2_node *parNode = NULL;

        // printf("thread id : %ld , node key : %d\n", pthread_self(),
        //        new_node->key);

        while (curNode) {
                parNode = curNode;
                if (new_node->key == curNode->key) {
                        pthread_mutex_unlock(&mutex);
                        return 1;
                }
                if (new_node->key < curNode->key)
                        curNode = curNode->left;
                else
                        curNode = curNode->right;
        }

        if (!parNode)
                tree->root = new_node;
        else if (new_node->key < parNode->key)
                parNode->left = new_node;
        else
                parNode->right = new_node;

        pthread_mutex_unlock(&mutex);
        return 0;
}

/*
 * TODO
 *  Implement a function which remove nodes from the BST.
 *
 *  @param lab2_tree *tree  : bst tha you need to remove node from bst which
 * contains key.
 *  @param int key          : key value that you want to delete.
 *  @return                 : status (success or fail)
 */
int lab2_node_remove(lab2_tree *tree, int key) {
        lab2_node *curNode = tree->root;
        lab2_node *parNode = NULL;

        /* Find the Node which contains given key*/
        while (curNode) {
                if (curNode->key == key)
                        break;
                else if (curNode->key > key) {
                        parNode = curNode;
                        curNode = curNode->left;
                } else {
                        parNode = curNode;
                        curNode = curNode->right;
                }
        }

        /* If there is no key return -1 */
        if (!curNode) {
#ifdef DEBUG
                printf("There is no such key : %d\n", key);
#endif
                return -1;
        }

        /* remove root node*/
        if (!parNode) {
                if (!curNode->left && !curNode->right) {
                        tree->root = NULL;
                }

                else if (!curNode->left != !curNode->right) {
                        if (curNode->right) {
                                lab2_node *tmp = curNode->right;
                                curNode->key = curNode->right->key;
                                curNode->right = curNode->right->right;
                                lab2_node_delete(tmp);
                        } else {
                                lab2_node *tmp = curNode->left;
                                curNode->key = curNode->left->key;
                                curNode->left = curNode->left->left;
                                lab2_node_delete(tmp);
                        }
                }

                else {
                        /* Find least key node */
                        lab2_node *leastFromRight = curNode->right;
                        lab2_node *parLFR = curNode;
                        while (leastFromRight->left) {
                                parLFR = leastFromRight;
                                leastFromRight = leastFromRight->left;
                        }

                        /* Swap the value and adjust pointer*/
                        curNode->key = leastFromRight->key;
                        if (parLFR->right == leastFromRight) {
                                parLFR->right = leastFromRight->right;
                                lab2_node_delete(leastFromRight);
                        } else {
                                parLFR->left = leastFromRight->right;
                                lab2_node_delete(leastFromRight);
                        }
                }
                return 0;
        }

        /* CASE 1 */
        if (!curNode->left && !curNode->right) {
                if (parNode->right)
                        if (parNode->right->key == curNode->key)
                                parNode->right = NULL;

                if (parNode->left)
                        if (parNode->left->key == curNode->key)
                                parNode->left = NULL;
                lab2_node_delete(curNode);
        }

        /* CASE 2 */
        else if (!curNode->left != !curNode->right) {
                if (parNode->right) {
                        if (parNode->right->key == curNode->key) {
                                if (curNode->right)
                                        parNode->right = curNode->right;
                                else
                                        parNode->right = curNode->left;
                        }
                }

                if (parNode->left) {
                        if (parNode->left->key == curNode->key) {
                                if (curNode->right)
                                        parNode->left = curNode->right;
                                else
                                        parNode->left = curNode->left;
                        }
                }
                lab2_node_delete(curNode);
        }

        /* CASE 3 */
        else {
                /* Find least key node */
                lab2_node *leastFromRight = curNode->right;
                lab2_node *parLFR = curNode;
                while (leastFromRight->left) {
                        parLFR = leastFromRight;
                        leastFromRight = leastFromRight->left;
                }

                /* Swap the value and adjust pointer*/
                curNode->key = leastFromRight->key;
                if (parLFR->right == leastFromRight) {
                        parLFR->right = leastFromRight->right;
                        lab2_node_delete(leastFromRight);
                } else {
                        parLFR->left = leastFromRight->right;
                        lab2_node_delete(leastFromRight);
                }
        }
        return 0;
}

/*
 * TODO
 *  Implement a function which remove nodes from the BST in fine-grained manner.
 *
 *  @param lab2_tree *tree  : bst tha you need to remove node in fine-grained
 * manner from bst which contains key.
 *  @param int key          : key value that you want to delete.
 *  @return                 : status (success or fail)
 */
int lab2_node_remove_fg(lab2_tree *tree, int key) {
        lab2_node *curNode = tree->root;
        lab2_node *parNode = NULL;

        pthread_rwlock_t rwlock;
        /* Find the Node which contains given key*/
        while (curNode) {
                rwlock = curNode->rwlock;
                pthread_rwlock_rdlock(&rwlock);
                if (curNode->key == key) {
                        /* Question
                        1. Can wrlock overwrite(?) upon read lock?
                                -> If possible, there is no need to unlock here
                        2. Is **_unlock function sending signal to others?
                        */
                        pthread_rwlock_unlock(&rwlock);
                        pthread_rwlock_wrlock(&rwlock);
                        break;
                } else if (curNode->key > key) {
                        parNode = curNode;
                        curNode = curNode->left;
                } else {
                        parNode = curNode;
                        curNode = curNode->right;
                }
                pthread_rwlock_unlock(&rwlock);
        }

        /* If there is no key return -1 */
        if (!curNode) {
#ifdef DEBUG
                printf("There is no such key : %d\n", key);
#endif
                return -1;
        }
#ifdef fg_remove
        printf("Thread id : %ld del key : %d\n", pthread_self(), key);
        // printf("\n");
#endif
        /* remove root node*/
        if (!parNode) {
                /* Since root nodes has no parent node, use global mutex lock*/
                if (!curNode->left && !curNode->right) {
                        tree->root = NULL;
                }

                else if (!curNode->left != !curNode->right) {
                        if (curNode->right) {
                                lab2_node *tmp = curNode->right;
                                curNode->key = curNode->right->key;
                                curNode->right = curNode->right->right;
                                lab2_node_delete(tmp);
                        } else {
                                lab2_node *tmp = curNode->left;
                                curNode->key = curNode->left->key;
                                curNode->left = curNode->left->left;
                                lab2_node_delete(tmp);
                        }
                }

                else {
                        /* Find least key node */
                        lab2_node *leastFromRight = curNode->right;
                        lab2_node *parLFR = curNode;
                        while (leastFromRight->left) {
                                parLFR = leastFromRight;
                                leastFromRight = leastFromRight->left;
                        }

                        /* Swap the value and adjust pointer*/
                        curNode->key = leastFromRight->key;
                        if (parLFR->right == leastFromRight) {
                                parLFR->right = leastFromRight->right;
                                lab2_node_delete(leastFromRight);
                        } else {
                                parLFR->left = leastFromRight->right;
                                lab2_node_delete(leastFromRight);
                        }
                }
                pthread_rwlock_unlock(&rwlock);
                return 0;
        }

        /* CASE 1 */
        if (!curNode->left && !curNode->right) {
                if (parNode->right)
                        if (parNode->right->key == curNode->key)
                                parNode->right = NULL;

                if (parNode->left)
                        if (parNode->left->key == curNode->key)
                                parNode->left = NULL;
                lab2_node_delete(curNode);
        }

        /* CASE 2 */
        else if (!curNode->left != !curNode->right) {
                if (parNode->right) {
                        if (parNode->right->key == curNode->key) {
                                if (curNode->right)
                                        parNode->right = curNode->right;
                                else
                                        parNode->right = curNode->left;
                        }
                }

                if (parNode->left) {
                        if (parNode->left->key == curNode->key) {
                                if (curNode->right)
                                        parNode->left = curNode->right;
                                else
                                        parNode->left = curNode->left;
                        }
                }
                lab2_node_delete(curNode);
        }

        /* CASE 3 */
        else {
                /* Find least key node */
                lab2_node *leastFromRight = curNode->right;
                lab2_node *parLFR = curNode;
                while (leastFromRight->left) {
                        parLFR = leastFromRight;
                        leastFromRight = leastFromRight->left;
                }

                /* Swap the value and adjust pointer*/
                curNode->key = leastFromRight->key;
                if (parLFR->right == leastFromRight) {
                        parLFR->right = leastFromRight->right;
                        lab2_node_delete(leastFromRight);
                } else {
                        parLFR->left = leastFromRight->right;
                        lab2_node_delete(leastFromRight);
                }
        }
        pthread_rwlock_unlock(&rwlock);
        return 0;
}

/*
 * TODO
 *  Implement a function which remove nodes from the BST in coarse-grained
 * manner.
 *
 *  @param lab2_tree *tree  : bst tha you need to remove node in coarse-grained
 * manner from bst which contains key.
 *  @param int key          : key value that you want to delete.
 *  @return                 : status (success or fail)
 */
int lab2_node_remove_cg(lab2_tree *tree, int key) {
        pthread_mutex_lock(&mutex);
        lab2_node *curNode = tree->root;
        lab2_node *parNode = NULL;

        /* Find the Node which contains given key*/
        while (curNode) {
                if (curNode->key == key)
                        break;
                else if (curNode->key > key) {
                        parNode = curNode;
                        curNode = curNode->left;
                } else {
                        parNode = curNode;
                        curNode = curNode->right;
                }
        }

        /* If there is no key return -1 */
        if (!curNode) {
#ifdef DEBUG
                printf("There is no such key : %d\n", key);
#endif
                pthread_mutex_unlock(&mutex);
                return -1;
        }

        /* remove root node*/
        if (!parNode) {
                if (!curNode->left && !curNode->right) {
                        tree->root = NULL;
                }

                else if (!curNode->left != !curNode->right) {
                        if (curNode->right) {
                                lab2_node *tmp = curNode->right;
                                curNode->key = curNode->right->key;
                                curNode->right = curNode->right->right;
                                lab2_node_delete(tmp);
                        } else {
                                lab2_node *tmp = curNode->left;
                                curNode->key = curNode->left->key;
                                curNode->left = curNode->left->left;
                                lab2_node_delete(tmp);
                        }
                }

                else {
                        /* Find least key node */
                        lab2_node *leastFromRight = curNode->right;
                        lab2_node *parLFR = curNode;
                        while (leastFromRight->left) {
                                parLFR = leastFromRight;
                                leastFromRight = leastFromRight->left;
                        }

                        /* Swap the value and adjust pointer*/
                        curNode->key = leastFromRight->key;
                        if (parLFR->right == leastFromRight) {
                                parLFR->right = leastFromRight->right;
                                lab2_node_delete(leastFromRight);
                        } else {
                                parLFR->left = leastFromRight->right;
                                lab2_node_delete(leastFromRight);
                        }
                }
                pthread_mutex_unlock(&mutex);
                return 0;
        }

        /* CASE 1 */
        if (!curNode->left && !curNode->right) {
                if (parNode->right)
                        if (parNode->right->key == curNode->key)
                                parNode->right = NULL;

                if (parNode->left)
                        if (parNode->left->key == curNode->key)
                                parNode->left = NULL;
                lab2_node_delete(curNode);
        }

        /* CASE 2 */
        else if (!curNode->left != !curNode->right) {
                if (parNode->right) {
                        if (parNode->right->key == curNode->key) {
                                if (curNode->right)
                                        parNode->right = curNode->right;
                                else
                                        parNode->right = curNode->left;
                        }
                }

                if (parNode->left) {
                        if (parNode->left->key == curNode->key) {
                                if (curNode->right)
                                        parNode->left = curNode->right;
                                else
                                        parNode->left = curNode->left;
                        }
                }
                lab2_node_delete(curNode);
        }

        /* CASE 3 */
        else {
                /* Find least key node */
                lab2_node *leastFromRight = curNode->right;
                lab2_node *parLFR = curNode;
                while (leastFromRight->left) {
                        parLFR = leastFromRight;
                        leastFromRight = leastFromRight->left;
                }

                /* Swap the value and adjust pointer*/
                curNode->key = leastFromRight->key;
                if (parLFR->right == leastFromRight) {
                        parLFR->right = leastFromRight->right;
                        lab2_node_delete(leastFromRight);
                } else {
                        parLFR->left = leastFromRight->right;
                        lab2_node_delete(leastFromRight);
                }
        }
        pthread_mutex_unlock(&mutex);
        return 0;
}

/*
 * TODO
 *  Implement function which delete struct lab2_tree
 *  ( refer to the ./include/lab2_sync_types.h for structure lab2_node )
 *
 *  @param lab2_tree *tree  : bst which you want to delete.
 *  @return                 : status(success or fail)
 */
void lab2_tree_delete(lab2_tree *tree) {
        // You need to implement lab2_tree_delete function.
        free(tree);
}

/*
 * TODO
 *  Implement function which delete struct lab2_node
 *  ( refer to the ./include/lab2_sync_types.h for structure lab2_node )
 *
 *  @param lab2_tree *tree  : bst node which you want to remove.
 *  @return                 : status(success or fail)
 */
void lab2_node_delete(lab2_node *node) {
        // You need to implement lab2_node_delete function.
        free(node);
}

int isTreeEmpty(lab2_tree *tree) { return tree->root == NULL; }