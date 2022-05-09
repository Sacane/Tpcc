/* tree.c */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
extern int lineno;       /* from lexer */

static const char *StringFromLabel[] = {
  "Prog",
  "DeclVars",
  "DeclFoncts",
  "types",
  "divstar",
  "id",
  "DeclFonct",
  "EnTeteFonct",
  "Corps",
  "Void",
  "Parametres",
  "ListTypVar",
  "SuiteInstr",
  "Exp",
  "Variable",
  "Assign",
  "Body",
  "Instr",
  "If",
  "Else",
  "While",
  "Or",
  "And",
  "Eq",
  "Plus",
  "Minus",
  "Order",
  "AddSub",
  "FunctionCall",
  "Neg",
  "Int",
  "Character",
  "Arguments",
  "ListExp",
  "Return",
  "EmptyInstr",
  "Switch",
  "Case",
  "Default",
  "Break",
  "Putchar",
  "Putint"
  /* list all other node labels, if any */
  /* The list must coincide with the label_t enum in tree.h */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
};

Node *makeNode(label_t label) {
  Node *node = malloc(sizeof(Node));
  if (!node) {
    printf("Run out of memory\n");
    exit(1);
  }
  node->label = label;
  node-> firstChild = node->nextSibling = NULL;
  node->lineno=lineno;
  return node;
}

void addSibling(Node *node, Node *sibling) {
  Node *curr = node;
  while (curr->nextSibling != NULL) {
    curr = curr->nextSibling;
  }
  curr->nextSibling = sibling;
}

void addChild(Node *parent, Node *child) {
  if (parent->firstChild == NULL) {
    parent->firstChild = child;
  }
  else {
    addSibling(parent->firstChild, child);
  }
}

void deleteTree(Node *node) {
  if (node->firstChild) {
    deleteTree(node->firstChild);
  }
  if (node->nextSibling) {
    deleteTree(node->nextSibling);
  }
  free(node);
}



void printTree(Node *node) {
  static bool rightmost[128]; // tells if node is rightmost sibling
  static int depth = 0;       // depth of current node
  for (int i = 1; i < depth; i++) { // 2502 = vertical line
    printf(rightmost[i] ? "    " : "\u2502   ");
  }
  if (depth > 0) { // 2514 = L form; 2500 = horizontal line; 251c = vertical line and right horiz 
    printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
  }
  printf("%s", StringFromLabel[node->label]);

  switch(node->label){
    case id:
    case types:
    case Variable:
    case FunctionCall:
      printf(" : %s", node->u.ident);
      break;
    case Int:
      printf(" : %d", node->u.num);
      break;
    case Addsub:
    case divstar:
    case Character: 
      if(node->u.byte == '\n'){
        printf(" : \\n");
      } else {
        printf(" : %c", node->u.byte);
      }
      
      break;
    case Order:
    case Eq:
      printf(": %s", node->u.comp);
      break;
    default:
      break;
  }
  printf("\n");
  depth++;
  for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
    rightmost[depth] = (child->nextSibling) ? false : true;
    printTree(child);
  }
  depth--;
}




char *getFuncNameFromDecl(Node *declFonctRoot){
  Node *id = declFonctRoot->firstChild->firstChild->firstChild;
  return id->u.ident;
}