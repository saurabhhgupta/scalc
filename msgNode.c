/*
 *  msgNode.c
 *  Currency Converter
 *
 *  Created by Mike on 7/27/13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "msgNode.h"

unsigned int messageMask = 0;

static msgNode_t *newMessageNode( char *message, messageType type);
static int messageLength(msgNode_t *root);
static void catMessages(msgNode_t *root, char *messageString);

unsigned int setMessageMask(messageType type)
{
	messageMask |= type;
	//printf("messageMask = %02x\n",messageMask);
	return messageMask;
}

unsigned int unsetMessageMask(messageType type)
{
	messageMask &= ~type;
	//printf("messageMask = %02x\n",messageMask);
	return messageMask;
}

msgNode_t *newMessageNode( char *message, messageType type)
{
    msgNode_t *new;
    new = (msgNode_t *)malloc(sizeof(msgNode_t));
		new->message = (char *)calloc(strlen(message)+2,sizeof(char));
		new->message[0] = '\0';
		strcpy(new->message,message);
    new->length  = strlen(message);
		new->type    = type;
		new->next    = NULL;
    return new;
}

/************************************************************
 *inserts a message at the end of the message queue
 ************************************************************/
msgNode_t *queueMessage(msgNode_t *root, char *message, messageType type)
{
	if(root == NULL){
		root = newMessageNode(message, type);
	}else if((type == EQUATION || type == VARIABLE) && !strcmp(root->message,message)){
			//don't print variable and equation decodes more than once.
			return root;
	}else{
		root->next = queueMessage( root->next, message, type);
	}
	return root;
}

/************************************************************
 *inserts a message at the front of the message queue
 ************************************************************/
msgNode_t *pushMessage(msgNode_t *root, char *message, messageType type)
{
	msgNode_t *new;
	new = newMessageNode(message, type);
	new->next = root;
	return new;
}


/************************************************************
 *inserts a message at the end of the message queue
 ************************************************************/
msgNode_t *queueMessageSprintf(msgNode_t *root, messageType type, const char *format, ...)
{

	char *message;
	va_list arglist;
	va_start(arglist, format );
	vasprintf(&message, format, arglist );
	va_end( arglist );
	root = queueMessage(root,message,type);
	free(message);
	return root;
}

/************************************************************
 *inserts a message at the front of the message queue
 ************************************************************/
msgNode_t *pushMessageSprintf(msgNode_t *root, messageType type, const char *format, ...)
{

	char *message;
	va_list arglist;
	va_start(arglist, format );
	vasprintf(&message, format, arglist );
	va_end( arglist );
	root = pushMessage(root,message,type);
	free(message);
	return root;
}

msgNode_t *msgNodeTrash(msgNode_t *root){
   if(root!=NULL){
      root->next = msgNodeTrash(root->next);
			free(root->message);
			free(root->next);
			free(root);
      root = NULL;
   }
   return root;
}

char *getMessageString(msgNode_t *root)
{
	int length = messageLength(root);
	//printf("message length: %d\n",length);
	char *messageString = (char *)calloc(length+1,sizeof(char));
	if(messageString == NULL) printf("problem creating memory for messages\n");
	messageString[0] = '\0';
	catMessages(root, messageString);
	//printf("%s\n",messageString);
	return messageString;
}

static int messageLength(msgNode_t *root)
{
	int length; 
	if(root == NULL) return 0;
	length = messageLength(root->next);
	if(root->type & messageMask ){
		length += root->length + 1 ; //add 1 for newline or null termination
	}
	return length;
}

static void catMessages(msgNode_t *root, char *messageString){
	if(root == NULL) return;
	if(root->type & messageMask){
		strcat(messageString,root->message);
		strcat(messageString,"\n");
	}
	catMessages(root->next,messageString);
	return;
}
