%{
#include <stdio.h>
#include "attr.h"
#include "instrutil.h"
int yylex();
void yyerror(char * s);
#include "symtab.h"
void insertToEnd(varInfo* h, char* name);
FILE *outfile;
char *CommentBuffer;
 
%}

%union {tokentype token;
        regInfo targetReg;
		stypeInfo typeinfo;
		varInfo	*varinfo;
		whileInfo whileinfo;
		ifInfo ifinfo;
       }

%token PROG PERIOD VAR 
%token INT BOOL PRINT THEN IF DO  
%token ARRAY OF 
%token BEG END ASG  
%token EQ NEQ LT LEQ GT GEQ AND OR TRUE FALSE
%token ELSE
%token WHILE 
%token <token> ID ICONST 

%type <targetReg> exp condexp
%type <targetReg> lhs 
%type <typeinfo> stype type vardcl
%type <varinfo> idlist
%type <whileinfo> WHILE wstmt;
%type <ifinfo> ifhead;
%start program

%nonassoc EQ NEQ LT LEQ GT GEQ 
%left '+' '-' AND
%left '*' OR

%nonassoc THEN
%nonassoc ELSE

%%
program : {emitComment("Assign STATIC_AREA_ADDRESS to register \"r0\"");
           emit(NOLABEL, LOADI, STATIC_AREA_ADDRESS, 0, EMPTY);} 
           PROG ID ';' block PERIOD { }
	;

block	: variables cmpdstmt { }
	;

variables: /* empty */
	| VAR vardcls { }
	;

vardcls	: vardcls vardcl ';' { }
	| vardcl ';' { }
	| error ';' { yyerror("***Error: illegal variable declaration\n");}  
	;

vardcl	: idlist ':' type {	

varInfo* curr = $1;
while(curr != NULL)
{
	int num=NextOffset($3.size);
	insert(curr->id,$3.type,num,$3.flag);
	//printf("%i",num);
	
	curr=curr->node;
} 

}
	;

idlist	: idlist ',' ID { 

varInfo *temp=$1;
insertToEnd(temp,$3.str);
							$$=$1; }

        | ID		{ varInfo* head = malloc(sizeof(varInfo));
						strcpy(head->id,$1.str);
						$$ = head;} 
	;


type	: ARRAY '[' ICONST ']' OF stype { $$.type = $6.type; $$.size = $3.num; $$.flag=1; }

        | stype { $$.type = $1.type; 
					$$.size = 1;
					$$.flag = 0;}
	;

stype	: INT { $$.type = TYPE_INT; }
        | BOOL { $$.type = TYPE_BOOL;}
	;

stmtlist : stmtlist ';' stmt { }
	| stmt { }
        | error { yyerror("***Error: ';' expected or illegal statement \n");}
	;

stmt    : ifstmt { }
	| wstmt { }
	| astmt { }
	| writestmt { }
	| cmpdstmt { }
	;

cmpdstmt: BEG stmtlist END { }
	;

ifstmt :  ifhead 
          THEN stmt { 	emit(NOLABEL, BR, $1.endLabel, EMPTY, EMPTY);
						emit($1.falseLabel, NOP, EMPTY, EMPTY, EMPTY); 	}

  	  ELSE 
          stmt { emit($1.endLabel, NOP, EMPTY, EMPTY, EMPTY); }
	;

ifhead : IF condexp { int lab1 = NextLabel();
						int lab2 = NextLabel();
						$$.trueLabel = lab1;
						$$.falseLabel = lab2;
						$$.endLabel = NextLabel();
						emit(NOLABEL, CBR, $2.targetRegister, lab1, lab2);
						emit(lab1, NOP, EMPTY, EMPTY, EMPTY);
 }
        ;

writestmt: PRINT '(' exp ')' { int printOffset = -4; /* default location for printing */
  	                         sprintf(CommentBuffer, "Code for \"PRINT\" from offset %d", printOffset);
	                         emitComment(CommentBuffer);
                                 emit(NOLABEL, STOREAI, $3.targetRegister, 0, printOffset);
                                 emit(NOLABEL, 
                                      OUTPUTAI, 
                                      0,
                                      printOffset, 
                                      EMPTY);
                               }
	;

