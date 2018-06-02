#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int numreg=0;
int maxreg=0;
int globaloffset=-4;
struct Instruction
	{
		char* op;
		char* arg1; //every op has at least 1 register
		char* arg2; //if only 1 register, will be filled with "XX"
		char* arg3; //if only 1 or 2 registers, will be filled with "XX"strcpy(node.arg2,curr);
		
		struct Instruction* next;
		struct Instruction* prev;
		
		int line;
		int maxlife;
		char* registers;
		int pattern;
		
	}Instruction;
	
struct Regist
	{
		char* name;
		int namenum;
		int LRlow;
		int LRhigh;
		int occ;
		int occreg[1000];
		int offset;
		int spilled;
		int spillnum;
		int beenseen;
		
	}Regist;
	
	
void RemoveSpaces(char* source)
{
	while (isspace(*source))
    ++source;
  char* i = source;
  char* j = source;
  while(*j != 0)
  {
    *i = *j++;
    if(*i != ' ' || *i != '\t')
      i++;
  }
  *i = 0;
}




struct Instruction fillInstruction(char* buffer,int line)
{
	struct Instruction node;
	char *curr = strtok(buffer," \t");
	node.pattern=-1;
	node.op = malloc(10);
	node.arg1 = malloc(100);
	node.arg2 = malloc(100);
	node.arg3 = malloc(100);
	node.line = line;
	strcpy(node.op,curr);

	if(!strcasecmp(node.op,"loadI") || 
		!strcasecmp(node.op,"store")|| 
		!strcasecmp(node.op,"load"))
	{
		node.pattern = 0;
	}
	
	if(!strcasecmp(node.op,"storeAI"))
	{
		node.pattern = 1;
	}
	
	if(!strcasecmp(node.op,"add") || 
		!strcasecmp(node.op,"sub") ||
		!strcasecmp(node.op,"mult")||
		!strcasecmp(node.op,"lshift")||
		!strcasecmp(node.op,"rshift")||
		!strcasecmp(node.op,"loadAI"))
	{
		node.pattern = 2;
	}
	
	if(!strcasecmp(node.op,"output"))
	{
		node.pattern = 3;
	}
	
	if(!strcasecmp(node.op,"nop\n"))
	{
		node.pattern = 4;
	}
	
	
	//op arg =>arg | pattern 0
	if(node.pattern==0)
	{
		curr = strtok(NULL, " \t=");
		RemoveSpaces(curr);
		strcpy(node.arg1,curr);
		
		curr = strtok(NULL,"\n");
		curr = strchr(curr,'r');
		RemoveSpaces(curr);
		strcpy(node.arg2,curr);
		
		strcpy(node.arg3,"XX");
		
		node.pattern = 0;
	}
	
	//op reg => reg i | pattern 1
	if(node.pattern ==1)
	{
		curr = strtok(NULL, " \t=");
		RemoveSpaces(curr);
		strcpy(node.arg1,curr);
		
		curr = strtok(NULL,"\n");	
		curr = strchr(curr,'r');
		curr = strtok(curr,",");
		RemoveSpaces(curr);
		strcpy(node.arg2,curr);
		
		curr = strtok(NULL,"\n");
		RemoveSpaces(curr);	
		strcpy(node.arg3,curr);

		node.pattern =1;
	}
	
	//op reg,reg => reg | pattern 2
	if(node.pattern==2)
	{
		curr = strtok(NULL, " ,");
		RemoveSpaces(curr);
		strcpy(node.arg1,curr);
		
		curr = strtok(NULL, " \t=");
		RemoveSpaces(curr);
		strcpy(node.arg2,curr);
		
		curr = strtok(NULL,"\n");
		curr = strchr(curr,'r');
		RemoveSpaces(curr);
		strcpy(node.arg3,curr);
		
		node.pattern = 2;
	}
	
	if(node.pattern==3)
	{
		curr = strtok(NULL, "\n");
		RemoveSpaces(curr);
		strcpy(node.arg1,curr);
		
		strcpy(node.arg2,"XX");
		
		strcpy(node.arg3,"XX");
		
		node.pattern = 3;
	}
	
