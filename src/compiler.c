#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "utils.h"
#include "ast.h"
#include "compiler.h"
#include "bytecode.h"


Program* image ;
int   const_index;
int scope_level;
int RANDNUM;

char* int_to_id(int id ){
	char* str = malloc(sizeof(int)*8+1);
	snprintf(str,sizeof(int)*8+1,"%d", id);
	return str;
}

char* concat(char *s1, char *s2){
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char *result = malloc(len1+len2+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    memcpy(result, s1, len1);
    memcpy(result+len1, s2, len2+1);//+1 to copy the null-terminator
    return result;
}

void* allocate_obj(ValTag tag){
	Value* res ;
	switch(tag){
		case INT_VAL: {
			IntValue* obj = (IntValue*)malloc(sizeof(IntValue));
			obj->tag = INT_VAL;
			obj->value = 0;
			res = obj;
			break;
		}
		case NULL_VAL:{
			res = (Value*) malloc(sizeof(Value*));
			res->tag = NULL_VAL;
			break;
		}
		case STRING_VAL:{
			StringValue* obj = (StringValue*)malloc(sizeof(StringValue));
			obj->tag = STRING_VAL;
			res = obj;
			break;
		}
		case METHOD_VAL:{
			MethodValue* obj = (MethodValue*)malloc(sizeof(MethodValue));
			obj->tag = METHOD_VAL;
			obj->name = 0;
			obj->nargs = 0;
			obj->nlocals = 0;
			obj->code = make_vector();
			res = obj;
			break;
			
		}
		case SLOT_VAL:{
			SlotValue* obj = (SlotValue*)malloc(sizeof(SlotValue));
			obj->tag = SLOT_VAL;
			res = obj;
			break;
		}
		case CLASS_VAL:{
			ClassValue* obj = (ClassValue*)malloc(sizeof(ClassValue));
			obj->tag = CLASS_VAL;
			res = obj;
			break;
		}
		default:
		    printf("Cannot allocate unknown type! exiting...");
		    exit(1);
	}
	return res;
}

typedef struct {
	char* key;
	int value;
}Symbol;

typedef struct {
	MethodValue* fn;
	Vector* display;
}SemanticRecord;


typedef struct {
	Vector* stack;
	Vector* constants;
	Vector* globals;
	int nestlevel;
}CompileState;

CompileState* compile_state;


#define ADD_INS(state, ins) SemanticRecord* frame = record_current_frame(state);\
                     vector_add(frame->fn->code, ins);
#define ADD_GLOBAL(state, val) vector_add(state->globals, val);
#define LOCSCOPE     (state->nestlevel > 0)

void emit_exp ( CompileState* state, Exp* e);
void emit_scopestmt (CompileState* state, ScopeStmt* s);
void compile_var_stmt(CompileState* state, ScopeVar* stmt);
void compile_fn_stmt(CompileState* state, ScopeFn* fn);
void compile_exp_stmt(CompileState* state, ScopeExp* exp);
void compile_int_exp(CompileState* state, IntExp* exp);


void init(){
	image = (Program*)malloc(sizeof(Program));
	image->values = make_vector();
	image->slots = make_vector();
	const_index = 0;
	RANDNUM = 34;
}

CompileState* compile_state_init(){
	CompileState* compile_state = malloc(sizeof(CompileState));
	compile_state->stack = make_vector();
	compile_state->constants = make_vector();
	compile_state->globals = make_vector();
	compile_state->nestlevel = 0;
	return compile_state;
}

int add_const( Vector* vec, Value* val){
	int index;
	vector_add(vec, val);
	index = vec->size-1;
	return index;

}

int add_symbol(Vector* display, char* key){
	Symbol* res = malloc(sizeof(Symbol));
	res->key = key;
	vector_add(display, res);
	int index = display->size-1;
	res->value = index;
	return index;
}

SemanticRecord* record_new( MethodValue* fn){
	SemanticRecord* res = malloc(sizeof(SemanticRecord));
	res->fn = fn;
	res->display = make_vector();
	return res;
}

void record_push(CompileState* state, SemanticRecord* frame){
	vector_add(state->stack, frame);
	state->nestlevel = state->stack->size-1;
}

SemanticRecord* record_pop(CompileState* state){
	SemanticRecord* res = vector_pop(state->stack);
	state->nestlevel = state->stack->size-1;
	return res;
	
}	

SemanticRecord* record_current_frame(CompileState* state){
	return vector_peek(state->stack);
}
	
int lookup(CompileState* state, char* key){
	int index;
    SemanticRecord* frame = record_current_frame(state);
    for(int it = 0; it<frame->display->size; it++){
		Symbol* sym = (Symbol*)vector_get(frame->display, it);
		if(strcmp(sym->key, key)==0){
			index = sym->value;
			goto success;
		}
	}
	for(int jt = 0; jt<state->constants->size; jt++){
		Value* val = (Value*)vector_get(state->constants, jt);
		if(val->tag == STRING_VAL){
			StringValue* s2 = (StringValue*)val;
			if(strcmp(s2->value, key)==0){
				index = jt;
				goto success;
			}
		}
	}
	printf("Symbol %s not defined\n",key);
	exit(1);
	success:
		
	return index;
}

MethodValue* lookup_global(CompileState* state, char* name){
	MethodValue* fn ;
	for(int jt = 0; jt<state->globals->size; jt++){
		int index = (int) vector_get(state->globals, jt);
        fn = (MethodValue*) vector_get(state->constants, index);
        StringValue* ref = (StringValue*) vector_get(state->constants, fn->name);
        if((int)strcmp(ref->value, name)==0){
			return fn;
		}
	}
	printf("No global named %s found\n", name);
	exit(1);
	succes:
	return fn;
	
}

int gen_global_ref(CompileState* state, char* name){
	 StringValue* n = (StringValue*)allocate_obj(STRING_VAL);
     n->value = name;  
     int index = add_const(state->constants, n);
     return index;
}
	
int gen_ref(CompileState* state, char* name ){
	int index;
	SemanticRecord* frame = record_current_frame(state);

    if(LOCSCOPE){
		frame->fn->nlocals++;
        index = add_symbol(frame->display, name);
         	
	}else{
        gen_global_ref(state, name);
	}
    RANDNUM++;
    return index;
 }
 
int gen_arg_ref(SemanticRecord* frame, char* name ){
	int index;
    index = add_symbol(frame->display, name);
         	
    return index;
 }
 
int gen_slot(CompileState* state, int index){
	SlotValue* slot = allocate_obj(SLOT_VAL);
	slot->name = index;
	return add_const(state->constants, slot);
	
}

int gen_method(CompileState* state, MethodValue* fn){
	int index = add_const(state->constants, fn);
	return index; 
}

int gen_literal(CompileState* state, IntExp* num){
	IntValue* val = (IntValue*)allocate_obj(INT_VAL);
	val->value = num->value;
	return add_const(state->constants, val);	
}

void gen_lit_ins(CompileState* state, int index){
    LitIns* lit  = malloc(sizeof(LitIns));
	lit->tag = LIT_OP;
	lit->idx = index;
	ADD_INS(state, lit);
	
}

void gen_get_local_ins(CompileState* state, int index){
	GetLocalIns* ins = malloc(sizeof(GetLocalIns));
	ins->tag = GET_LOCAL_OP;
	ins->idx = index;
	ADD_INS(state, ins);	
}

void gen_get_global_ins(CompileState* state, int index){
	GetLocalIns* ins = malloc(sizeof(GetGlobalIns));
	ins->tag = GET_GLOBAL_OP;
	ins->idx = index;
	ADD_INS(state, ins);	
}

void gen_set_local_ins(CompileState* state, int index){
	SetLocalIns* setins = malloc(sizeof(SetLocalIns)); 
	setins->tag = SET_LOCAL_OP;
	setins->idx = index;
	ADD_INS(state, setins);
	
}

void gen_call_ins(CompileState* state, MethodValue* fn){
	CallIns* ins = malloc(sizeof(CallIns));
	ins->tag = CALL_OP;
	ins->name = fn->name;
	ins->arity = fn->nargs;
	ADD_INS(state, ins);
}

void gen_set_global_ins(CompileState* state, int index){
	SetGlobalIns* setins = malloc(sizeof(SetGlobalIns));
	setins->tag = SET_GLOBAL_OP;
	setins->name = index;
	ADD_INS(state, setins);	
}

void gen_drop(CompileState* state){
	ByteIns* ins = malloc(sizeof(ByteIns));
	ins->tag = DROP_OP;
	ADD_INS(state, ins);
}

void gen_return(CompileState* state){
	ByteIns* ins = malloc(sizeof(ByteIns));
	ins->tag = RETURN_OP;
	ADD_INS(state, ins);
}


void gen_print_ins(CompileState* state, int format_index, int arity){
	PrintfIns* ins = malloc(sizeof(PrintfIns));
	ins->tag = PRINTF_OP;
	ins->format = format_index;
	ins->arity = arity;
	ADD_INS(state, ins);
}

char* gen_unique_name(char* name){
	return concat(name, int_to_id(RANDNUM++));
}

void compile_var_stmt(CompileState* state, ScopeVar* stmt){
	int slot_index;
	emit_exp(state, stmt->exp);
	
	if(LOCSCOPE){
		int index = gen_ref(state, stmt->name);
		gen_set_local_ins(state, index);
	}else{
		int index = gen_global_ref(state, stmt->name);
	    gen_set_global_ins(state, index);
	    slot_index = gen_slot(state, index);
	    ADD_GLOBAL(state, slot_index);
	}
	
	
	gen_drop(state);
}

void compile_int_exp(CompileState* state, IntExp* exp){
	int index = gen_literal(state, exp);
	gen_lit_ins(state, index);
}

void compile_ref_exp(CompileState* state, RefExp* exp){
    
	SemanticRecord* frame = record_current_frame(state);
	int index = lookup(state, exp->name);
	if(LOCSCOPE){
		gen_get_local_ins(state, index);
	}else{
		gen_get_global_ins(state, index);	
	}
}

void compile_fn_stmt(CompileState* state, ScopeFn* fn){
     MethodValue* method = allocate_obj(METHOD_VAL);
     SemanticRecord* frame = record_new(method);
     record_push(state, frame);
     method->nargs = fn->nargs;
     
     for(int it=0; it<fn->nargs; it++){
		 gen_arg_ref(frame, fn->args[it]);
	 }
     emit_scopestmt(state, fn->body);
     gen_return(state);
     int fnref = gen_global_ref(state, fn->name);
     method->name = fnref;
     int index = gen_method(state, method);
     
     record_pop(state);
     if(!LOCSCOPE){ADD_GLOBAL(state, index);} 
    	
}

void compile_call_exp(CompileState* state, CallExp* exp){
	MethodValue* fn = lookup_global(state, exp->name);
	gen_call_ins(state, fn);
	gen_drop(state);
}

int compile_program(CompileState* compile_state, ScopeStmt* stmt){
  MethodValue* entry = allocate_obj(METHOD_VAL);
  SemanticRecord* globalscope = record_new(entry);
  record_push(compile_state, globalscope);
  
  emit_scopestmt(compile_state, stmt);
  Value* null = allocate_obj(NULL_VAL);
  
  gen_lit_ins(compile_state, add_const(compile_state->constants, null));
  gen_return(compile_state);
  char* funcname = gen_unique_name("entry");
  int entry_index = gen_global_ref(compile_state, funcname);
  entry->nargs = 0;
  entry->name = entry_index;

  int index = gen_method(compile_state, entry);
  record_pop(compile_state);
  return index;
}

void compile_null_exp(CompileState* state){
    Value* null = allocate_obj(NULL_VAL);
	gen_lit_ins(state, add_const(compile_state->constants, null));
}


void compile_print_exp(CompileState* state, PrintfExp* exp){
	for(int i = 0; i<exp->nexps; i++){
		emit_exp(state, exp->exps[i]);
	}
	int formatref = gen_global_ref(state, exp->format);
	gen_print_ins(state, formatref, exp->nexps);
	gen_drop(state);
}


void emit_exp ( CompileState* state, Exp* e) {
  
  switch(e->tag){
  case INT_EXP:{
	IntExp* e2 = (IntExp*)e;
	compile_int_exp(state, e2);
	
    break;
  }
  case NULL_EXP:{
    compile_null_exp(state);
    break;
  }
  case PRINTF_EXP:{
    PrintfExp* e2 = (PrintfExp*)e;
    compile_print_exp(state, e);
    break;
  }
  case ARRAY_EXP:{
    ArrayExp* e2 = (ArrayExp*)e;
    printf("array(");
    print_exp(e2->length);
    printf(", ");
    print_exp(e2->init);
    printf(")");
    break;
  }
  case OBJECT_EXP:{
    ObjectExp* e2 = (ObjectExp*)e;
    printf("object : (");
    for(int i=0; i<e2->nslots; i++){
      if(i > 0) printf(" ");
      print_slotstmt(e2->slots[i]);
    }
    printf(")");
    break;
  }
  case SLOT_EXP:{
    SlotExp* e2 = (SlotExp*)e;
    print_exp(e2->exp);
    printf(".%s", e2->name);
    break;
  }
  case SET_SLOT_EXP:{
    SetSlotExp* e2 = (SetSlotExp*)e;
    print_exp(e2->exp);
    printf(".%s = ", e2->name);
    print_exp(e2->value);
    break;
  }
  case CALL_SLOT_EXP:{
    CallSlotExp* e2 = (CallSlotExp*)e;
    print_exp(e2->exp);
    printf(".%s(", e2->name);
    for(int i=0; i<e2->nargs; i++){
      if(i > 0) printf(", ");
      print_exp(e2->args[i]);
    }
    printf(")");
    break;
  }
  case CALL_EXP:{
    CallExp* e2 = (CallExp*)e;
    compile_call_exp(state, e2);
    break;
  }
  case SET_EXP:{
    SetExp* e2 = (SetExp*)e;
    printf("%s = ", e2->name);
    print_exp(e2->exp);
    break;
  }
  case IF_EXP:{
    IfExp* e2 = (IfExp*)e;
    
    printf("if ");
    print_exp(e2->pred);
    printf(" : (");
    print_scopestmt(e2->conseq);
    printf(") else : (");
    print_scopestmt(e2->alt);
    printf(")");
    break;
  }
  case WHILE_EXP:{
    WhileExp* e2 = (WhileExp*)e;
    printf("while ");
    print_exp(e2->pred);
    printf(" : (");
    print_scopestmt(e2->body);
    printf(")");
    break;
  }
  case REF_EXP:{
    RefExp* e2 = (RefExp*)e;
    compile_ref_exp(state, e2);
    
    break;
  }
  default:
    printf("Unrecognized Expression with tag %d\n", e->tag);
    exit(-1);
  }
  
}

void emit_slotstmt (SlotStmt* s) {
  switch(s->tag){
  case VAR_STMT:{
    SlotVar* s2 = (SlotVar*)s;
    printf("var %s = ", s2->name);
    print_exp(s2->exp);
    break;
  }
  case FN_STMT:{
    SlotMethod* s2 = (SlotMethod*)s;
    printf("method %s (", s2->name);
    for(int i=0; i<s2->nargs; i++){
      if(i>0) printf(", ");
      printf("%s", s2->args[i]);
    }
    printf(") : (");
    print_scopestmt(s2->body);
    printf(")");
    break;
  }
  default:
    printf("Unrecognized slot statement with tag %d\n", s->tag);
    exit(-1);
  }
}

void emit_scopestmt (CompileState* state, ScopeStmt* s) {
  switch(s->tag){
  case VAR_STMT:{
    ScopeVar* s2 = (ScopeVar*)s;
    compile_var_stmt(state, s2);
    break;
  }
  case FN_STMT:{
    ScopeFn* s2 = (ScopeFn*)s;
    compile_fn_stmt(state, s2);
    break;
  }
  case SEQ_STMT:{
    ScopeSeq* s2 = (ScopeSeq*)s;
    emit_scopestmt(state, s2->a);
    emit_scopestmt(state, s2->b);
    break;
  }
  case EXP_STMT:{
    ScopeExp* s2 = (ScopeExp*)s;
    emit_exp(state, s2->exp);

    break;
  }
  default:
    printf("Unrecognized scope statement with tag %d\n", s->tag);
    exit(-1);
  }

}

Program* compile (ScopeStmt* stmt) {
  /*printf("Compiling Program:\n");*/
  init();
  CompileState* compile_state = compile_state_init();
  int entry_index = compile_program(compile_state, stmt);
  
  image->values = compile_state->constants;
  image->slots = compile_state->globals;
  image->entry = entry_index;
  /*printf("\n");*/
  
  return image;
}



