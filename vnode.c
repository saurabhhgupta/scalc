/*
 *  vnode.c
 *  Currency Converter
 *
 *  Created by Mike on 8/1/13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 * Lingo:
 *	functions and constants are all variables.
 *  constants simply do not have dependents where
 *  the value of functions can depend on the values of other constants and/or other functions
 */

#include "vnode.h"

static msgNode_t *followDepLinksAndPrint(vlink_t *deplink, msgNode_t *messages);
static msgNode_t *followVarLinksAndPrint(vlink_t *varlink, msgNode_t *messages);
static vnode_t *copyVariable(vnode_t *original);
static vnode_t *buildRecalcList(vnode_t *head, vlink_t *deplink);
static vnode_t *addCopyToRecalcList(vnode_t *this, vnode_t *copy);

vnode_t *trashVariables(vnode_t *variable)
{
	if(variable != NULL){
		removeFunctionVlinks(variable);
		//printf("trash %s\n",variable->name);
		variable->next_variable = trashVariables(variable->next_variable);
		free(variable->eqn);
		free(variable->name);
		free(variable);
		variable = NULL;
	}
	return variable;
}

vnode_t *newVariable(void){
	vnode_t* new;
	new = (vnode_t *)malloc(sizeof(vnode_t));
	new->eqn        = NULL;
	new->value      = 0;
	new->accessMark = false;
	new->next_variable = NULL;
	new->dependents = NULL;
	new->variables = NULL;
	return new;
}

vnode_t *addConstant(vnode_t *root, char *name,  double value){
	vnode_t* var;
	var = searchVariables(root, name);
	if(var == NULL){
		var = newVariable();
		addName(var,name);      
		var->value = value;
		var->eqn = NULL;
		var->next_variable = root;
		root = var;
	}else{
		if(var->eqn != NULL){
			free(var->eqn);
			var->eqn = NULL;
		}
		var->value = value;
	}
	return root;
}

//push a new variable into the variable stack
vnode_t *addVariable(vnode_t *root, char *name,  char *eqn){
	vnode_t* var;
	var = searchVariables(root, name);
	if(var == NULL){
		var = newVariable();
		addName(var,name);      
		addEquation(var,eqn);
		var->next_variable = root;
		root = var;
	}else{
		free(var->eqn);
		addEquation(var,eqn);
	}
	removeFunctionVlinks(var);
	linkVars(var,root);
	return root;
}

static vnode_t *copyVariable(vnode_t *original)
{
	vnode_t* new = newVariable();
	addName(new,original->name);
	addEquation(new,original->eqn);
	new->value      = original->value;
	new->accessMark = original->accessMark;
	new->next_variable = NULL;
	new->dependents = NULL;
	new->variables = NULL;
	return new;
}

void addEquation(vnode_t *var, char* eqn){
	//allocate memory and copy the equation into the data structure
	if(eqn == NULL) return;
	var->eqn = (char *)calloc(strlen(eqn)+1,sizeof(char));
	var->eqn[0] = '\0';
	strcpy(var->eqn,eqn);
}

void addName(vnode_t *var, char *name){
	if(name == NULL) return;
	var->name = (char *)calloc(strlen(name)+1,sizeof(char));
	var->name[0] = '\0';
	strcpy(var->name,name);
}

char *getVariableEquation(vnode_t *var){
	//returns a copy of the thing pointed to by *var
	char *eqn_copy;
	if(var == NULL) return(NULL);
	if(var->eqn == NULL) return(NULL);
	eqn_copy = (char *)calloc(strlen(var->eqn)+1,sizeof(char));
	eqn_copy[0] = '\0';
	strcpy(eqn_copy,var->eqn);
	return(eqn_copy);
}

char *getVariableName(vnode_t *var){
	//returns a copy of the thing pointed to by *var
	char *name_copy;
	if(var == NULL) return(NULL);
	if(var->name == NULL) return(NULL);
	name_copy = (char *)calloc(strlen(var->name)+1,sizeof(char));
	name_copy[0] = '\0';
	strcpy(name_copy,var->name);
	return(name_copy);
}

double getVariableValue(vnode_t *var)
{
	return var->value;
}

void setVariableValue(vnode_t *var, double value)
{
	var->value = value;
}

vnode_t *searchVariables(vnode_t *variable, char *name){
	if(variable == NULL) return(NULL); 
	//printf("%s\t%s\n",variable_list->name,name);
	if(strcmp(variable->name,name) == 0){

		//ans should probably not get linked.
		//it gets reset after every equation as a convenience
		//and should never be set by the user even though the 
		//program does not block it.
		if(strcmp(variable->name,"ans")){
			setAccessMark(variable);
		}
		return(variable);
	}
	return searchVariables(variable->next_variable, name);
}