wstmt	: WHILE  { $<whileinfo>$.headLabel = NextLabel();
					$<whileinfo>$.footLabel = NextLabel();
					emit($<whileinfo>$.headLabel, NOP, EMPTY, EMPTY, EMPTY); } 
          
		condexp { int bodyLabel = NextLabel(); 
							
					emit(NOLABEL, CBR, $3.targetRegister, bodyLabel, $<whileinfo>2.footLabel);
					emit (bodyLabel, NOP, EMPTY, EMPTY, EMPTY);   } 

          DO stmt  { emit(NOLABEL, BR, $<whileinfo>2.headLabel, EMPTY, EMPTY);
					emit($<whileinfo>2.footLabel, NOP, EMPTY, EMPTY, EMPTY);
 } 
	;


astmt : lhs ASG exp             { 
 				  if (! ((($1.type == TYPE_INT) && ($3.type == TYPE_INT)) || 
				         (($1.type == TYPE_BOOL) && ($3.type == TYPE_BOOL)))) {
				    printf("\n*** ERROR ***: Assignment types do not match.\n");
				  }

				  emit(NOLABEL,
                                       STORE, 
                                       $3.targetRegister,
                                       $1.targetRegister,
                                       EMPTY);
                                }
	;

lhs	: ID			{
                                 	int newReg1 = NextRegister();
                                	int newReg2 = NextRegister();
                                  	SymTabEntry *var = lookup($1.str); 
				  					if(var!=NULL)
									{
										int ofst = var->offset;
                                  		$$.targetRegister = newReg2;
                                  		$$.type = var->type;
				 						emit(NOLABEL, LOADI, ofst, newReg1, EMPTY);
				 						emit(NOLABEL, ADD, 0, newReg1, newReg2);

										if(var->flag==1)
										{
										printf("\n*** ERROR ***: Variable %s is not a scalar variable.\n", $1.str); 
										}
									}
									else
									{
										printf("\n*** ERROR ***: Variable %s not declared.\n",$1.str);									
									}

									
				  
                         	  }

/* STUFF TO DO*/
                                |  ID '[' exp ']' {  

								SymTabEntry *var = lookup($1.str);

								if(var!=NULL)
								{
										int reg1 = NextRegister();
								int reg2 = NextRegister();
								
								
								

								$$.type = var->type; 
								$$.targetRegister = reg1;
								emit(NOLABEL, LOADI, 4, reg2, EMPTY);
								int reg3 = NextRegister();
								emit(NOLABEL, MULT, $3.targetRegister, reg2, reg3);
								int reg4 = NextRegister();
								emit(NOLABEL, LOADI, var->offset , reg4, EMPTY);
								int reg5 = NextRegister();
								emit(NOLABEL, ADD, reg3 , reg4, reg5 );
								emit(NOLABEL, ADD, 0 , reg5, reg1);
										if(var->flag == 0)
								{
										printf("\n*** ERROR ***: Variable %s is not an array variable.\n", $1.str); 
								}
								}
								else
								{
									printf("\n*** ERROR ***: Variable %s not declared.\n",$1.str);								
								}
								if($3.type != TYPE_INT)
								{
										printf("\n*** ERROR ***: Array variable %s index type must be integer.\n", $1.str); 
								}
								
								
								
}
                                

;


