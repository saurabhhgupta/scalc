#include "parse.h"

parser_t *calcMemoryFree(parser_t *calcMemory)
{
	calcMemory->functions = funcNodeTrash(calcMemory->functions);
	calcMemory->variables = trashVariables(calcMemory->variables);
	calcMemory->messages  = msgNodeTrash(calcMemory->messages);
	free(calcMemory);
	calcMemory = NULL;
	return calcMemory;
}

parser_t *memoryInit(void){
	parser_t *calcMemory = (parser_t *)malloc(sizeof(parser_t));
	char *answerString = NULL;

	setMessageMask(ANSWER);
	setMessageMask(ERROR);
	//setMessageMask(EQUATION);
	//setMessageMask(VARIABLE);
	/*
		 ANSWER    = 0, 
		 ERROR     = 1, 
		 EQUATION  = 2, 
		 VARIABLE  = 4,
		 STEP      = 8,
	 */

	calcMemory->functions = NULL;
	calcMemory->variables = NULL;
	calcMemory->messages  = NULL;
	calcMemory->thisFuncName  = NULL;

	calcMemory->functions = pushNode(calcMemory->functions,"abs(",(&fabs));
	calcMemory->functions = pushNode(calcMemory->functions,"acos(",(&acos));
	calcMemory->functions = pushNode(calcMemory->functions,"asin(",(&asin));
	calcMemory->functions = pushNode(calcMemory->functions,"atan(",(&atan));
	calcMemory->functions = pushNode(calcMemory->functions,"ceil(",(&ceil));
	calcMemory->functions = pushNode(calcMemory->functions,"cos(",(&cos));
	calcMemory->functions = pushNode(calcMemory->functions,"cosh(",(&cosh));
	calcMemory->functions = pushNode(calcMemory->functions,"exp(",(&exp));
	calcMemory->functions = pushNode(calcMemory->functions,"fabs(",(&fabs));
	calcMemory->functions = pushNode(calcMemory->functions,"floor(",(&floor));
	calcMemory->functions = pushNode(calcMemory->functions,"ln(",(&log));
	calcMemory->functions = pushNode(calcMemory->functions,"log(",(&log10));
	calcMemory->functions = pushNode(calcMemory->functions,"sin(",(&sin));
	calcMemory->functions = pushNode(calcMemory->functions,"sinh(",(&sinh));
	calcMemory->functions = pushNode(calcMemory->functions,"sqrt(",(&sqrt));
	calcMemory->functions = pushNode(calcMemory->functions,"tan(",(&tan));
	calcMemory->functions = pushNode(calcMemory->functions,"tanh(",(&tanh));
	calcMemory->functions = pushNode(calcMemory->functions,"freq(",(&freq));
	calcMemory->functions = pushNode(calcMemory->functions,"db(",(&db));
	calcMemory->functions = pushNode(calcMemory->functions,"undb(",(&undb));
	calcMemory->functions = pushNode(calcMemory->functions,"sgn(",(&sgn));
	calcMemory->functions = pushNode(calcMemory->functions,"(",(&parenFunction));
	//printList(funcs);

	calcMemory->variables = addConstant(calcMemory->variables,"pi",(double)M_PI);
	calcMemory->variables = addConstant(calcMemory->variables,"e",(double)M_E);

	/************************************************************
	 *setup vt = k*T/q
	 ************************************************************/
	calcMemory->variables = addConstant(calcMemory->variables,"k",(double)1.3806503e-23);
	calcMemory->variables = addConstant(calcMemory->variables,"q",(double)1.60217646e-19);
	calcMemory->variables = addConstant(calcMemory->variables,"C",(double)25);
	parseEquation("T=273.15+C",&answerString,calcMemory);
	free(answerString); answerString = NULL;

	parseEquation("vt=k*T/q",&answerString,calcMemory);
	free(answerString); answerString = NULL;

	calcMemory->variables = addConstant(calcMemory->variables,"n",(double)0.8);
	parseEquation("dv=vt*ln(n)",&answerString,calcMemory);
	free(answerString); answerString = NULL;
	/************************************************************/

	/************************************************************
	 *setup amortization formula vt = k*T/q
	 ************************************************************/
	calcMemory->variables = addConstant(calcMemory->variables,"APR",(double) 2.5);
	parseEquation("mpr=APR/1200",&answerString,calcMemory);
	free(answerString); answerString = NULL;

	calcMemory->variables = addConstant(calcMemory->variables,"term",(double) 3);
	parseEquation("n_pmts=term*12",&answerString,calcMemory);
	free(answerString); answerString = NULL;

	calcMemory->variables = addConstant(calcMemory->variables,"down",(double)5000);
	calcMemory->variables = addConstant(calcMemory->variables,"principal",(double)30000);
	parseEquation("pmt=(principal-down)*mpr*(1+mpr)^n_pmts/((1+mpr)^n_pmts-1)",
			&answerString,calcMemory);
	free(answerString); answerString = NULL;

	parseEquation("total=pmt*n_pmts",&answerString,calcMemory);
	free(answerString); answerString = NULL;

	parseEquation("cost=principal+down-total",&answerString,calcMemory);
	free(answerString); answerString = NULL;

	//printf("(pointer)calcMemory %p\n",calcMemory);
	return calcMemory;
}