char *printVariables(vnode_t *variable_list){
	char *answerString;
	int nameLength = getMaxNameLength(variable_list);
	msgNode_t *msg = queueMessageSprintf(NULL,ANSWER,
			"%*s  %12s  %s",nameLength,"name","value","equation");
	msg = printVarsRecurse(variable_list,msg,nameLength);
	answerString = getMessageString(msg);
	msgNodeTrash(msg);
	return answerString;
}

msgNode_t *printVarsRecurse(vnode_t *variable_list, msgNode_t *msg, int nameLength)
{
	if(variable_list == NULL) return msg;
	if(variable_list->eqn == NULL){
		msg = queueMessageSprintf(msg,ANSWER,
				"%*s  %12g",
				nameLength,variable_list->name,variable_list->value);
	}else{
		msg = queueMessageSprintf(msg,ANSWER,
				"%*s  %12g  %s",
				nameLength,variable_list->name,variable_list->value,variable_list->eqn);
	}
	return printVarsRecurse(variable_list->next_variable, msg, nameLength);
}

vnode_t *copyVarsRecurse(vnode_t *variable_list, vnode_t *newcopy)
{
	if(variable_list == NULL) return newcopy;
	if(variable_list->eqn == NULL){
		newcopy = addConstant(newcopy,variable_list->name,variable_list->value);
	}else{ //this one is holding an equation
		newcopy = addVariable(newcopy,variable_list->name,variable_list->eqn);
	}
	return copyVarsRecurse(variable_list->next_variable, newcopy);
}

//finds the length of the longest variable in the variable_list
//used for pretty printing the calculator's memory
//returns the length of the longest variable name
int getMaxNameLength(vnode_t *variable_list)
{
	if(variable_list == NULL) return 0;
	int longest = getMaxNameLength(variable_list->next_variable);
	int thisLength = strlen(variable_list->name);
	if(thisLength > longest) return thisLength;
	return longest;
}

int variableListLength(vnode_t *variables)
{
	if(variables == NULL) return 0;
	return 1+variableListLength(variables->next_variable);
}

vnode_t *variableAtIndex(vnode_t *variables, int indexTarget, int index)
{
	if(variables == NULL) return NULL;
	if(indexTarget == index) return variables;
	return variableAtIndex(variables->next_variable, indexTarget, index+1);
}


/*****
 * Vlinks....
 *****/

vlink_t *newVlink(void)
{
	vlink_t *new = (vlink_t *)malloc(sizeof(vlink_t));
	new->variable = NULL; //points at the function's variable
	new->function = NULL; //points at the function
	new->v_prev   = NULL; //variable's previous vlink
	new->v_next   = NULL; //variable's next vlink
	new->f_prev   = NULL; //function's previous vlink
	new->f_next   = NULL; //function's next vlink
	return new;
}


void pushVariableLink(vnode_t *function, vlink_t *newlink){
	vlink_t *oldLink = function->variables;
	if(oldLink == NULL){
		//should not need this if the vlink was initalized properly.
		newlink->f_next = NULL;
		newlink->f_prev = NULL;
	}else{
		newlink->f_prev = oldLink->v_prev;
		newlink->f_next = oldLink;
		oldLink->f_prev = newlink;
	}
	function->variables = newlink;
}

void pushDependentLink(vnode_t *variable, vlink_t *newlink){
	vlink_t *oldLink = variable->dependents;
	if(variable->dependents == NULL){
		//should not need this if the vlink was initalized properly.
		newlink->v_next = NULL;
		newlink->v_prev = NULL;
	}else{
		newlink->v_prev = oldLink->v_prev; //this should always be null.
		newlink->v_next = oldLink;
		oldLink->v_prev = newlink;
	}
	variable->dependents = newlink;
}

//call this after creating a new Variable node.
void linkVars(vnode_t *function, vnode_t *variable)
{

	if(variable == NULL) return ; 
	if(variable->accessMark && variable != function){
		// accessed variables only after this point.

		//create vlinks based on marked vnodes in the variable list.
		vlink_t *link = newVlink();

		//build the list of vlinks with respect to the function.
		//set links to the variables as vlinks are created.
		link->variable = variable;
		link->function = function;

		//set links to the functions other vlinks as the vlinks are created.
		pushVariableLink(function,link);

		//once the function's list is populated, populate the list from 
		//the point of view of the variables.
		pushDependentLink(variable,link);
		//printf("Linked: %s to %s\n",function->name,variable->name);
	}
	linkVars(function,variable->next_variable);
}

vnode_t *getRecalcList(vnode_t *variable)
{
	vnode_t *head = NULL;
	lastRecalcFunction = NULL;
	if(variable == NULL) return NULL;
	if(variable->dependents == NULL) return NULL;
	head = buildRecalcList(head,variable->dependents);
	lastRecalcFunction = head;
	return head;
}