exp	: exp '+' exp		{ int newReg = NextRegister();

                                  if (! (($1.type == TYPE_INT) && ($3.type == TYPE_INT))) {
    				    printf("\n*** ERROR ***: Operand type must be integer.\n"); 
                                  }
                                  $$.type = $1.type;

                                  $$.targetRegister = newReg;
                                  emit(NOLABEL, 
                                       ADD, 
                                       $1.targetRegister, 
                                       $3.targetRegister, 
                                       newReg);
                                }

        | exp '-' exp		{ int newReg = NextRegister();

                                  if (! (($1.type == TYPE_INT) && ($3.type == TYPE_INT))) {
    				    printf("\n*** ERROR ***: Operand type must be integer.\n"); 
                                  }
                                  $$.type = $1.type;

                                  $$.targetRegister = newReg;
                                  emit(NOLABEL, 
                                       SUB, 
                                       $1.targetRegister, 
                                       $3.targetRegister, 
                                       newReg); }

        | exp '*' exp		{ int newReg = NextRegister();

                                  if (! (($1.type == TYPE_INT) && ($3.type == TYPE_INT))) {
    				    printf("\n*** ERROR ***: Operand type must be integer.\n"); 
                                  }
                                  $$.type = $1.type;

                                  $$.targetRegister = newReg;
                                  emit(NOLABEL, 
                                       MULT, 
                                       $1.targetRegister, 
                                       $3.targetRegister, 
                                       newReg); }

        | exp AND exp		{ 
		
						int newReg = NextRegister();				
						if (! (($1.type == TYPE_BOOL) && ($3.type == TYPE_BOOL))) {
    				    printf("\n*** ERROR ***: Operand type must be boolean.\n"); }
								
								$$.type = $1.type;
								$$.targetRegister=newReg;
								emit(NOLABEL, 
                                       AND_INSTR, 
                                       $1.targetRegister, 
                                       $3.targetRegister, 
                                       newReg); 

								
							
							 } 


        | exp OR exp       	{ int newReg = NextRegister();				
						if (! (($1.type == TYPE_BOOL) && ($3.type == TYPE_BOOL))) {
    				    printf("\n*** ERROR ***: Operand type must be boolean.\n"); }
								
								$$.type = $1.type;
								$$.targetRegister=newReg;
								emit(NOLABEL, 
                                       OR_INSTR, 
                                       $1.targetRegister, 
                                       $3.targetRegister, 
                                       newReg);  }


        | ID			{ 
	                        int newReg = NextRegister();				
							 SymTabEntry *var = lookup($1.str); 
				  					if(var!=NULL)
									{
										int offset = var->offset;
	                        			$$.targetRegister = newReg;
				  						$$.type = var->type;
				  						emit(NOLABEL, LOADAI, 0, offset, newReg);
									if(var->flag==1)
									{
										printf("\n*** ERROR ***: Variable %s is not a scalar variable.\n", $1.str); 
									}
										
									}
									else
									{
										printf("\n*** ERROR ***: Variable %s not declared.\n",$1.str);									
									}
									
                                  
	                        }
/* STUFF TO DO*/
        | ID '[' exp ']'	{  
								

								SymTabEntry *var = lookup($1.str);
								if(var!=NULL)
									{int reg1 = NextRegister();
								int reg2 = NextRegister();
							
								$$.type = var->type; 
								$$.targetRegister = reg1;
								emit(NOLABEL, LOADI, 4, reg2, EMPTY);
								int reg3 = NextRegister();
								emit(NOLABEL, MULT, $3.targetRegister, reg2, reg3);
								int reg4 = NextRegister();
								emit(NOLABEL, LOADI, var->offset , reg4, EMPTY);
								int reg5 = NextRegister();

								emit(NOLABEL, ADD, reg3 , reg4, reg5 );
								emit(NOLABEL, LOADAO, 0 , reg5, reg1);
										if(var->flag == 0)
								{
										printf("\n*** ERROR ***: Variable %s is not an array variable.\n", $1.str); 
								}
									}
								else
								{
									printf("\n*** ERROR ***: Variable %s not declared.\n",$1.str);						
								}
								if($3.type != TYPE_INT)
								{
									printf("\n*** ERROR ***: Array variable %s index type must be integer.\n", $1.str); 
								}
								
								


							}
 


	| ICONST                 { int newReg = NextRegister();
	                           $$.targetRegister = newReg;
				   $$.type = TYPE_INT;
				   emit(NOLABEL, LOADI, $1.num, newReg, EMPTY); }

        | TRUE                   { int newReg = NextRegister(); /* TRUE is encoded as value '1' */
	                           $$.targetRegister = newReg;
				   $$.type = TYPE_BOOL;
				   emit(NOLABEL, LOADI, 1, newReg, EMPTY); }

        | FALSE                   { int newReg = NextRegister(); /* TRUE is encoded as value '0' */
	                           $$.targetRegister = newReg;
				   $$.type = TYPE_BOOL;
				   emit(NOLABEL, LOADI, 0, newReg, EMPTY); }

	| error { yyerror("***Error: illegal expression\n");}  
	;


