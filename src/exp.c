/***************************************************************************/
/****************************** Includes ***********************************/
/***************************************************************************/
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "exp.h"

/***************************************************************************/
/***************** Defines used by this module only ************************/
/***************************************************************************/
#define begin_func()                                                           \
  DEBUG(stderr, "Function is %s line is %d\n", __FUNCTION__, __LINE__)
#define end_func()                                                             \
  DEBUG("On line [%d]\n", __LINE__) // fprintf(stderr, "Function %s is end on
                                     // line %d\n", __FUNCTION__, __LINE__)
#ifdef BTREE_USE

#include "tree.h"

int cmp_var_name(const void *left, const void *right);
typedef struct btree_node_t listof;

listof *vars;
#define push_var(var) btree_insert(&vars, &var, sizeof(var), cmp_var_name)
#define find_var(var) btree_search(vars, &var, cmp_var_name)
#define VAR_STORAGE "BTREE_USE"

#elif LIST_USE

#include "generic_list.h"

void *cmp_var_name(const void *vars, const void *data);
typedef struct list_t listof;

static listof *vars;
#define push_var(var) push(&vars, &var, sizeof(var))
#define find_var(var) foreach_element(vars, var.name, cmp_var_name)
#define VAR_STORAGE "LIST_USE"

#endif

/***************************************************************************/
/********** Global Variables defined for this module only ******************/
/************************ (should be static) *******************************/
/***************************************************************************/
const char *var_storage = VAR_STORAGE;

/***************************************************************************/
/*********************** Function Prototypes *******************************/
/******  (should be static, if not they should be in '.h' file) ************/
/***************************************************************************/
int exp_parser_init();

static int make_builtin_vars();

int make_builtin_vars() {
  variable _false = { "false", 0 };
  variable _true = { "true", 1, };
  push_var(_false);
  push_var(_true);
}

int exp_parser_init() { 
  DEBUG("Used var storage: %s\n", var_storage);
  make_builtin_vars(); 
}

#ifdef BTREE_USE

int cmp_var_name(const void *left, const void *right) {
  return strcmp(((variable*)left)->name, ((variable*)right)->name) == 0;
}

#endif //BTREE_USE

#ifdef LIST_USE

void *cmp_var_name(const void *vars, const void *data) {
  char *name = (char*)data;
  variable *cur_var = (variable*)vars;
  if (!strcmp(name, cur_var->name)) {
    DEBUG("founded var with name  = %s\n", name);
    return cur_var;
  }
  return 0;
}

#endif //LIST_USE

void print_var(void *var) {
  printf("var name = %s\n", ((variable*)(var))->name);
}

int lookup(node_t *node) {
  variable *cur_var;
  int res = 0;
  variable *var = NULL;
  variable tmp;
  tmp.name = node->text;
  if (var = find_var(tmp))
  {
    node->value.f = var->value;
    res = 1;
  }
  return res;
}

float assign_value(node_t *node) {
  variable *cur_var;
  variable tmp;
  assert(node != NULL);
  tmp.name = node->text;
  if (cur_var = find_var(tmp)) {
    //cur_var->name = name;
    cur_var->value = node->value.f;
  }
  else  {
    
    //tmp.name = name;
    tmp.value = node->value.f;
    push_var(tmp);
  }
  return node->value.f;
}

int primary_expression(node_t **root) {
  int res = 0;

  switch (curr_token->type) {

  case lcNUMBER:
  case lcSTRING:
  case lcIDENT: {
    assert(curr_token->text != NULL);
    *root = create_node(curr_token->type, strdup(curr_token->text));
    assert(*root != NULL);
    break;
  }
  case lcLBRACE: {
    get_token(/*NEXT_TOKEN*/);
    res = assignment_expression(root);
    if (curr_token->type != lcRBRACE) {
      printf("Error, expected ')'\n");
      getchar();
      // exit(-1);
    }
    break;
  }
  default:
    ERROR("Error, expected primary on line %d!!! ", get_line());
    ERROR("Meeted %s\n", curr_token->text);
    getchar();
    exit(-1);
  }
  get_token();
  return res;
}