parser_t *copyMemory(parser_t *original){
	parser_t *newmem = memoryInit();
	newmem->variables = copyVarsRecurse(original->variables, newmem->variables);
	//printf("(pointer)newmem %p\n",newmem);
	return newmem;
}

/*
 * TODO: 
 * if expression is in the variable list, print its answer and all of its links
 *
 * make it so that it is easy to see a variable's dependencies and dependents.
 *
 * make a way to remove variables
 *
 * if an equation has an undefined variable, make it and set it to zero so that everything
 * will recalculate when the variable is finally defined.
 *
 * vnode.c needs the ability to add messagesNodes.
 */
int parseEquation (const char *expression, char **answerString, parser_t *calcMemory){
	int retval = 0;
	char *eqn;
	char *eqn_save;
	char *variable;
	char *calcResult;
	vnode_t *recalcList;
	char *recalcEquation, *recalcName;
	double answer;
	//char *vhead;

	//printf("(pointer)calcMemory %p\n",calcMemory);
	eqn = (char *)calloc(strlen(expression)+1,sizeof(char));
	eqn[0] = '\0';
	strcpy(eqn,expression);

	//look for equals sign, return the variable from the left side of the '='
	if(DEBUG) perror("detect equals");
	variable = detectEqualsSign(eqn);
	if(DEBUG) perror("A");
	calcMemory->thisFuncName = NULL;
	if(DEBUG) perror("B");
	calcMemory->thisFuncName = variable;
	if(DEBUG) perror("C equals");

	/************************************************************
	 * see if equation starts with operator.
	 * this implies that the first number in the equation was the last answer.
	 * add 'ans' to the front of the equation string
	 * this has a problem distinguishing a negative number at the beginning of
	 * an expression.  does it mean ans-4 or just -4?
	 ************************************************************/
	if(DEBUG) perror("prepend ANS");
	prependANS(&eqn);

	//copy the equation in case it needs to be saved as in a variable struct.
	eqn_save = (char *)calloc(strlen(eqn)+1,sizeof(char));
	eqn_save[0] = '\0';
	strcpy(eqn_save,eqn);

	//if eqn is exactly the same as a variable then show
	//all of the info stored on it.
	calcMemory->messages = reportVariableData(calcMemory->variables,eqn,calcMemory->messages);

	//this is where the equation is ripped apart and crunched.
	if(DEBUG) perror("evaluate expression");
	if(evaluateExpression(eqn,calcMemory,&answer)){ 
		printInEngineeringNotation(answer,&calcResult);
		calcMemory->messages = queueMessage(calcMemory->messages,calcResult,ANSWER);
		free(calcResult);
		retval = 1;
	}

	if(DEBUG) perror("make variables");

	//always store the last variable as 'ans';
	calcMemory->variables = addConstant(calcMemory->variables,"ans",answer);

	//save result into a variable if needed.
	if(variable != NULL){
		calcMemory->thisFuncName = NULL;
		//this is the spot to re-calculate everything after a variable has been changed.
		if(retval || 0){
			calcMemory->variables = addVariable(calcMemory->variables,variable,eqn_save);
			setVariableValue(searchVariables(calcMemory->variables, variable),answer);
			recalcList = getRecalcList(searchVariables(calcMemory->variables, variable));
			while(getRecalcStrings(&recalcEquation,&recalcName)){
				//printf("\n%s",recalcEquation);
				if(evaluateExpression(recalcEquation,calcMemory,&answer)){ 
					printInEngineeringNotation(answer,&calcResult);
					calcMemory->messages = pushMessageSprintf(calcMemory->messages,ERROR,
							"%s = %s",recalcName,calcResult);
					//store the recalc result in the variable's value
					setVariableValue(searchVariables(calcMemory->variables, recalcName),answer);

					free(calcResult);
				}
			}
			trashVariables(recalcList);
		}
		//cleanup the variable memory
		free(variable);
		variable =  NULL;
		calcMemory->thisFuncName = NULL;
	}
	cleanAccessMarks(calcMemory->variables);

	//test the variable length counter
	if(0){ //probably dont need this block anymore.  delete it when it gets annoying
		char *name = getVariableName(variableAtIndex(calcMemory->variables,2,0));
		calcMemory->messages = queueMessageSprintf(calcMemory->messages,ERROR,
				"var list Length = %d",variableListLength(calcMemory->variables));

		//the string returned by getVariableName is not getting freed.
		calcMemory->messages = queueMessageSprintf(calcMemory->messages,ERROR,
				"var @ pos 2 = %s", name);
	}

	*answerString = getMessageString(calcMemory->messages);
	//printf("*** MESSAGES: ***\n%s*****************\n",*answerString);
	//printf("\n%s",*answerString);

	if(DEBUG) perror("free memory");
	msgNodeTrash(calcMemory->messages);
	calcMemory->messages = NULL;
	free(eqn);
	eqn = NULL;
	free(eqn_save);
	eqn_save = NULL;
	return retval;
}