static vnode_t *buildRecalcList(vnode_t *head, vlink_t *deplink)
{
	if(deplink == NULL) return NULL;
	//need to run down all of the distant dependents.
	//this is effectively turning a binary tree into a list.
	if(head == NULL){
		head = addCopyToRecalcList(head,deplink->function);
	}else{
		addCopyToRecalcList(head,deplink->function);
	}
	addCopyToRecalcList(head,buildRecalcList(head, deplink->function->dependents));
	addCopyToRecalcList(head,buildRecalcList(head, deplink->v_next));
	//return the link to it.
	return head;
}

static vnode_t *addCopyToRecalcList(vnode_t *this, vnode_t *copy)
{
	if(this == NULL){
		//printf("addCopyToRecalcList: %s\n",copy->name);
		return copyVariable(copy);
	}
	if(copy == NULL) return NULL;
	if(strcmp(this->name,copy->name)) //see if the copy is alread in the list.
	{ //don't add the function for recalculation when it is already in the list.
		this->next_variable = addCopyToRecalcList(this->next_variable,copy);
	}
	return this;
}

bool getRecalcStrings(char **eqn, char **name)
{
	if(lastRecalcFunction==NULL) return false;
	//printf("\n%s",lastRecalcFunction->name);
	*eqn   = lastRecalcFunction->eqn;
	*name  = lastRecalcFunction->name;
	lastRecalcFunction = lastRecalcFunction->next_variable;
	return true;
}

msgNode_t *printDependentLinks(vnode_t *function, msgNode_t *messages)
{
	if(function == NULL) return 0;
	if(function->dependents == NULL) return(0);
	return followDepLinksAndPrint(function->dependents, messages);
}

static msgNode_t *followDepLinksAndPrint(vlink_t *deplink, msgNode_t *messages)
{
	if(deplink == NULL) return messages;
	messages = queueMessageSprintf(messages,ANSWER,
			"%s(%s) = %s",deplink->function->name,deplink->variable->name,deplink->function->eqn);
	return followDepLinksAndPrint(deplink->v_next, messages);
}

static msgNode_t *followVarLinksAndPrint(vlink_t *varlink, msgNode_t *messages)
{
	if(varlink == NULL) return messages;
	messages = queueMessageSprintf(messages,ANSWER,
			"%s(%s)",varlink->function->name,varlink->variable->name);
	return followVarLinksAndPrint(varlink->f_next, messages);
}

void removeFunctionVlinks(vnode_t *function)
{
	if(function == NULL) return; 
	if(function->variables == NULL) return;
	function->variables = trashVlinks(function->variables);
	//remove link from the dependents list v_prev->v_next = v_next
	//remove link to the variable
	//remove link from the function's list f_prev->f_next = f_next
}

vlink_t *trashVlinks(vlink_t *link)
{
	if(link == NULL) return NULL;
	//recurse to the bottom of the functions link list.
	link->f_next = trashVlinks(link->f_next); 

	//remove this vlink from the variable's vlink list.
	//if(link->v_prev == NULL){ //this link must be on the top of the variable's dependent stack
	if(link->variable->dependents == link){
		//printf("\nremoving last dependent link from %s",link->variable->name);
		link->variable->dependents = link->v_next; //removed the pointer from the variable to this vlink
	}else{
		link->v_prev->v_next = link->v_next;
	}

	//delete the node.
	free(link);
	return NULL;
}


/*
 *vnodes are searched and returned using the searchVariables function
 *when a vnode is found, set the accessMark it as used.  The mark can be used to not
 *print the vnode's value twice when an equation references it twice.
 *they also can be used to build a list of vnodes that need to be linked 
 *to dependent functions.
 */
void setAccessMark(vnode_t *variable)
{
	variable->accessMark = true;
}

int cleanAccessMarks(vnode_t *variable)
{
	int marked = 0;
	if(variable == NULL) return 0;
	if(variable->accessMark){
		marked = 1;
	}
	variable->accessMark = false;
	return marked + cleanAccessMarks(variable->next_variable);
}

msgNode_t *reportVariableData(vnode_t *listHead, char *name, msgNode_t *messages)
{
	if(listHead == NULL) return messages; 
	//printf("%s\t%s\n",listHead->name,name);
	if(strcmp(listHead->name,name) == 0){
		if(listHead->eqn != NULL){
			messages = queueMessageSprintf(messages,ANSWER,
					"%s=%s",listHead->name,listHead->eqn);
		}else{
			messages = queueMessageSprintf(messages,ANSWER,
					"%s=%g",listHead->name,listHead->value);
		}
		messages = followDepLinksAndPrint(listHead->dependents,messages);
		//messages = followVarLinksAndPrint(listHead->variables,messages);
		return messages;
	}
	return reportVariableData(listHead->next_variable, name, messages);
}