	if(node.pattern==4)
	{
		strcpy(node.arg1,"XX");
		strcpy(node.arg2,"XX");
		strcpy(node.arg3,"XX");
		node.pattern = 4;
	}
	return node;
}
void printTable(struct Regist* array)
{
	int i;
	printf("Name\t Namenum\t Occ\t Start\t Finish\t Spilled Spillnum\t Usage\n");
	for(i=0;i<numreg;i++)
	{
		printf("%s\t %i\t %i\t %i\t %i\t %i\t %i\t[",array[i].name,array[i].namenum,array[i].occ,array[i].LRlow,array[i].LRhigh,array[i].spilled,array[i].offset);
		int j;
		for(j = 1; j<=array[i].occreg[0];j++)
		{
			printf("%i,",array[i].occreg[j]);
		}
		printf("]\n");
	}
	
}



int* calculateMaxLive(struct Regist* array, int* output)
{
	int i;
	int minstrt=array[0].LRlow;
	int maxleave=array[0].LRhigh;
	
	for(i=1;i<numreg;i++)
	{
		if(array[i].LRlow < minstrt)
		{
			minstrt=array[i].LRlow;
		}
		if(array[i].LRhigh > maxleave)
		{
			maxleave=array[i].LRhigh;
		}
	}
	//printf("Checking from lines %i to %i\n",minstrt,maxleave);
	int count[maxleave];
	memset(count,0,sizeof(int)*maxleave);
	
	int j;
	for(i=0;i<numreg;i++)
	{
		for(j=array[i].LRlow;j<=array[i].LRhigh;j++)
		{
			if (array[i].spilled==0)
			{
			count[j]++;
			}
			//printf("Register[%s]: %i\n",array[i].name,count[i]);
		}
	}
	
	output[0]=0;	//linenum
	output[1]=0;	//maxlive
	
	for(i=minstrt;i<maxleave;i++)
	{
		
		//printf("Line %i : Live reg => %i\n",i,count[i]);
		if(count[i]>output[1])
		{
			output[1]=count[i];
			output[0]=i;
		}
	}

	return output;
	
}


void printCode(struct Instruction* head)
{
	int i=0;
	while(head != NULL)
	{
		//printf("Line #%i ",head->line);
		//printf("%s\n",head->op);
			struct Instruction* node = head;
		
			if(node->pattern ==0) 
			{
				printf("%s %s => %s\n",node->op,node->arg1,node->arg2);
			}
			else if(node->pattern ==1)
			{
				printf("%s %s => %s, %s\n",node->op,node->arg1,node->arg2,node->arg3);
			}
			else if(node->pattern ==2)
			{
				printf("%s %s, %s => %s\n",node->op,node->arg1,node->arg2,node->arg3);
			}
			else if(node->pattern ==3)
			{
				printf("%s %s\n",node->op,node->arg1);
			}
			else if(node->pattern ==4)
			{
				printf("%s",node->op);
			}
			else
			{
				printf("THERE SHOULD BE AN INSTRUCTION HERE :)\n");
			}
			head = head->next;
			i++;
	}
	
}

void printCodeBack(struct Instruction* head)
{
	int i=0;
	while(head != NULL)
	{
		printf("Line #%i ",i);
			struct Instruction* node = head;
		
			if(node->pattern ==0) 
			{
				printf("%s %s => %s\n",node->op,node->arg1,node->arg2);
			}
			else if(node->pattern ==1)
			{
				printf("%s %s => %s, %s\n",node->op,node->arg1,node->arg2,node->arg3);
			}
			else if(node->pattern ==2)
			{
				printf("%s %s, %s => %s\n",node->op,node->arg1,node->arg2,node->arg3);
			}
			else if(node->pattern ==3)
			{
				printf("%s %s\n",node->op,node->arg1);
			}
			else if(node->pattern ==4)
			{
				printf("%s",node->op);
			}
			else
			{
				printf("THERE SHOULD BE AN INSTRUCTION HERE :)\n");
			}
			head = head->prev;
			i++;
	}
	
}

void fillRegister(struct Regist* array, char* name, int line)
{
	int added=0;
	int i;
	if(!strcmp(name,"r0"))
	{
		return;
	}
	
	for(i=0;i<numreg;i++)
	{
		//printf("Does %s == %s?\n",array[i].name,name);
		if(!(strcmp(array[i].name,name)))
		{
			array[i].occ++;
			array[i].occreg[array[i].occ]=line;
			array[i].occreg[0]++;
			array[i].LRhigh = line-1;
			added=1;
			//printf("Updating array.\n");
			return;
		}
		
	}
	//printf("Adding |%s| array.\n",name);
	if(added==0)
	{
		array[numreg].name = malloc(100);
		strcpy(array[numreg].name,name);
		char * temp = array[numreg].name+1;
		array[numreg].namenum = atoi(temp);
		array[numreg].LRhigh = line;
		array[numreg].LRlow = line;
		array[numreg].occ=1;
		array[numreg].occreg[1] = line;
		array[numreg].occreg[0] = 1;
		array[numreg].spillnum = 0;
		array[numreg].offset = 0;
		array[numreg].beenseen = 0;
		array[numreg].spilled=0;
		numreg++;
	}

}