condexp	: exp NEQ exp		{ int newReg = NextRegister();
								if (! ($1.type == $3.type)) 
								{
									printf("\n*** ERROR ***: == or != operator with different types.\n"); 								
								}
								$$.type=$3.type;
								$$.targetRegister=newReg;
								emit(NOLABEL, CMPNE, $1.targetRegister , $3.targetRegister ,newReg);
 } 

        | exp EQ exp		{ int newReg = NextRegister();
								if (! ($1.type == $3.type)) 
								{
									printf("\n*** ERROR ***: == or != operator with different types.\n"); 								
								}
								$$.type=$3.type;
								$$.targetRegister=newReg;
								emit(NOLABEL, CMPEQ, $1.targetRegister , $3.targetRegister ,newReg); } 

        | exp LT exp		{ int newReg = NextRegister();
								if (! ($1.type == $3.type)&& $1.type == TYPE_INT) 
								{
									printf("\n*** ERROR ***: Relational operator with illegal type.\n"); 							
								}
								$$.type=$3.type;
								$$.targetRegister=newReg;
								emit(NOLABEL, CMPLT, $1.targetRegister , $3.targetRegister ,newReg); }

        | exp LEQ exp		{ int newReg = NextRegister();
								if (! ($1.type == $3.type) && $1.type == TYPE_INT) 
								{
									printf("\n*** ERROR ***: Relational operator with illegal type.\n"); 								
								}
								$$.type=$3.type;
								$$.targetRegister=newReg;
								emit(NOLABEL, CMPLE, $1.targetRegister , $3.targetRegister ,newReg); }

	| exp GT exp		{ int newReg = NextRegister();
								if (! ($1.type == $3.type) && $1.type == TYPE_INT) 
								{
									printf("\n*** ERROR ***: Relational operator with illegal type.\n"); 								
								}
								$$.type=$3.type;
								$$.targetRegister=newReg;
								emit(NOLABEL, CMPGT, $1.targetRegister , $3.targetRegister ,newReg); }

	| exp GEQ exp		{ int newReg = NextRegister();
								if (!($1.type == $3.type) && $1.type == TYPE_INT) 
								{
									printf("\n*** ERROR ***: Relational operator with illegal type.\n"); 						
								}
								$$.type=$3.type;
								$$.targetRegister=newReg;
								emit(NOLABEL, CMPGE, $1.targetRegister , $3.targetRegister ,newReg); }

	| error { yyerror("***Error: illegal conditional expression\n");}  
        ;

%%

void insertToEnd(varInfo* h, char* name)
{
	varInfo* new = malloc(sizeof(varInfo));
	strcpy(new->id,name);
	varInfo* curr=h;
	while(curr->node != NULL)
	{
		curr=curr->node;
	}
	curr->node = new;
}

void yyerror(char* s) {
        fprintf(stderr,"%s\n",s);
        }


int
main(int argc, char* argv[]) {

  printf("\n     CS415 Spring 2018 Compiler\n\n");

  outfile = fopen("iloc.out", "w");
  if (outfile == NULL) { 
    printf("ERROR: cannot open output file \"iloc.out\".\n");
    return -1;
  }

  CommentBuffer = (char *) malloc(650);  
  InitSymbolTable();

  printf("1\t");
  yyparse();
  printf("\n");

  PrintSymbolTable();
  
  fclose(outfile);
  
  return 1;
}