int postfix_expression(node_t **root) {
  int res = 0;
  node_t *node;
  int need_get = TRUE;
  
  res = primary_expression(root);

  switch (curr_token->type)
  {
  case lcPLUS_PLUS: {
  case lcMINUS_MINUS:
    DEBUG("This id postfix expression: %s\n", curr_token->text);
    node = create_node(curr_token->type, strdup(curr_token->text));
    node->left = (*root);
    *root = node;
  } break;
  case '[': {
    get_token();
    res = primary_expression(root);
    if (curr_token->type != ']')
      ERROR("Expected ']'\n");
  } break;
  case lcLBRACE: {
    if (get_token()->type != lcRBRACE)
    {
      //list_t *args;  
      node = create_node(lcCALL, strdup(curr_token->text));
      node->left = (*root);
      *root = node;
      
      res = assignment_expression(root);
      while(curr_token->type == lcCOMMA) {
        get_token();
        res = assignment_expression(root);
      }
      if (curr_token->type != lcRBRACE)
        ERROR("Expected RBRACE\n");
    }
    else
    {
      printf("Empty arg list\n");
      node = create_node(lcCALL, strdup(curr_token->text));
      node->left = (*root);
      *root = node;
    }
  } break;
  case lcPOINT: {
  case lcARROW:
    if (get_token()->type != lcIDENT) {
      ERROR("Expected identifier on line %d\n", get_line());
    } 
    else {
      get_token();
      need_get = FALSE;
      while (curr_token->type == lcPOINT, curr_token->type == lcARROW) {
        if (get_token()->type == lcIDENT)
        {
          get_token();
        }
        else break;
      }
    }
  } break;

  default: {
    need_get = FALSE; 
  } break;
  }

  if (need_get) get_token();
  return res;
}

int unary_expression(node_t **root) {
  int res = 0;
  node_t *node;

  res = postfix_expression(root);

  return res;
}

int multiplicative_expression(node_t **root) {
  int res = 0;
  int stop = 0;
  node_t *node;

  // Build left subtree
  res = unary_expression(root);
  while (curr_token->type == lcMUL || curr_token->type == lcDIV) {
    assert(curr_token->text != NULL);
    node = create_node(curr_token->type, strdup(curr_token->text));
    assert(node != NULL);
    node->left = (*root);
    get_token();
    unary_expression(&(node->right));
    *root = node;
  }
  assert(*root != NULL);
  return res;
}

int additive_expression(node_t **root) {
  int res = 0;
  node_t *node;

  res = multiplicative_expression(root);
  assert(*root != NULL);
  while (curr_token->type == lcPLUS || curr_token->type == lcMINUS) {
    assert(curr_token->text != NULL);
    node = create_node(curr_token->type, strdup(curr_token->text));
    assert(node != NULL);
    node->left = *root;
    get_token();
    multiplicative_expression(&(node->right));
    *root = node;
  }
  assert(*root != NULL);

  return res;
}

int is_relop(token_type type) {
  return (lcAND_OP == type || lcOR_OP == type || lcLE_OP == type ||
          lcGE_OP == type || lcG_OP == type || lcEQ_OP == type ||
          lcL_OP == type || lcNE_OP == type);
}

int conditional_expression(node_t **root) {
  int res = 0;
  node_t *node;
  token_type type = curr_token->type;
  res = additive_expression(root);

  while (is_relop(type = curr_token->type)) {
    assert(curr_token->text != NULL);
    node = create_node(curr_token->type, strdup(curr_token->text));
    assert(node != NULL);
    node->left = *root;
    get_token(/*NEXT_TOKEN*/);
    additive_expression(&(node->right));
    *root = node;

    type = curr_token->type;
  }
  assert(*root != NULL);
  return res;
}

int assignment_expression(node_t **root) {
  char *name;
  int tmp = 0;
  int res = 0;
  token_t prev_token;

  node_t *node;

  name = curr_token->text;
  if (curr_token->type == lcSEMI) {
    return res;
  }
  if (curr_token->type == lcIDENT) {
    char *name;
    int tmp = 0;
    name = curr_token->text;
    char *prev_pos = get_pos();
    memcpy(&prev_token, curr_token, sizeof(token_t));

    *root = create_node(curr_token->type, strdup(curr_token->text));
    token_type type = get_token()->type;
    if (type == lcASSIGN || type == lcPLUS_ASSIGN || type == lcMINUS_ASSIGN ||
        type == lcMUL_ASSIGN || type == lcDIV_ASSIGN) {
      assert(curr_token->text != NULL);
      node = create_node(curr_token->type, strdup(curr_token->text));
      assert(node != NULL);
      node->left = *root;
      get_token(/*NEXT_TOKEN*/);
      assignment_expression(&(node->right));
      node_t *exp = create_node(lcEXP, "expression");
      exp->right = node;
      *root = exp;
      return res;
    } else {
      memcpy(curr_token, &prev_token, sizeof(token_t));
      set_pos(prev_pos);
    }
  }
  res = conditional_expression(root);
  return res;
}

void make_space(int n) {
  for (; n > 0; n--) {
    printf(" ");
  }
}

