#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h>
#include <string.h> 

#define NR_BLOCKS     100
#define MAX_USE_SEC   5
#define MAX_SLEEP_SEC 3
#define NR_THREADS    5 

#define  empty 0
#define occupied 1


struct Buffer
{
	int no;
	int status;
	int data;
};

struct HashTable
{
	struct HashTable *prev;
	struct Buffer b;
	struct HashTable *next;
};

struct FreeList
{
	struct FreeList *prev;
	struct Buffer b;
	struct FreeList *next;
};


void AppendInHashTable(struct HashTable **head,struct Buffer b)
{
	struct HashTable *newnode;
	newnode = malloc(sizeof(struct HashTable));
	(newnode->b).no = b.no;
	(newnode->b).status = b.status;
	(newnode->b).data = b.data;
	newnode->next = newnode->prev = NULL;
	if(*head == NULL)
	{
		*head = newnode;
	}
	else
	{
		struct HashTable *temp = (*head);
		while(temp->next != NULL)
		{
			temp = temp->next;
		}
		temp->next = newnode;
		newnode->prev = temp;
		
	}
}

void DisplayInHashTable(struct HashTable *head)
{
	struct HashTable *temp = head;
	printf("\n\tNo\tStatus\tData\n");
	while(temp != NULL)
	{
		printf("\n\t%d\t%d\t%d\n",((temp->b).no),((temp->b).status),((temp->b).data));
		temp = temp->next;
	}
	printf("\n");
}

void AppendInFreeList(struct FreeList **head,struct Buffer b)
{
	struct FreeList *newnode;
	newnode = malloc(sizeof(struct FreeList));
	(newnode->b).no = b.no;
	(newnode->b).status = b.status;
	(newnode->b).data = b.data;
	newnode->prev = newnode->next = NULL;
	if(*head == NULL)
	{
		*head = newnode;
	}
	else
	{
		struct FreeList *temp = *head;
		while(temp->next != NULL)
		{
			temp = temp->next;
		}
		temp->next = newnode;
		newnode->prev = temp;
	}
}

void DisplayInFreeList(struct FreeList *head)
{
	struct FreeList *temp = head;
	printf("\n\tNo\tStatus\t\tData\n");
	while(temp != NULL)
	{
		printf("\n\t%d\t%d\t\t%d\n",((temp->b).no),((temp->b).status),((temp->b).data));
		temp = temp->next;
	}
	printf("\n");
}

int DeleteFromHashTable(struct HashTable **head,int bufferNo)
{
	struct HashTable *temp = *head;
	while(temp != NULL && (temp->b).no != bufferNo)
	{
		temp = temp->next;
	}
	if(temp == NULL)
	{
		printf("\n\tNot Found\n");
		return 0;
	}
	else
	if(temp != NULL)
	{
		if(temp == *head)
		{
			*head = temp->next;
		}
		else
		{
			(temp->prev)->next = temp->next;
		}
		free(temp);
		return 1;
	}
}

int DeleteFromFreeList(struct FreeList **head,int bufferNo,struct Buffer *b)
{
	struct FreeList *temp = *head;
	while(temp != NULL && (temp->b).no != bufferNo)
	{
		temp = temp->next;
	}
	if(temp == NULL)
	{
		printf("\n\tNot Found\n");
		return 0;
	}
	else
	if(temp != NULL)
	{
		if(temp == *head)
		{
			*head = temp->next;
			if(b != NULL)
			{
				b->no = (temp->b).no;
				b->status = (temp->b).status;
				b->data = (temp->b).data;
			}
		}
		else
		{
			(temp->prev)->next = temp->next;
			if(b != NULL)
			{
				b->no = (temp->b).no;
				b->status = (temp->b).status;
				b->data = (temp->b).data;
			}
		}
		free(temp);
		return 1;
	}
}

int DeleteFromFreeListFCFS(struct FreeList **head,struct Buffer *b)
{
	struct FreeList *temp = *head;
	
	*head = temp->next;
	b->no = (temp->b).no;
	b->status = (temp->b).status;
	b->data = (temp->b).data;
	free(temp);
	return 1;
}

int SearchInFreeList(struct FreeList *head,int bufferNo)
{
	struct FreeList *temp = head;
	while(temp != NULL && (temp->b).no != bufferNo)
	{
		if((temp->b).no == bufferNo)
		{
			break;
		}
		temp = temp->next;
	}
	if(temp == NULL)
	{
		printf("\n\tNot Found\n");
		return 0;
	}
	return 1;
}