bool evaluateExpression(char *eqn, parser_t *calcMemory, double *answer){
	cnode_t *root=NULL;
	double number;
	char operator;
	bool numberParsed = false;

	//parse equation into numbers and operators
	while(1){
		if(DEBUG || debug) printf("evaluateExpression: %s\n",eqn);

		//look for either a function,paren or a number.
		numberParsed = false;
		numberParsed = parseFunction(eqn, calcMemory, &number);
		if(!numberParsed) numberParsed = parseNumber(eqn, &number);
		if(!numberParsed) numberParsed = parseVariable(eqn,calcMemory, &number);
		if(!numberParsed){
			//if here then there is trouble parsing a number.
			//report the error, dump the tree and return a tree
			//holding a zero at its root.
			//printf("SYNTAX ERROR\n");
			//calcMemory->messages = queueMessage(calcMemory->messages,"SYNTAX ERROR",ERROR);
			root = cnodeTrash(root);
			*answer=0;
			return false;
		}
		//printf("\n%g\t%s\n",*number,eqn);

		if(DEBUG || debug) printf("evaluateExpression: insertNumber\n");
		root = insertNumber(root,number);
		if(DEBUG || debug) printf("evaluateExpression: insertNumber done\n");

		//the equation should only end after a number
		//or close paren.
		if(strlen(eqn) == 0 || parseCloseParen(eqn)){
			if(DEBUG || debug) printf("evaluateExpression: calcTree\n");
			calcTree(root); 
			*answer = getCnodeNumber(root);
			cnodeTrash(root);
			if(DEBUG || debug) printf("evaluateExpression: done\n");
			return true; 
		}

		//printf("equation ");printTree(root);printf("\n");
		//look for an operator.
		if(isOp(*eqn)){
			//printf("evaluateExpression: operator->%c\n",*eqn);
			//if the next operator is a minus sign, set the operator to '+' and multiply the next thing on the 
			//string by -1.  This forces minus signs to be interpreted as adding a negative number
			//other wise minus signs create implicit parenthesis so that:
			// 8-5+4 = -1 = 8-(5+4) 
			// instead of 8-5+4 = 7
			operator=chomp(eqn);
			if(operator == '-'){
				root = insertOp(root,'+');
				root = insertNumber(root,-1.0);
				operator = '*';
			}
		}else{
			//if here then there is trouble parsing an operator.
			//report the error, dump the tree and return a tree
			//holding a zero at its root.
			//printf("UNKNOWN OPERATOR '%c'\t%d\n",*operator,(char)*operator);
			calcMemory->messages = queueMessageSprintf(calcMemory->messages,ERROR,
					"UNKNOWN OPERATOR '%c'\t%d",operator,operator);
			root = cnodeTrash(root);
			*answer=0;
			return false;
		}
		if(DEBUG || debug) printf("evaluateExpression: operator->%c\n",operator);
		root = insertOp(root,operator);
	}
}