int compareOcc(const void* a,const void* b)
{
	struct Regist *left= (struct Regist*)a;
	struct Regist *right= (struct Regist*)b;
	//printf("%i - %i\n",left->occ,right->occ);
	return right->occ - left->occ;
}

int compareOccTie(const void* a,const void* b)
{
	struct Regist *left= (struct Regist*)a;
	struct Regist *right= (struct Regist*)b;
	//printf("%i - %i\n",left->occ,right->occ);
	if(right->occ == left->occ)
	{
		return (right->LRhigh - right->LRlow) - (left->LRhigh - right->LRlow);
	}
	return right->occ - left->occ;
}
int compareStarts(const void* a,const void* b)
{
	struct Regist *left= (struct Regist*)a;
	struct Regist *right= (struct Regist*)b;
	//printf("%i - %i\n",left->occ,right->occ);

	return left->occ -  right->occ;
}

int isPhysical(struct Regist* array, char* test)
{
	int i;
	//printf("%s\n",test);
	for(i=0;i<maxreg-2;i++)
	{
		if(!(strcmp(array[i].name, test)))
		{
			return array[i].spillnum;
		}
	}
	return 0;
}
int isAllocated(struct Regist* array, char* test)
{
	int i;
	for(i=0;i<numreg;i++)
	{
		if(!(strcmp(array[i].name,test)))
		{
			return array[i].spillnum;
		}
	}
	return 0;
}

int beenSeen(struct Regist* array, char*test)
{
	int i;
	for(i=0;i<numreg;i++)
	{
		if(!(strcmp(array[i].name, test)))
		{
			if(array[i].beenseen==0)
				{
					array[i].beenseen++;
					return 1;
				}
			if(array[i].beenseen>0)
				{
					array[i].beenseen++;
					return 2;
				}
		}
	}
	return 0;
}

char f1[100];
char f2[100];
int f1use=0;
int f2use=0;
struct Instruction* insertStore(struct Instruction* curr,struct Regist* array)
{
	struct Instruction* store = malloc(sizeof(Instruction));
	
	store->op = "storeAI";
	store->arg1 = malloc(100);
	strcpy(store->arg1,f1);
	store->arg2 = "r0";
	char offset[100];
	sprintf(offset,"%i",globaloffset);
	
	
	
	globaloffset-=4;
	
	store->arg3 = malloc(100);
	strcpy(store->arg3,offset);
	store->pattern =1;
	if(!strcmp(curr->arg3,"XX"))
	{
		int i;
		for(i=0;i<numreg;i++)
		{
			if(!(strcmp(array[i].name, curr->arg2)))
			{
				array[i].offset=globaloffset+4;
			}
		}
		
		//printf("Replacing %s with %s",curr->arg2,f1);
		strcpy(curr->arg2,f1);
	}
	else
	{
		int i;
		for(i=0;i<numreg;i++)
		{
			if(!(strcmp(array[i].name, curr->arg3)))
			{
				array[i].offset=globaloffset+4;
			}
		}
		//printf("Replacing %s with %s",curr->arg3,f1);
		strcpy(curr->arg3,f1);
	} 

	store->next = curr->next; 
	store->prev = curr;
	
	curr->next = store;
	store->next->prev = store;
	
	/*
	next = curr->next;
	prev = curr;
	prev->next = store;
	
	curr = store;
	store->prev = prev;
	store->next = next;
	*/
	return curr;
}

struct Instruction* insertLoad(struct Instruction* curr, int a, struct Regist* array)
{	
	struct Instruction* head=curr;
	struct Instruction* load = malloc(sizeof(Instruction));
	