int SearchInHashTable(struct HashTable *head,int bufferNo)
{
	struct HashTable *temp = head;
	while(temp != NULL && (temp->b).no != bufferNo)
	{
		if((temp->b).no == bufferNo)
		{
			break;
		}
		temp = temp->next;
	}
	if(temp == NULL)
	{
		printf("\n\tNot Found\n");
		return 0;
	}
	return 1;
}

int checkStatusInHashTable(struct HashTable *head,int bufferNo,struct Buffer *outParameter)
{
	struct HashTable *temp = head;
	while(temp != NULL && (temp->b).no != bufferNo)
	{
		if((temp->b).no == bufferNo)
		{
			break;
		}
		temp = temp->next;
	}
	if(temp == NULL)
	{
		printf("\n\tNot Found\n");
		memset(outParameter,0,sizeof(struct Buffer));
		return -1;
	}
	if((temp->b).status == empty)
	{
		outParameter->no = (temp->b).no;
		outParameter->status = (temp->b).status;
		outParameter->data = (temp->b).data;
		return 1;
	}
	else
	{
		memset(outParameter,0,sizeof(struct Buffer));
		return 0;
	}
}



struct Buffer b[NR_BLOCKS];
struct HashTable *headHash[4];
struct Buffer b1;
struct FreeList *headFree = NULL;
int bufferNo,res,ret,swapBuffer,data;
pthread_mutex_t free_list_mutex, hash_list_mutex;

void *getblk (int); 
void use_blk (int, int); 
void free_blk (int); 
void *thread_function (void *args); 

int main (void) 
{
	pthread_t thread_arr[NR_THREADS]; 
	int i; 
	pthread_mutex_init ( &free_list_mutex, NULL);
	pthread_mutex_init ( &hash_list_mutex, NULL);
	
	for(i = 0 ;i < 4;i++)
	{
		headHash[i] = NULL;
	}
	
	for(i = 0;i< NR_BLOCKS;i++)
	{
 		b[i].no = i;
		b[i].status = empty;
		b[i].data = (i + 10);
		
		AppendInFreeList(&headFree,b[i]);	
	}
	printf("\n\tDisplaying from Free List \n");
	DisplayInFreeList(headFree);
	printf("\n\tAppending in HashTable \n");
	for(i = 0;i < NR_BLOCKS;i++)
	{
		if(b[i].no % 4 == 0)
		{
			AppendInHashTable(&(headHash[0]),b[i]);
		}
		else
		if(b[i].no % 4 == 1)
		{
			AppendInHashTable(&(headHash[1]),b[i]);
		}
		else
		if(b[i].no % 4 == 2)
		{
			AppendInHashTable(&(headHash[2]),b[i]);
		}
		else
		if(b[i].no % 4 == 3)
		{
			AppendInHashTable(&(headHash[3]),b[i]);
		}
	}

	for (i=0; i < NR_THREADS; i++)
	{
		pthread_create (&thread_arr[i], NULL, thread_function, NULL); 
	}

	for (i=0; i < NR_THREADS; i++) 
	{
		pthread_join (thread_arr[i], NULL); 
	}

	exit (EXIT_SUCCESS); 
}

void *thread_function (void *args) 
{
	int nr_blk; 
	
	while (1)
	{
		nr_blk = (rand () % (NR_BLOCKS)) + 1; 
		getblk (nr_blk); 
		use_blk ((rand () % MAX_USE_SEC) + 1, nr_blk); 
		free_blk (nr_blk);
		sleep ((rand () % MAX_SLEEP_SEC) + 1);
	}
}

