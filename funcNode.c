#include "funcNode.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

funcNode_t *newNode( const char *name, fp func_p ){
    funcNode_t *node;
    node = (funcNode_t *)malloc(sizeof(funcNode_t));
    node->name = (char *)calloc(strlen(name)+1,sizeof(char));
		node->name[0] = '\0';
		strcpy(node->name,name);
    node->func_p = func_p;
    node->next = NULL;
    return node;
}

//insert a function node in the list.
funcNode_t *pushNode(funcNode_t *root, char *name, fp func_p){
    funcNode_t *node;
    node = newNode(name,func_p);
    node->next = root;
    root = node;
    return root;
}

funcNode_t *funcNodeTrash(funcNode_t *root){
    if(root!=NULL){
        root->next = funcNodeTrash(root->next);
        free(root->name);
        free(root);
        root = NULL;
    }
    return root;
}

funcNode_t *getFunc(char *parse_p, funcNode_t *root){
   if(root == NULL) return(NULL);
   if(0 == strncmp(parse_p,root->name,strlen(root->name))){
      return(root); 
   }else{
      return(getFunc(parse_p,root->next));
   }
   return(NULL); //should never get here
}


void printList(funcNode_t *root){
   if(root == NULL) return;
   printList(root->next);
   printf("%s)\n",root->name);
}

