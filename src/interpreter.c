#include "interpreter.h"
#include "common.h"
#include "exp.h"
#include "lexer.h"
#include "preprocessor.h"

#include <memory.h> //memcpy
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define FATAL_ERROR(str) (fprintf(stderr, "Fatal error on line %d: %s\n", __LINE__, str), exit(-1))
//#define MATCH(tok) curr_token->type != tok ? FATAL_ERROR("error") : get_token(); 
static void MATCH(token_type tok) 
{
    curr_token->type != tok ? FATAL_ERROR("error") : (void)get_token();
}
#define TYPES_CAPACITY 256
#define STACK_SIZE 64 
#define FUNCTIONS_COUNT 128 


//#define CREATE_PRIMITIVE_TYPE(n, t) string_ref_assign(tmp_type->name, n); tmp_type->object_type = PRIMITIVE; tmp_type->btype = t; add_type(global_context, tmp_type);

typedef struct interpreter_context
{
    type_t* types;
    int num_types;
    int types_capacity;

    int num_funcs;
    function_t functions[FUNCTIONS_COUNT];
    function_t* current_function;
    function_t* entry_point;

    int stack[STACK_SIZE];
    int sp;
}interpreter_context;

extern char *token_to_string[];
static type_t* tmp_type = NULL;
interpreter_context* global_context;

void skip_compound_statement();
void skip_statement();
int function_definition();
int declaration_list();
static type_t* find_type(interpreter_context* ctx, string_ref lexem);

static void CREATE_PRIMITIVE_TYPE(char* name, builtin_types t);
void call_cfunction(interpreter_context* ctx, function_t* func);

void push_integer(interpreter_context* ctx, int value);

int myCFunction(interpreter_context* ctx)
{
  printf("this is %s\n", __FUNCTION__);
  push_integer(ctx, 123);
  return 1;
}

bool register_cfunction(interpreter_context *ctx, CFunction* function, const char* name)
{
  function_t* func = &ctx->functions[ctx->num_funcs++];
  
  func->name = string_ref_create(name);
  func->cfunc = function;
  func->type = C_FUNCTION;
}

bool is_cfunction(interpreter_context* ctx, string_ref name)
{
  for (int i = 0; i < ctx->num_funcs; i++)
  {
    if (!strncmp(ctx->functions[i].name.pos, name.pos, ctx->functions[i].name.len) && ctx->functions[i].type == C_FUNCTION)
    {
      return true;
    }
  }
  return false;
}

interpreter_context* create_interpreter_context()
{
    interpreter_context *ctx = malloc(sizeof(interpreter_context));
    global_context = ctx;
    if (ctx != NULL)
    {
        ctx->num_types = 0;
        ctx->types_capacity = TYPES_CAPACITY;
        ctx->types = malloc(sizeof(type_t) * TYPES_CAPACITY);
        ctx->num_funcs = 0;
        ctx->entry_point = NULL;
        ctx->current_function = NULL;
        ctx->sp = 0;
    }
    return ctx;
}

static void add_type(interpreter_context* ctx, type_t* t)
{
    ctx->types[ctx->num_types++] = *t;
}

static type_t* create_type()
{
    type_t* result = malloc(sizeof(type_t));

    if (result != NULL)
    {
        string_ref_assign(&result->name, "unknown");
        result->names = NULL;
        result->object_type = UNKNOWN_OBJECT;
        result->types = NULL;
        result->num_types = 0;
    }
}


static void interpreter_init()
{
    create_interpreter_context();
    tmp_type = create_type();
    CREATE_PRIMITIVE_TYPE("int", INT_TYPE);
    CREATE_PRIMITIVE_TYPE("char", CHAR_TYPE);
    CREATE_PRIMITIVE_TYPE("uint", UINT_TYPE);
    CREATE_PRIMITIVE_TYPE("uchar", UCHAR_TYPE);
    CREATE_PRIMITIVE_TYPE("float", FLOAT_TYPE);
    register_cfunction(global_context, myCFunction, "myCFunction");
//#undef CREATE_PRIMITIVE_TYPE 
}

token_type eat_tokens(token_type skip_to);

int gen_label() {
	static int label;
	return label++;
}

void exptected_func(char *exptected) {
  printf("Error. Expected %s\n", exptected);
}

void skip_if() {
  if (eat_tokens(lcRBRACE) == lcRBRACE) {
    get_token(/*NEXT_TOKEN*/);
    skip_statement();
  }
}

void skip_while() {
  if (eat_tokens(lcRBRACE) == lcRBRACE) {
    get_token(/*NEXT_TOKEN*/);
    skip_statement();
  }
}