void print_node(node_t *node, int level) {
  assert(node != NULL);
  // make_space(level);
  switch (node->type) {
  case lcPLUS:
    printf("add");
    break;
  case lcMINUS:
    printf("sub");
    break;
  case lcASSIGN:
    printf("move");
    break;
  }
}

void functional(node_t *tree, int level) {
  if (tree) {
    switch (tree->type) {
    case lcPLUS:
    case lcMINUS:
    case lcASSIGN: {
      print_node(tree, level + 1);
      putchar('(');
      functional(tree->left, level + 1);
      printf(", ");
      functional(tree->right, level + 1);
      putchar(')');
      break;
    }
    default:
      printf("%s", tree->text);
    }
  }
}

void calculate(node_t *tree) {
  
  float *val = &tree->value.f, val1 = 0, val2 = 0;
  if (tree) {
    calculate(tree->left);
    calculate(tree->right);
    switch (tree->type) {
    case lcPLUS: {
      *val = tree->value.f = tree->left->value.f + tree->right->value.f;
    } break;
    case lcMINUS: {
      *val = tree->value.f = tree->left->value.f - tree->right->value.f;
    } break;
    case lcMUL: {
      *val = tree->value.f = tree->left->value.f * tree->right->value.f;
    } break;
    case lcDIV: {
      *val = tree->value.f = tree->left->value.f / tree->right->value.f;
    } break;
    case lcINC_OP: {
      *val = tree->value.f++;
    } break;
    case lcDEC_OP: {
      *val--;
    } break;

    case lcAND_OP: {
      *val = tree->value.f = (tree->left->value.f &&  tree->right->value.f);
      //*val = (val1 && val2);
    } break;
    case lcOR_OP: {
      *val = tree->value.f = (tree->left->value.f ||  tree->right->value.f);
      //*val = (val1 || val2);
    } break;
    case lcLE_OP: {
      *val = tree->value.f = (tree->left->value.f <=  tree->right->value.f);
    } break;
    case lcGE_OP: {
      *val = tree->value.f = (tree->left->value.f >=  tree->right->value.f);
    } break;
    case lcEQ_OP: {
      *val = tree->value.f = (tree->left->value.f ==  tree->right->value.f);
      //*val = (val1 == val2);
    } break;
    case lcNE_OP: {
      *val = tree->value.f = (tree->left->value.f !=  tree->right->value.f);
      //*val = (val1 != val2);
    } break;

    case lcG_OP: {
      *val = tree->value.f = (tree->left->value.f >  tree->right->value.f);
      //*val = (val1 > val2);
    } break;
    case lcL_OP: {
      //printf("right value = %f\n",tree->right->value.f); 
      *val = tree->value.f = (tree->left->value.f <  tree->right->value.f);
      //*val = (val1 < val2);
    } break;

    case lcNUMBER: {
      *val = atof(tree->text);
    } break;
    case lcIDENT: {
      if (!lookup(tree)) {
        printf("Undefined var: %s\n", tree->text);
      }
    } break;

    case lcASSIGN: {
      calculate(tree->right);
      tree->left->value.f = tree->right->value.f;
      *val = assign_value(tree->left);
      //printf("assign res = %f\n", *val);
    } break;
    case lcPLUS_ASSIGN: {
      calculate(tree->right);
      tree->left->value.f += tree->right->value.f; 
      *val = assign_value(tree->left);
      // printf("plus_asign = %f\n", *val);
    } break;
    case lcMINUS_ASSIGN: {
      calculate(tree->right);
      tree->left->value.f -= tree->right->value.f; 
      *val = assign_value(tree->left);
    } break;
    case lcMUL_ASSIGN: {
      calculate(tree->right);
      tree->left->value.f *= tree->right->value.f; 
      *val = assign_value(tree->left);
    } break;
    case lcDIV_ASSIGN: {
      calculate(tree->right);
      tree->left->value.f /= tree->right->value.f; 
      *val = assign_value(tree->left);
    } break;
    case lcPLUS_PLUS: {
      calculate(tree->left);
      *val = tree->left->value.f++;
      assign_value(tree->left);
    } break;
    case lcMINUS_MINUS: {
      calculate(tree->left);
      *val = tree->left->value.f--;
      assign_value(tree->left);
    } break;
    case lcCALL: {
      calculate(tree->left);
      //printf("func call\n");
      assign_value(tree->left);
      *val = tree->left->value.f;
    } break;
    }
  }
}


node_t *eval() {
  node_t *root;
  float retval = assignment_expression(&root);

  // functional(root,0);
  //calculate(root);
  DEBUG("result = %f\n", root->value.f);
  return root;
}
