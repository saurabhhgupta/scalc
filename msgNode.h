/*
 *  msgNode.h
 *  Currency Converter
 *
 *  Created by Mike on 7/27/13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef	_MSGNODE_H
#define	_MSGNODE_H	1
typedef enum {
	ANSWER    = 1, 
	ERROR     = 2, 
	EQUATION  = 4, 
	VARIABLE  = 8,
	STEP      = 16,
	_msgDEBUG = 32
} messageType; 

// linked list that gathers messages as expressions are processed.
typedef struct msgNode_s {
	char *message; //errors, variable decodes, eqn decodes etc...
	int length; //strlen(message) 
	messageType type; //only messages less <= the level will be printed.
	struct msgNode_s *next;
} msgNode_t;

unsigned int setMessageMask(messageType type);
unsigned int unsetMessageMask(messageType type);
/************************************************************
 *inserts a message at the front or end of the message queue
 ************************************************************/
msgNode_t *queueMessage(msgNode_t *root, char *message, messageType type);
msgNode_t *queueMessageSprintf(msgNode_t *root, messageType type, const char *format, ...);
msgNode_t *pushMessage(msgNode_t *root, char *message, messageType type);
msgNode_t *pushMessageSprintf(msgNode_t *root, messageType type, const char *format, ...);
msgNode_t *msgNodeTrash(msgNode_t *root);
char *getMessageString(msgNode_t *root);

#endif