token_type eat_tokens(token_type skip_to) {
  token_type type = lcEND;
  if ((type = curr_token->type) == skip_to)
    return type;
  while (((type = get_token(/*NEXT_TOKEN*/)->type) != skip_to) && type != lcEND)
    ;
  return type;
}
void skip_statement() {
  int stop = 0;
  while (!stop) {
    switch (curr_token->type) {
    case lcLBRACKET: {
      skip_compound_statement();
      if (curr_token->type != lcRBRACKET) {
        printf("error: expected }\n");
      }
      /*else
      {

      }*/
      return;
    } break;
    case lcIF: {
      skip_if();
    } break;
    case lcWHILE: {
      skip_while();
    } break;
    case lcRBRACKET:
      return;
    default:
      if (eat_tokens(lcSEMI) == lcSEMI)
        break;
      else
        return;
    }
    get_token(/*NEXT_TOKEN*/);
  }
}

void skip_compound_statement() {
  int bracket_lvl = 0;
  int res = 0;
  token_t prev_token;

  memcpy(&prev_token, curr_token, sizeof(token_t));
  if (!(curr_token->type == lcLBRACKET &&
        get_token(/*NEXT_TOKEN*/)->type == lcRBRACKET) &&
      (curr_token->type != lcEND)) {
    if (prev_token.type == lcLBRACKET) {
      skip_statement();
      if (curr_token->type == lcRBRACKET) {

      } else {
        printf("error: expected }\n");
      }
    }
  }
}

way_out do_if() {
  int condition = 0;
  way_out out = NORMAL;

  MATCH(lcLBRACE); 
  
  int endif = gen_label();
  int else_label = gen_label();
  // gencode
  assignment_expression();
  MATCH(lcRBRACE);
  statement(SELECTION); 
  if (curr_token->type == lcELSE) {
	MATCH(lcELSE);
	statement(SELECTION);
	printf("_label_%04x:\n", else_label);
  }
  printf("_label_%04x:\n", endif);


 
  ///*get_token(/*NEXT_TOKEN*/);*/
  return out;
}

way_out do_while() {
  int condition = 0;
  way_out out = NORMAL;

  MATCH(lcLBRACE); 
  //assignment_expression();
  int header = gen_label();
  int end_while = gen_label();
    // gencode
  printf("_label_%04x:\n",header);
  //printf("\tcmp\n");
  assignment_expression();
  //printf("\tcmp\n");
  //printf("\tjnz _label_%04x\n", end_while);
  MATCH(lcRBRACE);
  //get_token();

  statement(ITERATION);
  printf("\tjmp _label_%04x\n", header);
  printf("_label_%04x:\n",end_while);

  get_token();
  /*
	while (get_token(), condition = assignment_expression()) {
	  if ((curr_token = curr_token)->type == lcRBRACE) {
		get_token();
		if ((out = statement(ITERATION)) == BREAK) {
		  break;
		} else if (out == CONTINUE || out == NORMAL) {
		  pos_end = curr_token->pos;
		  set_pos(pos_begin);
		  get_token();
		  continue;
		} else if (out == RETURN) {
		  return RETURN;
		}
	  }
	}
  }
  */

  return out;
}

int func_decl() { return 0; }

void print_var(type_t* type, string_ref name, int level)
{
  type_t* curr_type;

  {
    switch (type->object_type)
    {
    case PRIMITIVE:
    {

      printf(
        "%*sstype [%.*s], name [%.*s]\n", level, "\t",
        type->name.len, type->name.pos,
        name.len, name.pos
      );
    }
    break;
    case STRUCT:
    {
      printf("Struct %.*s parsed and have %d fields:\n", type->name.len, type->name.pos, type->num_types);
      for (int i = 0; i < type->num_types; i++)
      {
        print_var(&type->types[i], type->names[i], level + 1);
      }
    }
    break;
    default:
      FATAL_ERROR("unknown type");
      break;
    }
  }
}

bool declare_variable(variable* var)
{
  var->type = find_type(global_context, curr_token->text);
  if (get_token()->type == lcIDENT)
  {
    var->name = curr_token->text;
    get_token();
    return true;
  }
  return false;
}