bool parseFunction(char *eqn, parser_t *calcMemory, double *number){

	funcNode_t *func;

	func = getFunc(eqn,calcMemory->functions);
	if(func == NULL) return(false);
	stringCopy(eqn,eqn+strlen(func->name));
	if(evaluateExpression(eqn,calcMemory,number)){
		*number = func->func_p (*number);

		calcMemory->messages = queueMessageSprintf(calcMemory->messages,STEP,
				"%s%g) = %g",func->name,*number,*number);

		if(debug || DEBUG) printf("%s%g) = %g\n",func->name,*number,*number);
		return true;
	}
	*number = 0; 
	return false;
}

//what happens when *eqn is empty?
char chomp( char *eqn){
	char op;
	if(eqn == NULL) return('\0');
	op = *eqn; stringCopy(eqn,eqn+1);
	return op;
}

//string strcpy gets messed up when the source and destination are
//the same.  This works around that problem.
void stringCopy(char *eqn, char *newStart){
	int eqn_length=0;
	char *tmpStr=NULL;

	if(DEBUG || debug) printf("\tstringCopy: eqn = %s\n",eqn);
	eqn_length = strlen(newStart);
	tmpStr = (char *)calloc(eqn_length+1,sizeof(char));
	tmpStr[0] = '\0';
	strcpy(tmpStr,newStart);
	if(DEBUG || debug) printf("\tstringCopy: tmpString = %s\n",tmpStr);
	strcpy(eqn,tmpStr);
	free(tmpStr);
	if(DEBUG || debug) printf("\tstringCopy: eqn = %s\n",eqn);
}

bool parseCloseParen(char *eqn){
	if(*eqn != ')') return false;
	chomp(eqn);
	return true;
}

/***
 *grab tokens in front of operators and parenthesis.  
 *see if the token is in the variable list.
 *if so then decode the variable.
 */
