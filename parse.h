#ifndef	_PARSE_H
#define	_PARSE_H	1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>
//#include <ncurses/curses.h>
//#include <ncurses/term.h>
#include <curses.h>
#include <term.h>
#include "cnode.h"
#include "vnode.h"
#include "funcNode.h"
#include "msgNode.h"

#if !defined (DEBUG)
#define DEBUG 0
#endif

typedef struct parser_s {
	char *thisFuncName;
	funcNode_t *functions; 
	vnode_t *variables;//points to memory holding the value.  Can also point to a variable's value.
	msgNode_t *messages;
} parser_t;

bool debug;
//parser_t *calcMemory;

parser_t *calcMemoryFree(parser_t *calcMemory);
parser_t *memoryInit(void);
parser_t *copyMemory(parser_t *original);
int parseEquation (const char *eqn, char **answerString, parser_t *calcMemory);
char chomp( char *eqn);
void stringCopy(char *eqn, char *newStart);
char *stripwhite(char *eqn);
bool evaluateExpression(char *eqn, parser_t *calcMemory, double *answer);
bool parseFunction(char *eqn, parser_t *calcMemory, double *number);
bool parseOpenParen(char *eqn);
bool parseCloseParen(char *eqn);
char *parseEqn( char *eqn, char *valid_chars );
bool parseNumber( char *eqn, double *number );
bool parseVariable( char *eqn, parser_t *calcMemory, double *number );
double decodeEngineeringNotation( char *eqn );
bool isOp( char token );
double parenFunction(double number);
double freq(double number);
double db(double number);
double undb(double number);
double sgn(double number); //returns -1 if the number is < 0, 1 otherwise
void help(void);
void prependANS(char **eqn);
char *detectEqualsSign(char *eqn);
bool printInEngineeringNotation(double number, char **answerString);

#endif