void struct_declaration(type_t *new_type)
{
  //type_t new_type;

  if (get_token()->type == lcIDENT)
  {
    new_type->object_type = STRUCT;
    new_type->name = curr_token->text;
    if (get_token()->type == lcLBRACKET)
    {
      do
      {
        get_token();
        if (curr_token->type == lcRBRACKET)
        {
          ;// print_var(new_type, new_type->name);
        }
        else if (curr_token->type == lcSTRUCT)
        {
          type_t* nested_struct = create_type();
          struct_declaration(nested_struct);

          new_type->num_types++;
          new_type->types = realloc(new_type->types, sizeof(type_t) * new_type->num_types);
          new_type->types[new_type->num_types - 1] = *nested_struct;
          if (curr_token->type != lcSEMI)
          {
            if (get_token()->type == lcIDENT)
            {
              new_type->names = realloc(new_type->names, sizeof(string_ref) * new_type->num_types);
              new_type->names[new_type->num_types - 1] = nested_struct->name;
            }
            get_token();
          }
          else
          {

          }
        }
        else if (is_type(curr_token->text))
        {
          variable var;
          if (declare_variable(&var))
          {
            new_type->num_types++;
            new_type->names = realloc(new_type->names, sizeof(string_ref) * new_type->num_types);
            new_type->names[new_type->num_types - 1] = var.name;
            new_type->types = realloc(new_type->types, sizeof(type_t) * new_type->num_types);
            new_type->types[new_type->num_types - 1] = *var.type;
          }
          else
          {
            FATAL_ERROR("could not declare struct field");
          }

        }
        else
        {
          FATAL_ERROR("this is not type");
        }
      } while (curr_token->type == lcSEMI);
      get_token();
    }
  }
}

int start(char **buffer) {
  int retval = 0;

  exp_parser_init();
  interpreter_init();

  string_ref name = string_ref_create("myCFunction");
  function_t* func = find_cfunction(global_context, name);
  interpreter_context* ctx = global_context;

  //if (is_cfunction(ctx, name))
  //  call_cfunction(ctx, func);

  if ((lexer_init(*buffer)) != 0) {
    while (get_token(/*NEXT_TOKEN*/)->type != lcEND) {
      switch (curr_token->type)
      {
      case lcSTRUCT:
      {
        //type_t *new_type = &ctx->types[ctx->num_types - 1];
        type_t *new_type = create_type();
        struct_declaration(new_type);
        add_type(ctx, new_type);
        if (curr_token->type != lcSEMI)
        {
          FATAL_ERROR("semi not found");
        }
        ctx->num_types++;
        print_var(new_type, new_type->name, 0);
      }
      break;
      default:
        retval = function_definition();
        break;
      }
    }
  }

  return retval;
}

int is_print() { return 0; }

int print() {
  int stop = 0;

  get_token(/*NEXT_TOKEN*/);
  do {
    char *number = "%d";
    char *string = "%s";
    char *curtype;
    int expr_val = 0;
    if (curr_token->type == lcSTRING) {
      printf("%.*s", curr_token->text.len, curr_token->text.pos);
      get_token(/*NEXT_TOKEN*/);
    } else {
      curtype = number;
      expr_val = assignment_expression();
      printf("%d", expr_val);
    }
  } while ((curr_token = curr_token)->type == lcSTRING ||
           curr_token->type == lcIDENT);
  puts("");
}

int read() {
  int stop = 0;
  int tmp;

  while (get_token(/*NEXT_TOKEN*/)->type != lcSEMI &&
         curr_token->type != lcEND) {
    if (curr_token->type == lcIDENT) {
      scanf("%d", &tmp);
      assign_value(curr_token->text, tmp);
    }
  }
}

int interprete() {
  if (get_token(/*NEXT_TOKEN*/)->type == lcSTRING) {
    start((char **)&(curr_token->text));
  }
  get_token(/*NEXT_TOKEN*/);
  return 0;
}