	if(a==1)
	{
		int i;
		for(i=0;i<numreg;i++)
		{
			if(!strcmp(array[i].name,curr->arg1))
			{
				//printf("###load into %s from r0,%i\n",f1,array[i].offset);
				strcpy(curr->arg1,f1);
				
				load->op = malloc(100);
				strcpy(load->op,"loadAI");
				load->arg1 = malloc(10);
				strcpy(load->arg1,"r0");
				load->arg2 = malloc(100);
				sprintf(load->arg2,"%i",array[i].offset);
				load->arg3 = malloc(100);
				strcpy(load->arg3,f1);
				load->pattern=2;
	
				break;
			}
		}
		
	}
	else if(a==2)
	{
		int i;
		for(i=0;i<numreg;i++)
		{
			
			if(!strcmp(array[i].name,curr->arg2))
			{
				
				strcpy(curr->arg2,f2);
				
				load->op = malloc(100);
				strcpy(load->op,"loadAI");
				load->arg1 = malloc(10);
				strcpy(load->arg1,"r0");
				load->arg2 = malloc(100);
				sprintf(load->arg2,"%i",array[i].offset);
				
				load->arg3 = malloc(100);
				strcpy(load->arg3,f2);
				load->pattern=2;
	
				break;
			}
		}
	}
	else
	{
		printf("Panic\n");
		return head;
	}
	
	load->next = curr;
	load->prev = curr->prev;
	load->prev->next = load;
	curr->prev = load;
	
	
	return head;
}

struct Regist* activeReg(struct Regist* array, struct Instruction* head,int line, int maxlive)
{
	struct Regist* output = malloc(sizeof(struct Regist) * 256);
	int i;
	int j=0;
	for(i=0;i<numreg;i++)
	{
		if(line>= array[i].LRlow && line<= array[i].LRhigh && array[i].spilled ==0)
		{
			output[j]=array[i];
			j++;
			//printf("%s\n",array[i].name);
		}
	}
	return output;
}

struct Instruction* simpleAllocator(struct Regist* array,struct Instruction* head)
{
	qsort(array,256,sizeof(Regist),compareOcc);
	int i;
	for(i=0;i<maxreg;i++)
	{
		if(i==maxreg-2)
		{
			sprintf(f2,"r%i",i+1);
		}
		else if(i==maxreg-1)
		{
			sprintf(f1,"r%i",i+1);
		}
		else
		{
			array[i].spillnum=i+1;
		}
	}
	//printTable(array);
	
	struct Instruction* temp = head;
	
	temp = temp->next;
	while(temp != NULL)
	{				
		if(!strcmp(temp->op,"storeAI"))
		{
			temp = temp->next;
		}
			
		if(strchr(temp->arg1,'r')!=NULL)
		{
			int spill = isPhysical(array,temp->arg1);
			if(spill)
			{
				char regname[100];
				sprintf(regname,"r%d",spill);
				strcpy(temp->arg1,regname);
			}
			else
			{
				int seen = beenSeen(array,temp->arg1);
				if(seen)
				{
					if(seen==1)
					{
						//printf("This should never happen\n");
						temp = insertStore(temp,array);
					}
					else
					{
						insertLoad(temp,1,array);
					}
				}
			}
		}
		
		if(strchr(temp->arg2,'r')!=NULL)
		{
			
	
			int spill = isPhysical(array,temp->arg2);
			if(spill)
				{
					
					char regname[100];
					sprintf(regname,"r%d",spill);
					strcpy(temp->arg2,regname);
				}
			else
			{
				
				int seen = beenSeen(array,temp->arg2);
				if(seen)
				{
					
					if(seen==1)
					{
						temp = insertStore(temp,array);
					}
					else
					{
						insertLoad(temp,2,array);
					}
				}
			}		
		}
		
		if(strchr(temp->arg3,'r')!=NULL)
		{
			int spill = isPhysical(array,temp->arg3);
			if(spill)
				{
					char regname[100];
					sprintf(regname,"r%d",spill);
					strcpy(temp->arg3,regname);
				}
			else
			{
				
				int seen = beenSeen(array,temp->arg3);
				if(seen)
				{
					if(seen==1)
					{
						temp = insertStore(temp,array);
					}
					else
					{
						insertLoad(temp,3,array);
					}
				}
			}
		}

		
		
		temp = temp->next;
	}
	
	printCode(head);
	return temp;
}

void spiller(struct Regist* array,struct Regist* active, int* maxlive)
{
	int i,j;
	int numtospill = maxlive[1] - (maxreg-2);
	
	//printf("%i\n",numtospill);
	qsort(active,256,sizeof(Regist),compareOccTie);
	
	for(j=1;j<=numtospill;j++)
	{
		//printf("%s is being spilled.\n",active[maxlive[1]-j].name);
		
		for(i=0;i<numreg;i++)
		{
			if(!strcmp(array[i].name,active[maxlive[1]-j].name))
			{
				array[i].spilled=1;
			}
		}
		
	}
	
	
	
}
int getReg(struct Regist* array, char* test)
{
	int i;
	for(i=0;i<numreg;i++)
	{
		if(!strcmp(array[i].name,test))
		{
			return array[i].spillnum;
		}
	}
	return 0;
}

