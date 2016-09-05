#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "utils.h"
#include "ast.h"
#include "compiler.h"
#include "bytecode.h"



/*
  INT_VAL,
  NULL_VAL,
  STRING_VAL,
  METHOD_VAL,
  SLOT_VAL,
  CLASS_VAL
*/

Program* image ;
int   const_index;





void init(){
	image = (Program*)malloc(sizeof(Program));
	const_index = 0;
}

Value* lookup_ref(Vector* vec, char* value){
	Value* v ;
	for(int i=0; i< vec->size; i++){
		v = vector_get(vec, i);
		if(v->tag == STRING_VAL){
		   StringValue* s = (StringValue*) v;
		   if(s->value == value){
			   return s;
		   }
		}
	}
}


int gen_id(char* name ){
	StringValue* n = (StringValue*)allocate_obj(STRING_VAL);
    n->value = name;
    index = add_val(image->values, n);
    return index;
   }
   
int gen_literal(IntExp* num){
	IntValue* val = (IntValue*)allocate_obj(INT_VAL);
	val->value = num->value;
	return add_val(image->values, val);	
}

void gen_var_stmt(MethodValue* parent, MethodValue* method, ScopeVar* stmt){
	emit_exp(parent, method,  stmt->exp);
	StringValue* name = (StringValue*)allocate_obj(STRING_VAL);
	name->value = stmt->name;
	int index = add_value(image->values, name);
	if(parent != NULL){
		method->locals++;
	}else{
		vector_add(method
	}
	SlotValue* slot = (SlotValue*)allocate_obj(SLOT_VAL);
		
}

int  gen_slot(int index){
	SlotValue* slot = malloc(sizeof(SlotValue));
	slot->name = index;
	return add_value(image->values, slot);
	
}

ByteIns* gen_set_local_ins(int id){
	SetLocalIns* setins = malloc(sizeof(SetLocalIns)); 
	setins->tag = SET_LOCAL_OP;
	setins->idx = id;
	return setins;
}

ByteIns* gen_set_global_ins(int id, ){
	SetGlobalIns* setins = malloc(sizeof(SetGlobalIns));
	setins->tag = SET_GLOBAL_OP;
	setins->name = id;
	return setins;
	
}

ByteIns* gen_get_local_ins(int localindex){
	ByteIns* res;
	GetLocalIns* ins = malloc(sizeof(GetLocalIns));
	ins->tag = GET_LOCAL_OP;
	ins->idx = localindex;
	res = ins;
	return res ;
	
}

ByteIns* gen_lit_ins( int index){
    LitIns* lit  = malloc(sizeof(LitIns));
	lit->tag = LIT_OP;
	lit->index = index;
	return lit;
	
}

int add_val( Vector vec*, Value* val){
	int index;
	vector_add(vec->values, val);
	index = vec->values->size-1;
	return index;

}

void* allocate_obj(ValTag tag){
	Value* res ;
	switch(tag){
		case INT_VAL: {
			IntValue* obj = (IntValue*)malloc(sizeof(IntValue));
			obj->tag = INT_VAL;
			obj->val = 0;
			res = obj;
		}
		case NULL_VAL:{
			res = NULL;
			
		}
		case STRING_VAL:{
			StringValue* obj = (StringValue*)malloc(sizeof(StringValue));
			obj->tag = STRING_VAL;
			res = obj;
		}
		case METHOD_VAL:{
			MethodValue* obj = (MethodValue*)malloc(sizeof(MethodValue));
			obj->tag = METHOD_VAL;
			obj->name = 0;
			obj->nargs = 0;
			obj->nlocals = 0;
			obj->code = make_vector();
			res = obj;
			
		}
		case SLOT_VAL:{
			SlotValue* obj = (SlotValue*)malloc(sizeof(SlotValue));
			obj->tag = SLOT_VAL;
			res = obj;
		}
		case CLASS_VAL:{
			ClassValue* obj = (ClassValue*)malloc(sizeof(ClassValue));
			obj->tag = CLASS_VAL;
			res = obj;
		}
	}
	return res;
}

void emit_exp ( MethodValue* parent, MethodValue* env, Exp* e) {
  ByteIns* res;
  switch(e->tag){
  case INT_EXP:{
	IntExp* e2 = (IntExp*)e;
    index = gen_literal(e2)
    vector_add(env->code, gen_lit_ins(index));
    break;
  }
  case NULL_EXP:{
    printf("null");
    break;
  }
  case PRINTF_EXP:{
    PrintfExp* e2 = (PrintfExp*)e;
    printf("printf(");
    print_string(e2->format);
    for(int i=0; i<e2->nexps; i++){
      printf(", ");
      print_exp(e2->exps[i]);
    }
    printf(")");
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
    printf("%s(", e2->name);
    for(int i=0; i<e2->nargs; i++){
      if(i > 0) printf(", ");
      print_exp(e2->args[i]);
    }
    printf(")");
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
    printf("%s", e2->name);
    ByteIns* ins = gen_get_local_ins(localindex);
    
    break;
  }
  default:
    printf("Unrecognized Expression with tag %d\n", e->tag);
    exit(-1);
  }
  return res;
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


void emit_scopestmt (ScopeStmt* s, MethodValue* parent,  MethodValue* env) {
  switch(s->tag){
  case VAR_STMT:{
    ScopeVar* s2 = (ScopeVar*)s;
    emit_exp(parent, method, s2->exp);

   
    if(parent != NULL){
		int localindex = env->nlocals++;
        gen_set_local_ins(localindex);
        
	else{
		int id = gen_id(s2->name);
		gen_set_global_ins(id);
		gen_slot(id);
	}
	
	

	
	emit_drop(env);


    break;
  }
  case FN_STMT:{
    ScopeFn* s2 = (ScopeFn*)s;
    MethodValue* fn = malloc(sizeof(MethodValue));
    emit_scopestmt(method, fn, s2->body);
    fn->name = gen_id(s2->name);
    fn->nargs = s2->nargs;
    


    break;
  }
  case SEQ_STMT:{
    ScopeSeq* s2 = (ScopeSeq*)s;
    emit_scopestmt(env, s2->a);
    emit_scopestmt(env, s2->b);
    break;
  }
  case EXP_STMT:{
    ScopeExp* s2 = (ScopeExp*)s;
    ByteIns ins = emit_exp(parent, env, s2->exp);
    vector_add(env->code, ins);
    break;
  }
  default:
    printf("Unrecognized scope statement with tag %d\n", s->tag);
    exit(-1);
  }

}


Program* compile (ScopeStmt* stmt) {
  printf("Compiling Program:\n");
  StringValue* name = allocate_obj(STRING_VAL);
  name->value = "entry";
  MethodValue* method = allocate_obj(METHOD_VAL);
  
  emit_scopestmt(stmt, method);
  printf("\n");
  exit(-1);
  return image;
}