bool parseVariable( char *eqn, parser_t *calcMemory, double *number ){
	size_t var_length; size_t eqn_length; 
	char *variable; 
	vnode_t *var_node;
	char *var_equation=NULL;
	bool evalSuccess;

	var_length = strcspn(eqn,"^*/+-\\|()%<>");
	eqn_length = strlen(eqn);
	//printf("eqn: %s\nvar_length: %d\neqn_length: %d\n",eqn,var_length,eqn_length);
	if(var_length > 0 && var_length <= eqn_length){
		variable = (char *)calloc(eqn_length+1,sizeof(char));
		variable[0] = '\0';
		strncpy(variable,eqn,var_length);
		stringCopy(eqn,eqn+var_length);

		var_node = searchVariables(calcMemory->variables,variable);

		if(var_node == NULL){
			calcMemory->messages = queueMessageSprintf(calcMemory->messages,ERROR,
					"undefined variable: '%s'",variable);
			//printf("\nundefined variable: '%s'\n",variable);
			free(variable);
			return(false);
		}

		if(calcMemory->thisFuncName != NULL){
			if(!strcmp(calcMemory->thisFuncName,var_node->name)){
				calcMemory->messages = queueMessageSprintf(calcMemory->messages,ERROR,
						"Circular Reference: %s",variable,var_node->value);
				free(variable);
				return false;
			}
		}

		var_equation = getVariableEquation(var_node);
		if(var_equation == NULL){
			if(DEBUG || debug) printf("parseVariable: constant: %s\n",variable);
			calcMemory->messages = queueMessageSprintf(calcMemory->messages,VARIABLE,
					"%s=%g",variable,var_node->value);
			//TODO: Set the access mark here.
			//printf("%s=%g\n",variable,var_node->value);
			*number = var_node->value;
			free(variable);
			return(true);
		}else{
			if(DEBUG || debug) printf("parseVariable: variable: %s\n",variable);
			calcMemory->messages = queueMessageSprintf(calcMemory->messages,EQUATION,
					"%s=%s",variable,var_equation);
			//printf("%s=%s\n",variable,var_equation);
			evalSuccess = evaluateExpression(var_equation,calcMemory,number); 
			setVariableValue(var_node,*number);
			free(var_equation);
			free(variable);
			return evalSuccess;
		}
		//if(DEBUG || debug) 
		printf("parseVariable: fall through on variable: %s\n",variable);
	}
	return false;
}

bool parseNumber( char *eqn, double *value ){
	double number; char *endptr;
	number = strtod(eqn,&endptr);
	if(eqn == endptr) return false; 
	//stringCopy(eqn,endptr);
	if(DEBUG || debug) printf("parseNumber: eqn-> %s\n",eqn);
	stringCopy(eqn,endptr);
	if(DEBUG || debug) printf("parseNumber: eqn-> %s\n",eqn);

	*value = number;
	if(DEBUG || debug) printf("parseNumber: %f -> %f\n",*value,number);
	
	//decode engineering notation.
	*value *= decodeEngineeringNotation(eqn);
	if(DEBUG || debug) printf("parseNumber: done\n");

	return true;
}

//if the first character in the equation looks like an
//engineering notation character the character is stripped
//from the front of the equation and the correct multiplier is 
//returned.  This is assuming that the number has already been taken
//off the equation and only the units indicator is left.
double decodeEngineeringNotation( char *eqn ) {
	double multiplier = 1;
	double power = 0;

	if(strncmp(eqn,"meg",3)){
		switch(*eqn){
			case 'f': power = -15; break;
			case 'p': power = -12; break;
			case 'A': power = -10; break;
			case 'n': power = -9;  break;
			case 'u': power = -6;  break;
			case 'm': power = -3;  break;
			case '%': power = -2;  break;
			case 'k': power = 3;   break;
			case 'M': power = 6;   break;
			case 'G': power = 9;   break;
			case 'T': power = 12;  break;
			case 'P': power = 15;  break;
			default:
								power = 0;
		}
		if(power != 0){
			multiplier = pow(10.0,power);
			stringCopy(eqn,eqn+1);
		}

		return multiplier;
	}else{
		multiplier = pow(10.0,6);
		stringCopy(eqn,eqn+3);
		return multiplier;
	}

}

//test wether the token is an operator.
bool isOp( char token ) {
	bool answer = true;
	if(precedence( token ) < 1 ){answer=false;}
	//switch(*token){
	//   case '+':
	//   case '-':
	//   case '|':
	//   case '*':
	//   case '/':
	//   case '%':
	//   case '^':
	//      //case '%':
	//      break;
	//   default:
	//      answer=false;
	//}
	return answer;
}