int isSpilled(struct Regist* array, char* test)
{
	int i;
	for(i=0;i<numreg;i++)
	{
		if(!strcmp(array[i].name,test))
		{
			return array[i].spilled;
		}
	}
	return 0;
}
void registerFreer(int* regs, struct Regist* array, int line)
{
	int i;
	int check[maxreg-2];
	memset(check,0,sizeof(check));
	
	for(i=numreg-1;i>=0;i--)
	{
		//printf("%i\n",array[i].spillnum);
		
		if(array[i].spillnum != 0 && check[array[i].spillnum-1]==0)
		{
			check[array[i].spillnum-1]=1;
			
			
			if (line>array[i].LRhigh)
			{
				//printf("%s is dead by line %i, so we can free the register r%i\n",array[i].name,line,array[i].spillnum);
				regs[array[i].spillnum-1] = 0;
			}
			
		}
		
	}
	
}

void registerAllocator(struct Regist* array)
{
	int i;
	int allocced;
	int regs[maxreg-2];
	memset(regs,0,sizeof(regs));
	int j;
	
	for(i=0;i<numreg;i++)
	{
		if(array[i].spilled)
		{
	
		}
		else
		{
			
			for(j=0;j<maxreg-2;j++)
			{
				//printf("%i\n",regs[j]);
				if(regs[j]==0)
				{
					regs[j]=1;
					array[i].spillnum=j+1;
					allocced = 1;
					
					//printf("We've allocated r%i to %s\n",j+1,array[i].name);
					break;
				}
			}
			if (allocced==1)
			{
				allocced=0;
			}
			else
			{
				//printf("%s needs a register!!!!\n",array[i].name);
				registerFreer(regs, array , array[i].LRlow);
				i--;
			}
		}
	}
	
	
	return;
}


struct Instruction* uliAllocator(struct Regist* array,struct Instruction* head)
{
	struct Instruction* curr=head;
	struct Regist* actives;
	int maxlive[] = {0,0};
	
	sprintf(f1,"f%i",maxreg-1);
	sprintf(f2,"f%i",maxreg);
	
	calculateMaxLive(array,maxlive);
	//printf("Maxlive: Line %i; active registers: %i\n",maxlive[0],maxlive[1]);
	
	while(maxlive[1]>maxreg-2)
	{
		actives = activeReg(array,curr,maxlive[0],maxlive[1]);
		spiller(array, actives,maxlive);
		free(actives);
		calculateMaxLive(array,maxlive);
		//printf("Maxlive: Line %i; active registers: %i / %i \n",maxlive[0],maxlive[1],maxreg-2);
	}
	
	while(curr!=NULL)
	{
		if(!strcmp(curr->op,"storeAI"))
		{
			curr = curr->next;
		}
		
		if(strchr(curr->arg1,'r')!=NULL)
		{
			if(isSpilled(array,curr->arg1))
			{
				int seen = beenSeen(array,curr->arg1);
				if(seen)
				{
					if(seen==1)
					{
						curr=insertStore(curr,array);
					}
					else
					{
						insertLoad(curr,1,array);
					}
				}
			}
		}
		if(strchr(curr->arg2,'r')!=NULL)
		{
			if(isSpilled(array,curr->arg2))
			{
				int seen =beenSeen(array,curr->arg2);
				if(seen)
				{
					if(seen==1)
					{
						curr = insertStore(curr,array);
					}
					else
					{
						insertLoad(curr,2,array);
					}
				}
			}
		}
		if(strchr(curr->arg3,'r')!=NULL)
		{
			if(isSpilled(array,curr->arg3))
			{
				int seen =beenSeen(array,curr->arg3);
				if(seen)
				{
					if(seen==1)
					{
						curr = insertStore(curr,array);
					}
					else
					{
						insertLoad(curr,3,array);
					}
				}
			}
		}
		
		curr = curr->next;
	}
	//printf("Maxlive: Line %i; active registers: %i / %i \n",maxlive[0],maxlive[1],maxreg-2);
	//qsort(array,256,sizeof(Regist),compareOcc);
	
