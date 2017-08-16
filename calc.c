
/*TODO:
 * open paren should imply multiplication
 *
 * store constants as tree nodes. DONE
 * then variables should be easy to add. DONE
 *
 * make a tree copy function so that equations can be saved
 * then saved equations can be referenced the same as saved 
 * constants or variables or functions. DONE SORT OF .
 *
 * Add support for list data?
 *
 * understand lines starting with an op imply ans as first arg. DONE
 *
 * add modulo. DONE
 * 
 * Make a help command. DONE
 * Add descriptions of what the functions do into the function data structures.
 *
 * Parse hex numbers? use a variation of the decodeEngineeringNotation function...
 * or just add a hex(function) or h2d() o2d() d2h() etc...
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>
//#include <ncurses/curses.h>
//#include <ncurses/term.h>
#include <curses.h>
#include <term.h>
#include <stdbool.h>
#include "msgNode.h"
#include "parse.h"

#if !defined (DEBUG)
#define DEBUG 0
#endif

#define OUTFILE "/.scalc"

bool debug = 0;

int writeFile(char *fileName, char *outstring, int length);
unsigned char *readFile(char *name, unsigned long *fileLen);
size_t SIZEOF_PARSER = sizeof(parser_t);

int main (int argc, char **argv) {
	char prompt[20];
	char *r;
	char *eqn; // moves along the equation string while the equation is being parsed.
	char *answerString=NULL;
	char *linesave;
	parser_t *calcMemory = memoryInit();

	char upbind[20] = "previous-history";
	char dnbind[20] = "next-history";
	rl_bind_key(30,(rl_command_func_t *)&upbind);
	rl_bind_key(31,(rl_command_func_t *)&dnbind);
	help();

	if(DEBUG || debug) printf("\ncompiled in DEBUG mode\n\n");

	strcpy(prompt,":");
	r = readline(prompt);
	eqn = stripwhite(r);
	free(r);

	while(strcmp(eqn,"quit") && strcmp(eqn,"exit") && strcmp(eqn,"Quit")){

		//do this to save the equation after the whitespace has been stripped
		linesave = (char *)calloc(strlen(eqn)+1,sizeof(char));
		linesave[0] = '\0'; strcpy(linesave,eqn);

		if(DEBUG || debug) printf("\neqn:     %s",eqn);
		if(DEBUG || debug) printf("\nhistory: %s\n",linesave);

		//add the last equation into the command line history
		if (linesave && *linesave && strlen(eqn) != 0) add_history (linesave);
		free(linesave);

		if(!strcmp(eqn,"pv")) {
			char *answer = printVariables(calcMemory->variables);
			printf("%s\n\n",answer);
			free(answer);
		} else if(!strcmp(eqn,"pf")) {
			printList(calcMemory->functions);
		} else if(!strcmp(eqn,"save")) {

			char pwdfile[1024];
			char *home = strdup(getenv("HOME"));
			strncat(pwdfile,home,1024);
			strncat(pwdfile,OUTFILE,1024-strlen(pwdfile));
			printf("\npwdFile is %s\n",pwdfile);
			free(home);

			char *answer = printVariables(calcMemory->variables);
			printf("%s\n\n",answer);

			writeFile(pwdfile,answer,strlen(answer));
			free(answer);

			//handle opening and printing to a file here? or in msgNode?
		} else if(!strcmp(eqn,"help")) {
			help();
		} else if(!strcmp(eqn,"debug")) {
			debug = debug ? false : true;
			if(debug) strcpy(prompt,"debug:");
			if(!debug) strcpy(prompt,":");
		} else if(!strcmp(eqn,"showsteps")) {
			show_steps = show_steps ? false : true;
			printf("show_steps = %d\n",show_steps);
		} else if(strlen(eqn) != 0) { //process the equation
			if(DEBUG || debug) printf("mainLoop: %s\n",eqn);
			parseEquation(eqn,&answerString,calcMemory);
			if(answerString != NULL){
				printf("%s\n\n",answerString);
				free(answerString);
				answerString=NULL;
			}
		}
		free(eqn);
		r = readline(prompt);
		eqn = stripwhite(r);
		if(DEBUG || debug) printf("mainLoopEnd: %s\n%s\n",r,eqn);
		free(r);
	}
	calcMemoryFree(calcMemory);
	free(eqn);
	return 0;
}

unsigned char *readFile(char *name, unsigned long *fileLen)
{
	FILE *file;
	unsigned char *buffer;

	//Open file
	file = fopen(name, "r");
	if (!file)
	{
		//fprintf(stderr, "Unable to open file %s\n", name);
		return NULL;
	}

	//Get file length
	fseek(file, 0, SEEK_END);
	*fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	buffer=(unsigned char *)malloc(*fileLen+1);
	memset(buffer,0,*fileLen+1);
	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
		fclose(file);
		return NULL;
	}

	//Read file contents into buffer
	fread(buffer, *fileLen, 1, file);
	fclose(file);

	return(buffer);
}

int writeFile(char *fileName, char *outstring, int length) 
{
	FILE* file;
	file=fopen(fileName,"w");
	fwrite(outstring, sizeof(char), length, file);  
	fclose(file);
	return(0);
}