//strip white space from the equation.
//destroys eqn
char *stripwhite(char *eqn){
	char *token;
	char *equation = (char *)calloc(strlen(eqn)+1,sizeof(char));
	equation[0] = '\0'; //initalize the string with the terminating null

	token = strtok(eqn," \n");
	while(token != NULL){
		if(DEBUG || debug) printf("stripwhite: token = %s\n",token);
		strcat(equation,token);
		token = strtok(NULL," \n");
	}
	if(DEBUG || debug) printf("stripwhite: %s\n",equation);
	return equation;
}

double parenFunction(double number){
	return number;
}

//returns 1/2*pi*number;  useful for calculating RC frequency
//or R given C and frequency or C given R and frequency
double freq(double number){
	double freq = 1/(2*M_PI*number);
	return freq;
}

//returns 20*log10(number);
double db(double number){
	double answer = 20*log10(number);
	return answer;
}

double undb(double number){
	double answer = pow(10,number/20);
	return answer;
}

double sgn(double number){
	double answer = number < 0 ? -1.0 : 1.0;
	return answer;
}

void help( void ){
	printf("type pf to print function list\n");
	printf("type pv to print variable list\n\n");
	printf("type showsteps to show calculation steps\n\n");
	printf("type debug to show parsing steps\n\n");
	printf("special symbols:\n");
	printf("\\: resistor divider\n");
	printf("\t Vout = Vin  * R1\\R2\n");
	printf("\t Vin  = Vout / R1\\R2\n");
	printf("|: parallel resistors\n\n");
	printf("precedence:\n");
	printf("%c %c %c %c %c %c %c %c %c %c %c\n\n" ,'\\' ,'|' ,'^' ,'x' ,'*' ,'/' ,'%' ,'+' ,'-' ,'<', '>');  
}

//see if equation starts with operator.
//this implies that the first number in the equation was the last answer.
//add 'ans' to the front of the equation string
//dont prepend ans if the eqn starts with '-'
void prependANS(char **eqn){
	if(isOp(**eqn) && !(**eqn == '-')){
		int eq;
		char *eq2;
		eq=5+strlen(*eqn); //space for eqn plus 'ans=' and '\0'
		eq2 = (char *)calloc(eq,sizeof(char)); eq2[0] = '\0';
		strcpy(eq2,"ans");
		strcat(eq2,*eqn);
		free(*eqn);
		*eqn=eq2;
	}
}

char *detectEqualsSign(char *eqn){
	int eq_index;
	char *variable = NULL;
	eq_index = strcspn(eqn,"=");
	if(strlen(eqn) > eq_index){
		variable = (char *)calloc(eq_index+4,sizeof(char));
		strncpy(variable,eqn,eq_index);
		stringCopy(eqn,eqn+eq_index+1);
		//printf("\n%d\t%s\n",eq_index,variable);
	}
	return variable;
}

bool printInEngineeringNotation( double number, char **answerString){
	double multiplier = 1;
	int power = floor(log10(sqrt(number*number)));
	char suffix = ' ';

	//printf("%g\t%d\n",number,power);
	switch(power){
		case -15: case -14: case -13: multiplier = 1e15; suffix='f'; break;
		case -12: case -11: case -10: multiplier = 1e12; suffix='p'; break;
		case  -9: case -8: case -7: multiplier = 1e9; suffix='n'; break;
		case  -6: case -5: case -4: multiplier = 1e6; suffix='u'; break;
		case  -3: case -2: case -1: multiplier = 1e3; suffix='m'; break;

		case  0 : case 1 : case 2 : multiplier = 1;     suffix=' '; break;
		case  3 : case 4 : case 5 : multiplier = 1e-3;  suffix='k'; break;
		case  6 : case 7 : case 8 : multiplier = 1e-6;  suffix='M'; break;
		case  9 : case 10: case 11: multiplier = 1e-9;  suffix='G'; break;
		case  12: case 13: case 14: multiplier = 1e-15; suffix='T'; break;
	}
	number *= multiplier;
	asprintf(answerString,"%.9g%c",number,suffix);
	return true;
}