	registerAllocator(array);
	
	
	//printTable(array);
	curr = head;
	while(curr!=NULL)
	{
		if(strchr(curr->arg1,'r')!=NULL)
		{
			int spill = isAllocated(array,curr->arg1);
			if(spill)
			{
				char regname[100];
				sprintf(regname,"r%d",spill);
				strcpy(curr->arg1,regname);
			}
		}
		
		if(strchr(curr->arg1,'f')!=NULL)
		{
			curr->arg1[0]='r';
		}
		
		if(strchr(curr->arg2,'r')!=NULL)
		{
			int spill = isAllocated(array,curr->arg2);
			if(spill)
			{
				char regname[100];
				sprintf(regname,"r%d",spill);
				strcpy(curr->arg2,regname);
			}
		}
		
		if(strchr(curr->arg2,'f')!=NULL)
		{
			curr->arg2[0]='r';
		}
		
		
		if(strchr(curr->arg3,'r')!=NULL)
		{
			int spill = isAllocated(array,curr->arg3);
			if(spill)
			{
				char regname[100];
				sprintf(regname,"r%d",spill);
				strcpy(curr->arg3,regname);
			}
		}
		if(strchr(curr->arg3,'f')!=NULL)
		{
			curr->arg3[0]='r';
		}
		
		curr=curr->next;
	}
	
	
	
	printCode(head);
	
	return NULL;
}

int allocate(struct Regist* array, char* name, int* regs)
{
	int i;
	for (i=0;i<numreg;i++)
	{
		if(!strcmp(array[i].name,name))
		{
			int j;
			for(j=0;j<maxreg;j++)
			{
				if(regs[j]==0)
				{
					//printf("\t\tAllocating r%i to %s\n",j+1,array[i].name);
					regs[j]=array[i].namenum;
					array[i].spillnum=j+1;
					array[i].spilled=0;
					return j+1;
				}
			}	
		}
	}
	return 0;
}

void freeTheFurthest(int* regs, int line, struct Instruction* curr, struct Regist* array)
{
	int i;
	int j;
	int max=0;
	int maxindex=0;
	
	//printf("\t\t\t%i\n",curr->line);
	for(j=0;j<maxreg;j++)
	{
		for(i=0;i<numreg;i++)
			{
				if(array[i].namenum==regs[j])
				{
					//printf("%s\n",array[i].name);
					int k;			
					for(k=1; k<=array[i].occreg[0];k++)
						{
							//printf("\t\t%i\n",array[i].occreg[k]);
							if(array[i].occreg[k] >= line )
							{
								//printf("\t\t\tRegister %i has furthest at line %i\n",regs[j],array[i].occreg[k]);
								if(max<=array[i].occreg[k])
								{
									max=array[i].occreg[k];
									maxindex=j;
								}
								break;
								
							}
						}
				}
			}
	}

	//printf("%i\n",max);
	//printf("\t\tFreeing r%i from r%i for now.\n",regs[maxindex],maxindex+1);
	
	struct Instruction* store = malloc(sizeof(Instruction));
	store->op = "storeAI";
	store->arg1 = malloc(100);
	sprintf(store->arg1,"r%i",maxindex+1);
	
	store->arg2 = "r0";
	store->arg3 = malloc(100);
	i=0;
	for(i=0;i<numreg;i++)
	{
		if(array[i].namenum == regs[maxindex])
		{
			sprintf(store->arg3,"%i",array[i].offset);
			array[i].spilled = 1;
		}
	}
	
	store->pattern =1;
	
	store->next = curr;
	store->prev = curr->prev;
	curr->prev = store;
	store->prev->next = store;
	
	
	regs[maxindex]=0;
}
void singleFree(int* regs, struct Regist* array, struct Instruction* curr, int regnum)
{
	int i;
	for(i=0;i<numreg;i++)
	{
		if(regnum == array[i].namenum)
		{
			int k;
			for(k=0;k<maxreg;k++)
			{
				if (regs[k]==array[i].namenum && curr->line > array[i].LRhigh)
				{
					//printf("\t\tWe're done with r%i, so we can free r%i\n",regs[k],k+1);
					regs[k]=0;
					
				}
			}
		}
		
	}
	


}


struct Instruction* bottomUpAllocator(struct Regist* array, struct Instruction* head)
{
	struct Instruction* curr =head;
	int maxlive[] = {0,0};
	