way_out statement(compound_origin origin) {
  int res = 0;
  int expr_len;
  way_out out = NORMAL;
  int stop = 0;

  while (!stop) {
    switch (curr_token->type) {
    case lcIF: {
	  get_token(/*NEXT_TOKEN*/);
      if ((out = do_if()) == CONTINUE || out == BREAK)
        return out;
    } break;
    case lcWHILE: {
	  get_token();
      do_while();
    } break;
    case lcBREAK: {
      if (get_token(/*NEXT_TOKEN*/)->type == lcSEMI) {
        get_token(/*NEXT_TOKEN*/);
        skip_statement();
        return BREAK;
      } else {
        exptected_func("SEMI");
      }
    } break;
    case lcGOTO: {
		MATCH(lcIDENT);
		//gencode
		printf("\tjmp %s\n", curr_token->text);
		MATCH(lcSEMI);
    } break;
    case lcCONTINUE: {
      if (get_token(/*NEXT_TOKEN*/)->type == lcSEMI) {
        get_token(/*NEXT_TOKEN*/);
        skip_statement();
        return CONTINUE;
      } else {
        exptected_func("SEMI");
      }
    } break;
    case lcLBRACKET: {
      out = compound_statement(origin);
      if (curr_token->type != lcRBRACKET) {
        printf("error: expected }\n");
      }
      return out;
    } break;
    case lcFUNCTION: {
      if (get_token(/*NEXT_TOKEN*/)->type == lcIDENT) {
        func_decl();
      } else {
        exptected_func("IDENT");
      }
    } break;
    case lcPRINT: {
      print();
      if ((curr_token = curr_token)->type == lcSEMI) {
        get_token(/*NEXT_TOKEN*/);
      }
    } break;
    case lcREAD: {
      read();

    } break;
    case lcABORT: {
      get_token(/*NEXT_TOKEN*/);
      puts("This is abort!");
      out = -1;
      goto abort;
    } break;
    case lcINTERPRETE: {
      interprete();
    } break;
    case lcNUMBER: {
    case lcIDENT: 

      res = assignment_expression();
      if (curr_token->type != lcSEMI) {
        exptected_func("SEMI");
        goto abort;
      }
      get_token(/*NEXT_TOKEN*/);
    } break;
    default:
      stop = 1;
      break;
    }
  }

abort:
  return out;
}
way_out compound_statement(compound_origin origin) {
  way_out out = NORMAL;
  int expr_len;
  token_t prev_token;
  int retval = 0;

  prev_token = *curr_token;
  if (curr_token->type == lcLBRACKET &&
      get_token(/*NEXT_TOKEN*/)->type == lcRBRACKET) {
    retval = 0;
  } else {
    if (prev_token.type == lcLBRACKET) {
      out = statement(origin);
      if (curr_token->type == lcRBRACKET) {

      } else {
        printf("error: expected }\n");
      }
    }
  }
  return out;
}

static type_t* find_type(interpreter_context* ctx, string_ref lexem)
{
    type_t* result = NULL;
    for (int i = 0; i < ctx->num_types; i++)
    {
        if (!strncmp(ctx->types[i].name.pos, lexem.pos, ctx->types[i].name.len))
        {
            result = &ctx->types[i];
            break;
        }
    }
    return result;
}

int is_type(string_ref lexem) {
#if 0
  int res = FALSE;
  switch (type) {
  case lcINT:
  case lcVOID:
  case lcCHAR:
    res = TRUE;
    break;
  }
#endif
  
  return find_type(global_context, lexem) != NULL;
}

int function_definition() {
  way_out out;
  token_type type = curr_token->type;
  if (is_type(curr_token->text) && get_token(/*NEXT_TOKEN*/)->type == lcIDENT) {
    get_token(/*NEXT_TOKEN*/);
    declaration_list();
    out = compound_statement(COMPOUND);
  }

  return 0;
}

int declaration_list() {
  token_type type;
  int retval = -1;
  if (curr_token->type == lcLBRACE) {
    if (get_token(/*NEXT_TOKEN*/)->type != lcRBRACE)
    {
      if (is_type(curr_token->text)) {
        if (get_token(/*NEXT_TOKEN*/)->type == lcIDENT) {
          while (get_token(/*NEXT_TOKEN*/)->type == lcCOMMA &&
                 is_type(get_token(/*NEXT_TOKEN*/)->text) &&
                 get_token(/*NEXT_TOKEN*/)->type == lcIDENT)
            ;
          if (curr_token->type != lcRBRACE) {
            exptected_func("RBRACE");
          } 
          else {
            retval = 0;
          }
        }
      } 
    }
    else {
      retval = 0;
    }
  }
  get_token(/*NEXT_TOKEN*/);
}

static void CREATE_PRIMITIVE_TYPE(char* name, builtin_types t)
{
    tmp_type->name = string_ref_create(name);
    tmp_type->object_type = PRIMITIVE;
    tmp_type->btype = t;
    add_type(global_context, tmp_type);
}

void call_cfunction(interpreter_context* ctx, function_t* func)
{
  if (func != NULL)
  {
    int result = func->cfunc(ctx);
    printf("function results count = %d, value = %d\n", result, ctx->stack[ctx->sp - 1]);
    ctx->sp -= result;
  }
}

void push_integer(interpreter_context* ctx, int value)
{
  if (ctx->sp < STACK_SIZE)
  {
    ctx->stack[ctx->sp++] = value;
  }
}

void pop_integer(interpreter_context* ctx)
{
  if (ctx->sp >= 0)
  {
    ctx->sp++;
  }
}

function_t* find_cfunction(interpreter_context* ctx, string_ref name)
{
  for (int i = 0; i < ctx->num_funcs; i++)
  {
    function_t* func = &ctx->functions[i];
    if (!strncmp(func->name.pos, name.pos, func->name.len))
    {
      return func;
    }
  }
}

