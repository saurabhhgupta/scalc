#include "cnode.h"
bool show_steps = false;

cnode_t *newCnode( char op, double number, cnodeType nodeType)
{
    cnode_t *new;
    new = (cnode_t *)malloc(sizeof(cnode_t));
		if(nodeType == CNODE_NUMBER){
			setCnodeNumber(new,number);
		}else if(nodeType == CNODE_OPERATOR){
			setCnodeOperator(new,op);
		}else{
			perror("Fatal error creating cnode\n");
			exit(1);
		}
    new->left = NULL;
    new->right = NULL;
    return new;
}

void setCnodeOperator(cnode_t *cnode, char op){
		cnode->nodeType = CNODE_OPERATOR;
		cnode->op = op;
		cnode->number = 0;
}

void setCnodeNumber(cnode_t *cnode, double number){
		cnode->nodeType = CNODE_NUMBER;
		cnode->op = '\0';
		cnode->number = number;
}

char getCnodeOperator(cnode_t *cnode)
{
	return cnode->op;
}

double getCnodeNumber(cnode_t *cnode)
{
	return cnode->number;
}

cnode_t *insertNumber(cnode_t *root, double number){
    if(root == NULL){
        root = newCnode(0,number,CNODE_NUMBER);
    }else if(isOpNode(root)){
        root->right = insertNumber( root->right, number );
    }
    return root;
}

//insert an operator in the tree.
cnode_t *insertOp(cnode_t *root, char op){
    cnode_t *newNode;
    //on an op node with lower precedence.
    if(isOpNode(root)){
        if(precedence(root->op) < precedence(op)){
            root->right = insertOp( root->right, op);
            return root;
        }
    }
    newNode = newCnode(op,0,CNODE_OPERATOR);
    newNode->left = root;
    return newNode;
}

bool isOpNode( cnode_t *root ){
    if(root->nodeType == CNODE_OPERATOR) return true;
    return false;
}

bool isNumberNode( cnode_t *root ){
    if(root->nodeType == CNODE_NUMBER) return true;
    return false;
}

/*
void printTree(cnode_t *root){
    if(root == NULL) return;
    printTree(root->left);
    if(root->value != NULL) printf("%g",*(root->value));
    if(root->op != NULL) printf("%c",*(root->op));
    printTree(root->right);
    return;
}
*/

double calcTree(cnode_t *root){
    double r_num, l_num, answer;
		if(root == NULL) return (0);
    if(isNumberNode(root))return(getCnodeNumber(root));

    l_num = calcTree(root->left);
    r_num = calcTree(root->right);
    switch(root->op){
       case '\\':
          answer = (r_num) / (l_num + r_num);
          if(show_steps) printf("%g \\ %g = %g / (%g + %g) = %g\n",l_num,r_num,r_num,l_num,r_num,answer);
          break;
       case '|':
          answer = (l_num * r_num) / (l_num + r_num);
					if(show_steps) printf("%g | %g = %g\n",l_num,r_num,answer);
          break;
       case '-':
          answer = l_num - r_num;
          if(show_steps) printf("%g - %g = %g\n",l_num,r_num,answer);
          break;
       case '+':
          answer = l_num + r_num;
          if(show_steps) printf("%g + %g = %g\n",l_num,r_num,answer);
          break;
       case '%':
          answer = fmod(l_num, r_num);
          if(show_steps) printf("%g %% %g = %g\n",l_num,r_num,answer);
          break;
       case '/':
          answer = l_num / r_num;
          if(show_steps) printf("%g / %g = %g\n",l_num,r_num,answer);
          break;
       case '*':
       case 'x':
          answer = l_num * r_num;
          if(show_steps) printf("%g * %g = %g\n",l_num,r_num,answer);
          break;
       case '^':
          answer = pow(l_num,r_num);
          if(show_steps) printf("%g ^ %g = %g\n",l_num,r_num,answer);
          break;
			case '<':
          answer = l_num < r_num ? 1 : 0;
          if(show_steps) printf("%g < %g = %g\n",l_num,r_num,answer);
          break;
			case '>':
          answer = l_num > r_num ? 1 : 0;
          if(show_steps) printf("%g > %g = %g\n",l_num,r_num,answer);
          break;

       default:
          answer = 0;
          printf("calcTree: unknown operator");
          exit(1);
          break;
    }
    setCnodeNumber(root,answer);
    return answer;
}    

cnode_t *cnodeTrash(cnode_t *root){
   if(root!=NULL){
      root->left = cnodeTrash(root->left);
      root->right = cnodeTrash(root->right);
      free(root);
      root = NULL;
   }
   return root;
}


int precedence(char op){
   switch(op){
      case '>':  return 1;
      case '<':  return 2;
      case '-':  return 3;
      case '+':  return 4;
      //case '%':  return 5; //used as percent symbol instead
      case '/':  return 6;
      case '*':  return 7;
      case '^':  return 9;
      case '|':  return 10;  //parallel symbol for resistor calcs.
      case '\\': return 11;  //resistor divider symbol
      default: return -1;
   }
   return 0;
}