void *getblk (int nr_blk) 
{
	struct Buffer outParameter;
	printf ("Thread:%x calling getblk nr_blk:%d\n", (unsigned int) pthread_self(), nr_blk); 
	again : if((ret = checkStatusInHashTable(headHash[res], nr_blk, &outParameter)) != -1)
	{
		printf("\n\tFound In HashTable\n");
		if(ret == 1)
		{
			printf("\n\tBuffer is empty\n");
			outParameter.status = occupied;
			if(headFree != NULL)
			{
				pthread_mutex_lock ( &free_list_mutex);
				//if(DeleteFromFreeList(&headFree, bufferNo, NULL) == 1)
				//{
				//	printf("\n\tDeleted\n");
				//	DisplayInFreeList(headFree);
				//	
				//	//use that buffer
				//	outParameter.data = outParameter.data + 10;
				//	outParameter.status = empty;
				//	AppendInFreeList(&hieadFree,outParameter);
					
				//}
				DeleteFromFreeList ( &headFree, nr_blk, NULL);
				pthread_mutex_unlock (&free_list_mutex);
				return;
			}
			else
			{
				//free list is NULL
				//sleep(5);
				goto again;
			}
		}
		else
		if ( ret == 0)
		{
			printf("\n\tBuffer is not empty\n");
			//wait for freeing
			//sleep(5);
			goto again;
		}
	}
	else
	{
		int free_list_ret, hash_table_ret;
		printf("\n\tNot found in HashTable\n");
		memset(&outParameter,0,sizeof(struct Buffer));
		pthread_mutex_lock (&free_list_mutex);
		free_list_ret = DeleteFromFreeListFCFS (&headFree, &outParameter);
		pthread_mutex_unlock (&free_list_mutex);
		if(ret == 1)
		{
			res = outParameter.no % 4;
			//hash_table_ret = DeleteFromHashTable (&(headHash[res]), nr_blk );
			//if(hash_table_ret != 1)
			//{
			//	printf("\n\tNot Deleted\n");
			//	exit(1);
			//}
			printf("\n\tDeleted bufferNo %d from freeList\n",outParameter.no);
			outParameter.status = occupied;
			
			
			b1.no = nr_blk;
			b1.status = occupied;
			//b1.data = data;
			//b1.status = empty;
			res = nr_blk % 4;
			if(res == 0)
			{
				pthread_mutex_lock ( &hash_list_mutex );
				AppendInHashTable(&(headHash[res]),b1);
				pthread_mutex_unlock (&hash_list_mutex);
			}
			else
			if(res == 1)
			{	
				pthread_mutex_lock ( &hash_list_mutex );
				AppendInHashTable(&(headHash[res]),b1);
				pthread_mutex_unlock (&hash_list_mutex);
			}
			else
			if(res == 2)
			{
				pthread_mutex_lock ( &hash_list_mutex );
				AppendInHashTable(&(headHash[res]),b1);
				pthread_mutex_unlock (&hash_list_mutex);
			}
			else
			if(res == 3)
			{
				pthread_mutex_lock ( &hash_list_mutex );
				AppendInHashTable(&(headHash[res]),b1);
				pthread_mutex_unlock (&hash_list_mutex);
			}
		}
	}
	printf ("Thread:%x called getblk nr_blk:%d\n", (unsigned int) pthread_self(), nr_blk); 
}

void use_blk (int secs, int nr_blk)
{
	printf ("Thread:%x using block %d for %d secs\n", 
		 (unsigned int) pthread_self (), nr_blk, secs); 
	sleep (secs); 
	printf ("Thread:%x used block %d for %d secs\n", 
		 (unsigned int) pthread_self (), nr_blk, secs); 
}

void free_blk (int nr_blk)
{
	int ret;
	struct Buffer outParameter;
	printf ("Thread:%x freeing block %d\n", 
		(unsigned int) pthread_self (), nr_blk); 
	ret = checkStatusInHashTable(headHash[res], nr_blk, &outParameter);
	if ( ret != -1 )
	{
		outParameter.status = empty;
		b1.no = nr_blk;
		b1.status = empty;
		res = nr_blk % 4;
		switch (res)
		{
			case 0:
				pthread_mutex_lock (&free_list_mutex);
				AppendInFreeList(&headFree,b1);
				pthread_mutex_unlock (&free_list_mutex);
				break;
			case 1:
				pthread_mutex_lock (&free_list_mutex);
				AppendInFreeList(&headFree,b1);
				pthread_mutex_unlock (&free_list_mutex);
				break;
			case 2:
				pthread_mutex_lock (&free_list_mutex);
				AppendInFreeList(&headFree,b1);
				pthread_mutex_unlock (&free_list_mutex);
				break;
			case 3:
				pthread_mutex_lock (&free_list_mutex);
				AppendInFreeList(&headFree,b1);
				pthread_mutex_unlock (&free_list_mutex);
				break;
		}
	}
	printf ("Thread:%x freed block %d\n", 
		(unsigned int) pthread_self (), nr_blk); 
}
