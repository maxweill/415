/**********************************************
        CS415  Project 3
        Spring  2018
        Student Version
**********************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "instrutil.h"
#include "valnum.h"
hashTable *hashtable;

void init()
{
	hashtable = malloc(sizeof(struct hashTable));
	hashtable->table = malloc( sizeof(CSE *) * 1024);
	int i;	
	for (i=0;i<1024;i++)
	{
		hashtable->table[i]=NULL;
	}
	hashtable->size = 1024;
	return;
}
int search(char *str)
{
	int res = hash(str);
	if (hashtable->table[res]==NULL)
	{
		return -1;
	}
	else
	{		
		CSE* curr = hashtable->table[res];
		while(curr!=NULL)
		{
			if(strcmp(curr->id,str)==0)
			{
				return curr->targetRegister;
			}
			curr = curr->next;
		}
		return -1;
	}
	
	
}
int hash(char *str) //djb2 hash
{
        unsigned long hash = 5381;
        int c;
        while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        return hash%1024;
}
int ins(char *str, int reg)
{
	int res = hash(str);
	
	if(hashtable->table[res]==NULL)
	{
		hashtable->table[res] = malloc(sizeof(CSE));
		hashtable->table[res]->targetRegister = reg;
		hashtable->table[res]->next = NULL;
		strcpy(hashtable->table[res]->id,str);
		//emitComment("First entry of this CSE.");
	}
	else
	{
		CSE* curr = hashtable->table[res];
		while(curr->next != NULL)
		{
			curr = curr->next;
		}
		curr->next = malloc(sizeof(CSE));
		curr->next->targetRegister = reg;
		curr->next->next = NULL;
		strcpy(curr->next->id,str);
		//emitComment("COLLISION: Adding to end of list.");
	}
	
	
	return 0;
}
int edit(char *str, int reg)
{
		int res = hash(str);
	if (hashtable->table[res]==NULL)
	{
		return -1;
	}
	else
	{		
		CSE* curr = hashtable->table[res];
		while(curr!=NULL)
		{
			if(strcmp(curr->id,str)==0)
			{
				curr->targetRegister = reg;
				return reg;
			}
			curr = curr->next;
		}
		return -1;
	}

}
