//funcNode.h
#if !defined (FUNCNODE_H_INCL)
#define FUNCNODE_H_INCL 1

#if !defined (BOOLEAN_DEFINED)
#define BOOLEAN_DEFINED
#define BOOLEAN char
#define TRUE 1
#define FALSE 0
#endif

typedef double (*fp) (double);
typedef struct funcNode_s {
    char *name;
    fp func_p;
    struct funcNode_s *next;
} funcNode_t;


extern funcNode_t *newNode( const char *name, fp func_p );
extern funcNode_t *pushNode(funcNode_t *root, char *name, fp func_p );
extern funcNode_t *funcNodeTrash(funcNode_t *root);
extern funcNode_t *getFunc(char *parse_p, funcNode_t *root);
extern void printList(funcNode_t *root);

#endif