	calculateMaxLive(array,maxlive);
	//printf("Maxlive: Line %i; active registers: %i\n",maxlive[0],maxlive[1]);
	
	if(maxlive[1]<=maxreg)
	{
		maxreg+=2;
		uliAllocator(array,head);
		return NULL;
	}
	int cnt;
	int ofst=0;
	for(cnt=0;cnt<numreg;cnt++)
	{
		array[cnt].offset = ofst;
		ofst-=4;
	}
	
	//printTable(array);
	int regs[maxreg];
	memset(regs,0,sizeof(regs));
	int r1=0;
	int r2=0;

	curr=curr->next;
	
	while(curr!=NULL)
	{	
					
		if(strchr(curr->arg1,'r')!=NULL && strcmp(curr->arg1,"r0"))
		{
			int i,isin=0;
			for(i=0;i<maxreg;i++)
			{
				if(atoi(curr->arg1+1)==regs[i])
				{
					//printf("\t\tr%i is allocated\n",regs[i]);
					isin=1;
					r1=regs[i];
					break;
				}
			}
			if(isin)
			{
				sprintf(curr->arg1,"r%i",i+1);
			}
			else
			{
				if(isSpilled(array,curr->arg1))
				{
					int alloc = allocate(array,curr->arg1,regs);
					if(!alloc)
					{
						freeTheFurthest(regs,curr->line,curr,array);
						alloc = allocate(array,curr->arg1,regs);
					}
					
					struct Instruction* load = malloc(sizeof(Instruction));
					
					load->op = malloc(100);
					strcpy(load->op,"loadAI");
					load->arg1 = malloc(10);
					strcpy(load->arg1,"r0");
					load->arg2 = malloc(100);
					
					int j;
					for(j=0;j<numreg;j++)
					{
						if(array[j].namenum == regs[alloc-1])
						{
							sprintf(load->arg2,"%i",array[j].offset);
						}
					}
					
					load->arg3 = malloc(100);
					sprintf(load->arg3,"r%i",alloc);
					load->pattern=2;
					
					load->next=curr;
					load->prev=curr->prev;
					curr->prev=load;
					load->prev->next=load;
					
					sprintf(curr->arg1,"r%i",alloc);
					r1=regs[alloc-1];
				}
				
				else
				{
					int alloc = allocate(array,curr->arg1,regs);
					if(!alloc)
					{
						freeTheFurthest(regs,curr->line,curr,array);
						alloc=allocate(array,curr->arg1,regs);
					}
					if(alloc)
					{
						sprintf(curr->arg1,"r%i",alloc);
						r1=regs[alloc-1];
					}
				}
			}	
		}
		
		if(!strcmp(curr->op,"load"))
		{
			singleFree(regs,array,curr,r1);
		}
		
		if(strchr(curr->arg2,'r')!=NULL && strcmp(curr->arg2,"r0"))
		{
			int i,isin=0;
			for(i=0;i<maxreg;i++)
			{
				if(atoi(curr->arg2+1)==regs[i])
				{
					//printf("\t\tr%i is allocated\n",regs[i]);
					isin=1;
					r2=regs[i];
					break;
				}
			}
			if(isin)
			{
				sprintf(curr->arg2,"r%i",i+1);
			}
			else
			{
				if(isSpilled(array,curr->arg2))
				{
					int alloc = allocate(array,curr->arg2,regs);
					if(!alloc)
					{
						freeTheFurthest(regs,curr->line,curr,array);
						alloc = allocate(array,curr->arg2,regs);
					}
					struct Instruction* load = malloc(sizeof(Instruction));
					
					load->op = malloc(100);
					strcpy(load->op,"loadAI");
					load->arg1 = malloc(10);
					strcpy(load->arg1,"r0");
					load->arg2 = malloc(100);
					
					int j;
					for(j=0;j<numreg;j++)
					{
						if(array[j].namenum == regs[alloc-1])
						{
							sprintf(load->arg2,"%i",array[j].offset);
						}
					}
					
					load->arg3 = malloc(100);
					sprintf(load->arg3,"r%i",alloc);
					load->pattern=2;
					
					load->next=curr;
					load->prev=curr->prev;
					curr->prev=load;
					load->prev->next=load;
					r2=regs[alloc-1];
					sprintf(curr->arg2,"r%i",alloc);
				}
				else
				{
					int alloc = allocate(array,curr->arg2,regs);
					if(!alloc)
					{
						freeTheFurthest(regs,curr->line,curr,array);
						alloc=allocate(array,curr->arg2,regs);
					}
					if(alloc)
					{
						sprintf(curr->arg2,"r%i",alloc);
						r2=regs[alloc-1];
					}
				}
			}
			
		}
		
		singleFree(regs,array,curr,r1);
		singleFree(regs,array,curr,r2);
		
		if(strchr(curr->arg3,'r')!=NULL && strcmp(curr->arg3,"r0"))
		{	
			int i,isin=0;
			for(i=0;i<maxreg;i++)
			{
				if(atoi(curr->arg3+1)==regs[i])
				{
					//printf("\t\tr%i is allocated\n",regs[i]);
					isin=1;
				}
			}
			if(isin)
			{
				sprintf(curr->arg3,"r%i",i+1);
			}
			else
			{
				int alloc = allocate(array,curr->arg3,regs);
				if(!alloc)
				{
					freeTheFurthest(regs,curr->line,curr,array);
					alloc=allocate(array,curr->arg3,regs);
				}
				if(alloc)
				{
					sprintf(curr->arg3,"r%i",alloc);
				}
			}
			
			
		}


/*
		int k;
		printf("Line %i:",curr->line);
		for(k=0;k<maxreg;k++)
		{
			printf("%i ",regs[k]);
		}
		printf("\n");
		
		
		r1=0;
		r2=0;
*/
		curr=curr->next;
	}

