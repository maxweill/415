/**********************************************
        CS415  Project 2
        Spring  2015
        Student Version
**********************************************/

#ifndef ATTR_H
#define ATTR_H

typedef struct {int num; char *str;} tokentype;

typedef enum type_expression {TYPE_INT=0, TYPE_BOOL, TYPE_ERROR} Type_Expression;

typedef struct {
        Type_Expression type;
        int targetRegister;
		int val;
        } regInfo;

typedef struct{
	Type_Expression type;
	int size;
	int flag;
	}stypeInfo;

typedef struct varInfo {
	char id[50];
	struct varInfo* node;
	}varInfo;

typedef struct{
	int headLabel;
	int footLabel;
	}whileInfo;

typedef struct{
	int trueLabel;
	int falseLabel;
	int endLabel;
	}ifInfo;

#endif


  
