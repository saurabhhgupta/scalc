//cnode.h
#if !defined (CNODE_H_INCL)
#define CNODE_H_INCL

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

typedef enum {
	CNODE_NUMBER, 
	CNODE_OPERATOR, 
} cnodeType; 


typedef struct cnode_s {
	cnodeType nodeType;
	char op;
	double number; //points to memory holding the value.  Can also point to a variable's value.
	struct cnode_s *left;
	struct cnode_s *right;
} cnode_t;

extern bool show_steps;

cnode_t *newCnode( char op, double number, cnodeType nodeType);
void setCnodeOperator(cnode_t *cnode, char operator);
void setCnodeNumber(cnode_t *cnode, double number);
char getCnodeOperator(cnode_t *cnode);
double getCnodeNumber(cnode_t *cnode);

cnode_t *insertOp(cnode_t *root, char op);
cnode_t *insertNumber(cnode_t *root, double number);
bool isOpNode( cnode_t *root );
bool isNumberNode( cnode_t *root );
//extern void printTree(cnode_t *root);
extern double calcTree(cnode_t *root);
cnode_t *cnodeTrash(cnode_t *root);
int precedence(char op);

#endif
