/*
 *  vnode.h
 *  Currency Converter
 *
 *  Created by Mike on 8/1/13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef	_VNODE_H
#define	_VNODE_H	1

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "msgNode.h"


//variable node
typedef struct vnode_s {
	char *eqn; //holds the that defined the variable as text.
	double value; //variable's last calculated value.
	char *name; //name the node. used to recall variables and trees.
	bool accessMark; //the access mark signals that this variable is relevant to the current calculation
	struct vnode_s *next_variable; //points at the next variable in the variable list
	struct vlink_s *variables;   //a list of variables that affect this vnode.
	struct vlink_s *dependents; //a list of variables that this vnode affects.
} vnode_t;

/************************************************************
 *vlinks structure
 *manages relationships between variables.
 *use this to link related nodes so that when a variable is re-defined, the dependent equations
 *can be automatically re-calculated.
 *equations link to needed variables so that old links can be erased if the equation changes
 *variables link to equations so that the equations can be re-calculated if 
 *by looking for loops in the vlinks, self referential equations can be discovered.
 ************************************************************/

typedef struct vlink_s {
   vnode_t *variable; //points at the function's variable
   vnode_t *function;  //points at the function
	 struct vlink_s *v_prev;  //variable's previous vlink
	 struct vlink_s *v_next;  //variable's next vlink
	 struct vlink_s *f_prev;  //function's previous vlink
	 struct vlink_s *f_next;  //function's next vlink
} vlink_t;

//holds list of variables that need to be recalculated after a functions
//variable has been changed
vnode_t *lastRecalcFunction;

//these functions manage variables which are stored in a linked list
extern char *printVariables(vnode_t *root);
int getMaxNameLength(vnode_t *variable_list);
msgNode_t *printVarsRecurse(vnode_t *root, msgNode_t *msg, int nameLength);
vnode_t *copyVarsRecurse(vnode_t *variable_list, vnode_t *newcopy);
extern vnode_t *searchVariables(vnode_t *variable_list, char *name);
extern vnode_t *addConstant(vnode_t *root, char *name,  double value);
extern vnode_t *addVariable(vnode_t *root, char *name,  char *eqn);
void addEquation(vnode_t *var, char *eqn);
void addName(vnode_t *var, char *name);
char *getVariableEquation(vnode_t *var);
char *getVariableName(vnode_t *var);
double getVariableValue(vnode_t *var);
void setVariableValue(vnode_t *var, double value);
extern vnode_t *newVariable(void);
extern vnode_t *trashVariables(vnode_t *variables);
extern int variableListLength(vnode_t *variables);
extern vnode_t *variableAtIndex(vnode_t *variables, int indexTarget, int index);

//vlinks functions
vlink_t *newVlink(void);
void linkVars(vnode_t *function, vnode_t *variable);
void pushDependentLink(vnode_t *variable, vlink_t *link);
void pushVariableLink(vnode_t *function, vlink_t *link);
vnode_t *getRecalcList(vnode_t *function);
msgNode_t *printDependentLinks(vnode_t *function, msgNode_t *messages);
void setAccessMark(vnode_t *variable);
vlink_t *trashVlinks(vlink_t *link); //private
void removeFunctionVlinks(vnode_t *function);
int cleanAccessMarks(vnode_t *variable);
bool getRecalcStrings( char **eqn, char **name);
msgNode_t *reportVariableData(vnode_t *listHead, char *name, msgNode_t *messages);

#endif

