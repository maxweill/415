/**********************************************
        CS415  Project 3
        Spring  2018
        Student Version
**********************************************/

#ifndef VALNUM_H
#define VALNUM_H

#include <string.h>

/* INSERT WHATEVER YOU NEED FOR THE VALUE NUMBER HASH FUNCTION */
extern
void init();
extern
int search(char *str);
extern
int hash(char *str);
extern
int ins(char *str, int reg);
extern
int del(char *str);

typedef struct CSE
{
int targetRegister;
char id[300];
struct CSE* next;
}CSE;

typedef struct hashTable
{
	int size;
	struct CSE **table;
}hashTable;
#endif
