/***************************************************************************/
/****************************** Includes ***********************************/
/***************************************************************************/
#include <stddef.h>
#include <stdio.h>

#include "common.h"
#include "interpreter.h"
#include "debug.h"
//#include "syntax_parser.h"
//
#define PRINT_PAD(level,dbg,...) { int i = level; \
  for (; i > 1; DEBUG_NONE("| "), i--); DEBUG_NONE("+"); \
  DEBUG(dbg, __VA_ARGS__);} 

static node_t *exec_expression(node_t *root,int level);
static void int_print(node_t *root);
void do_statements(node_t *node, int leve);
void do_statement(node_t *node, int level); 
void exec_print(node_t *root, int level)
{
  node_t *exp;
  for ( exp = root; exp->right != NULL; exp = exp->left) { 
    node_t *res = exec_expression(exp->right,level);   
    PRINT_PAD(level,DEBUG_PROD,"%f\n", res->value.f);
  }
}

node_t *exec_expression(node_t *root,int level) {
  //calculate(root); 
  if (root != NULL) {
    PRINT_PAD(level,DEBUG_TRACE,"node is < %s >\n", root->text);
    exec_expression(root->left,level+1);
    exec_expression(root->right,level+1);
    //PRINT_PAD(level,DEBUG_TRACE,"node is < %s >\n", root->text);
  }
  
  calculate(root);
  return root;
}

/*
void exec_while(node_t *root, int level) {
  printf("in while\n");
  int cond;
  //printf("cond = %d\n", cond = (int)(exec_expression(root->left)->value.f));
  do_statement(root->left, level + 1);
  do_statement(root->right, level + 1);
}
*/

void int_print(node_t *root) {

}
void do_statement(node_t *node, int level); 

void exec_while(node_t *node, int level)
{
  PRINT_PAD(level,DEBUG_TRACE,"begin while\n");
  //PRINT_PAD(level,DEBUG_TRACE,"check condition\n");
  while((int)exec_expression(node->left->right, level)->value.f != 0){ 
    do_statements(node->right, level + 1);
  }
  assert(node->right != NULL);
  PRINT_PAD(level,DEBUG_TRACE,"end while\n");
}

void exec_if(node_t *node, int level)
{
  //exec_while(node, level);
  PRINT_PAD(level,DEBUG_TRACE,"begin if\n");
  //printf("here\n");
  //PRINT_PAD(level,DEBUG_TRACE,"check condition\n");
  //do_statements(node->left, level + 1);

  if ((int)exec_expression(node->left->right, level)->value.f != 0) {
    do_statements(node->right, level + 1);
  }
  assert(node->right != NULL);
  PRINT_PAD(level,DEBUG_TRACE,"end if\n");
  //do_statements(node->right, level + 1); 

}

void exec_ifelse(node_t *node, int level)
{
  //exec_while(node, level);
  PRINT_PAD(level,DEBUG_TRACE,"begin ifelse\n");
  //PRINT_PAD(level,DEBUG_TRACE,"check condition\n");
  //do_statements(node->left, level + 1);

  if ((int)exec_expression(node->left->right, level)->value.f != 0) {
    PRINT_PAD(level,DEBUG_TRACE,"this is left branch\n");
    do_statements(node->right->left, level + 1);
  }
  else {
    PRINT_PAD(level,DEBUG_TRACE,"this is right branch\n");
    do_statements(node->right->right, level + 1);
  }
  assert(node->right != NULL);
  PRINT_PAD(level,DEBUG_TRACE,"end ifelse\n");
  //do_statements(node->right, level + 1); 

}

void do_statements(node_t *node, int level) {
  //printf("in function %s\n", __FUNCTION__);
  if (node != NULL) {
  //for (node= node_; node != NULL && node != NULL; node = node->left)
  //{
    PRINT_PAD(level,DEBUG_TRACE,"begin\n");
    if (node != NULL) {
      //assert(node->text != NULL);
      int i = level;
      PRINT_PAD(level,DEBUG_TRACE,"node is < %s >\n", node->text);
      switch(node->type)
      {
        case lcPRINT: {
          PRINT_PAD(level,DEBUG_TRACE,"this is print operator\n");
          //calculate(node->left);
          exec_print(node->right, level);
          //return;
        } break;
        case lcWHILE: {
          exec_while(node,level);
          return;
        } break;
        case lcIF: {
          exec_if(node,level); 
          return;
        } break;
        case lcIFELSE: {
          exec_ifelse(node,level); 
          return;
        } break;
        case lcEXP:
        {
          calculate(node->right->right);
          //do_statements(node->left, level + 1);
          exec_expression(node->right, level);
          DEBUG_ALL("in exp");
          PRINT_PAD(level,DEBUG_TRACE,"val exp = %f\n", node->right->value.f);
        } break;
        case lcBLOCK:
        {
          do_statements(node->right, level + 1);
        } break;
        case lcSTMNT:
        {
          if (node->right != NULL) {
            PRINT_PAD(level,DEBUG_TRACE,"in statement run of %s\n", node->right->text);
            do_statements(node->right, level);
          }
        }break;
        case lcVARDEF:
        {
          PRINT_PAD(level,DEBUG_TRACE,"var define\n");
          return;
        } break;
        default:
          printf("unknown stmnt is %s with code = %d\n", node->right->text, node->right->type);
          printf("unknown code = %d\n", lcUNKNOWN);

      }
      //PRINT_PAD(level,DEBUG_TRACE,"end\n");
      //assert(node->left != NULL);
      if (node->left!= NULL) do_statements(node->left, level + 1);
      //else { puts("left = 0");}
      //do_statements(node->right, level + 1);
      PRINT_PAD(level,DEBUG_TRACE,"end\n");

      }
    //}
  }
}

void extern_defs(node_t *root)
{
  node_t *node = root;
  printf("root = %s\n------\n", root->text);
  for( ; node->right != NULL; node = node->left)
  {
    switch (node->right->type) {
      case lcVARDEF:
        {
          puts("this is vardef");
        }break; 
      case lcFUNCTION:
        {
          printf("this is func %s\n", node->right->text);
          do_statements(node->right->right,1);
        } break;
      default:
        {
          printf("code type %d\n", node->right->type);
          puts("unknown node");
        }
    }
  }
}

int interprete(node_t *syntax_tree) {
  assert(syntax_tree != NULL);
  printf("Start interpreting...\n"); 
  assert(syntax_tree->text != NULL);
  printf("root text: %s\n", syntax_tree->text);
  //assert(syntax_tree->right != NULL);
  node_t test_var;
  test_var.text = "e";
  if (lookup(&test_var))
  {
    printf("var e is present\n");
  }
  else { printf("not present\n");}
  extern_defs(syntax_tree);
  //do_statements(syntax_tree->right,0);  
  return 0;
}