	printCode(head);
	return NULL;
}

int main(int argc, char **argv)
{
	
	if(isdigit(argv[1][0]))
	{
		maxreg=atoi(argv[1]);
	}
	else
	{
		printf("ERROR: First flag must be a number greater than 2.\n");
		return -1;
	}
	if(maxreg<2)
	{
		printf("ERROR: First flag must be a number greater than 2.\n");
		return -1;
	}
	
	int whichalloc=-1;
	
	switch(argv[2][0]){
	case 's':
		whichalloc=1;
		break;
	case 't':
		whichalloc=2;
		break;
	case 'b':
		whichalloc=3;
		break;
	case 'o':
		whichalloc=4;
		break;
	default:
		printf("ERROR: Second flag must be [s|t|b|o].\n");
		return -1;
	}
	
	FILE * fp=fopen(argv[3], "r");
	if(fp == NULL)
	{
		printf("ERROR: File not found or unable to be opened.\n");
		return -1;
	}
	char* buffer = malloc(1000);
	struct Instruction* head=NULL;
	struct Instruction* tail=NULL;
	
	
	int line=0;
	while(fgets(buffer, 1000, fp))
	{
		if((buffer[0]=='/' &&  buffer[1]=='/')||
			buffer[0]=='\n')
		{
			//printf("\n");
		}
		else
		{
			struct Instruction* node = malloc(sizeof(Instruction));

			*node = fillInstruction(buffer,line);
			line++;
			if(head == NULL)
			{
					head = node;
					tail = node;
					node->prev = NULL;
			}
			else
			{
					node->next = NULL;
					tail->next = node;
					node->prev = tail;
					tail = node;
					
			}
			//printf("line1:%s",buffer);
		}
	}
	
	
	struct Regist* array = malloc(sizeof(Regist)*256);
	struct Instruction* temp = head;
	int linenum=0;
	
	while(temp != NULL)
	{
		if(strchr(temp->arg1,'r')!=NULL)
		{
			fillRegister(array,temp->arg1,linenum);
		}
		if(strchr(temp->arg2,'r')!=NULL)
		{
			fillRegister(array,temp->arg2,linenum);
		}
		if(strchr(temp->arg3,'r')!=NULL)
		{
			fillRegister(array,temp->arg3,linenum);
		}
		
		//printf("Line: %i complete.\n",linenum);
		linenum++;
		temp = temp->next;
	}	

	int maxlive[] = {0,0};
	calculateMaxLive(array,maxlive);
	//printf("Maxlive: Line %i; active registers: %i\n",maxlive[0],maxlive[1]);
	//printCode(head);
	//printCodeBack(tail);
	//printTable(array);
	if(maxreg>=numreg)
	{
		printCode(head);
		return 0;
	}
	
	if(whichalloc==1)
	{
		simpleAllocator(array,head);
	}
	if(whichalloc==2)
	{
		uliAllocator(array,head);
	}
	if(whichalloc==3)
	{
		bottomUpAllocator(array,head);
	}
	if(whichalloc==4)
	{
		simpleAllocator(array,head);
	}
	
	return 0;
}


