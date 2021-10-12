/*
 * BituachLeumi.h
 *
 *  Created on: May 28, 2020
 *      Author: rsivan
 */

#ifndef BITUACHLEUMI_H_
#define TM_BITUACHLEUMI_H_


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <semaphore.h>
#include <sys/time.h>
/*
 * TM_CLOCK_TICK - the basic clock step (in microseconds)
 *
 * Relative times are multiplied by this factor to yield real running times
 */
#define TM_CLOCK_TICK  1000

/**
 * Customer is a struct for keeping all customer related information.
 */
typedef struct customer
{ 	
	char         id[10];  //id is string beacause of id that starts with '0'
    int          service; //which service needed

	
    int          sort_time; //time in information stand
    int          service_time; //the time that the clerk is with the customer

    struct timeval	enter_time;
    struct timeval	exit_time;

    int          wait_time;
    int          total_time;

}
    Customer,
   *pCustomer;
/*
services:
1 - Modiin
2 - Zikna	
3 - Sheerim	
4 - Avtala	
5 - Nechut	
6 - Siudi	
7 - Teunot	
8 - Kitsva	
9 - Tashlum	
*/

/*
 * Clerk is a struct for keeping all clerk related information
 */
typedef struct clerk
{
    int       id;
    pthread_t tid;
    int       service;
    int       work_time;
	int		  idleness_time;
	int		  total_time;
	
	struct timeval	enter_time;
    struct timeval	exit_time;

}
Clerk, *pClerk;

/*
 * Queue for each stand service
 */
typedef struct QNode { 
    int cu_index; 
    struct QNode* next; 
}QNode; 

typedef struct Queue { 
    struct QNode *front, *rear; 
}Queue; 

/*
 * stands is a struct that contain the service name, the number of clerks,number of customers and Queue of the customers that need it
 */
typedef struct stands
{
	char *name;
	int cl_num;
	int cu_num;
	int treated;
	Queue *q;
}
Stands, *pStand;

QNode* newNode(int k);
Queue* createQueue() ;
void enQueue(struct Queue* q, int k) ;
int deQueue(struct Queue* q) ;
#endif /* BITUACHLEUMI_H_ */
