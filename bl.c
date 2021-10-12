/*
*	Shai Siso 
*	Shahar Avital 
*/

#include "BituachLeumi.h"

int getIndex(Stands *stands,char* st,int stand_num){ //get the index of given service
	int i;
	for (i=0; i<stand_num; i++)
		if (strcmp(stands[i].name,st)==0)
			return i;
}

void *treatment(void *num);


sem_t *mutex,*sem_q;
Clerk *cl;
Customer *cu;
Stands *stands;
int clerk_num=0,stand_num,cus_num,*flag;

int main(int argc, char* argv[]){

int i,j,num,*p,*cnt_cus,delay_time,exec_time,avg_work[2],avg_idl[2],avg_tot[2];
char buff[256],*b=buff,*ser_name,*c_time_string;
size_t bsize=256;
time_t current_time;
struct timeval start_prog,end_prog;
FILE *f=fopen(argv[1], "r"),*fo=fopen("output.txt","w");
	if (f == NULL || fo == NULL) 
		{	perror("fopen"); exit(EXIT_FAILURE);	}
	gettimeofday(&start_prog, NULL); //for calculate execution time
	getline(&b,&bsize,f);
	stand_num=atoi(b); //to know how much lines to read
	stands=(Stands*)malloc(stand_num*sizeof(Stands)); //strings array of all the services 
	flag=(int*)malloc(stand_num*sizeof(int)); //indicator for services
	cnt_cus=(int*)malloc(stand_num*sizeof(int));
	
	if (stands == NULL || flag == NULL || cnt_cus == NULL)
		{	perror("stands/flag array"); exit(EXIT_FAILURE);	}

	// sem initial
	mutex=(sem_t*)malloc(stand_num*sizeof(sem_t));
	sem_q=(sem_t*)malloc(stand_num*sizeof(sem_t));
	if (mutex==NULL || sem_q==NULL)
		{	perror("semaphores creation"); exit(EXIT_FAILURE);	}
	for (i=0; i<stand_num; i++){
		sem_init(&mutex[i],0,1);	
		sem_init(&sem_q[i],0,0); // for queue of customers
	}

//setting clerks and services stands
	for (i=0; i<stand_num; i++){
		getline(&b,&bsize,f);
		ser_name=strtok(b,"	");
		flag[i]=1;
		cnt_cus[i]=0; //counting customers for each stand service
		stands[i].treated=0;//initial for treatment func
		stands[i].name=malloc((strlen(ser_name+1)) * sizeof(char));
		if (stands[i].name == NULL)
			{	perror("service string"); exit(EXIT_FAILURE);	}
		strcpy(stands[i].name,ser_name);
		num=atoi(strtok(NULL,"	")); //number of clerks in this stand
		stands[i].cl_num=num;
		stands[i].cu_num=0;
		stands[i].q=createQueue();///queue fo stand initial
		for (j=0; j<stands[i].cl_num; j++){
			if (clerk_num==0){ //first clerk in the system
				cl=(Clerk*)malloc(sizeof(Clerk));
				clerk_num++;
			}
			else
				cl=(Clerk*)realloc(cl,(++clerk_num)*sizeof(Clerk));
			cl[clerk_num-1].id=clerk_num;
			cl[clerk_num-1].service=i;
			//sending clerks to their service stands
			p=malloc(sizeof(*p));
			*p=clerk_num-1;

			pthread_create(&(cl[clerk_num-1].tid),NULL,treatment,(void*)p);
		}
	}


//saving customers details
	getline(&b,&bsize,f);
	cus_num=atoi(b); //to know how much lines to read
	cu=(Customer*)malloc(cus_num*sizeof(Customer));
	if (cu==NULL)
		{	perror("Customer array"); exit(EXIT_FAILURE);	}
	for (i=0; i<cus_num; i++){
		getline(&b,&bsize,f);
		strcpy(cu[i].id,strtok(b,"	"));
		delay_time=atoi(strtok(NULL,"	"));// how much time to wait for next customer
		cu[i].sort_time=atoi(strtok(NULL,"	"));
		cu[i].service=getIndex(stands,strtok(NULL,"	"),stand_num);
		cu[i].service_time=atoi(strtok(NULL,"	"));
		cnt_cus[cu[i].service]++;
		cnt_cus[getIndex(stands,"Modiin",stand_num)]++;//everyone go to modiin
		gettimeofday(&(cu[i].enter_time), NULL);
		enQueue(stands[getIndex(stands,"Modiin",stand_num)].q,i); //at first every customer is for modiin
		sem_post(&sem_q[getIndex(stands,"Modiin",stand_num)]);
		usleep(delay_time*TM_CLOCK_TICK); //this is delay time between cutomers
	}
	//--no more new customers arrive now--

	for (i=0; i<stand_num; i++){ 
		stands[i].cu_num=cnt_cus[i];
	}


	for (i=0; i<clerk_num; i++)
		pthread_join(cl[i].tid,NULL);
	//output
	current_time = time(NULL);
   	c_time_string = ctime(&current_time);
	fprintf(fo,"%s\nWelcome to Bituach Leumi\nAt your side, in the important moments of the life\n",c_time_string);
	gettimeofday(&end_prog, NULL);
	exec_time=(end_prog.tv_sec - start_prog.tv_sec) * 1000.0; 
   	exec_time += (end_prog.tv_usec - start_prog.tv_usec) / 1000.0; 
	fprintf(fo,"total execution time: %d seconds\n\nClerks:\n",exec_time);
	fprintf(fo,"id	service	work	idleness	total\n");
	avg_work[1]=0;
	avg_idl[1]=0;;
	avg_tot[1]=0;
	for (j=0;j<stand_num;j++){
		avg_work[0]=0;
		avg_idl[0]=0;
		avg_tot[0]=0;
		for (i=0; i<clerk_num; i++){
			if (cl[i].service==j){
				avg_work[0]+=cl[i].work_time;
				avg_idl[0]+=cl[i].idleness_time;
				avg_tot[0]+=cl[i].total_time;
				avg_work[1]+=cl[i].work_time;
				avg_idl[1]+=cl[i].idleness_time;
				avg_tot[1]+=cl[i].total_time;
				fprintf(fo,"%d	%5s	%5d	%d	%10d\n",cl[i].id,stands[j].name,cl[i].work_time,cl[i].idleness_time,cl[i].total_time);
			}
		}
		avg_work[0]=avg_work[0]/stands[j].cl_num;
		avg_idl[0]=avg_idl[0]/stands[j].cl_num;
		avg_tot[0]=avg_tot[0]/stands[j].cl_num;
		fprintf(fo,"%s clerks averages(sec): work time: %d	idleness time: %d	total time: %d\n\n",stands[j].name,avg_work[0],avg_idl[0],avg_tot[0]);
	}
	avg_work[1]=avg_work[1]/clerk_num;
	avg_idl[1]=avg_idl[1]/clerk_num;
	avg_tot[1]=avg_tot[1]/clerk_num;
	
	fprintf(fo,"Total clerks averages(sec):\n work time: %d	idleness time: %d	total time: %d\n",avg_work[1],avg_idl[1],avg_tot[1]);
	fprintf(fo,"\nCustomers:\ncno.	id	     total     wait  treament(service + sort)\n");
	avg_work[1]=0;
	avg_idl[1]=0;;
	avg_tot[1]=0;
	for (i=0; i<cus_num; i++){
		fprintf(fo,"%d	%10s	%5d	%5d	%5d	\n",i+1,cu[i].id,cu[i].total_time,cu[i].wait_time,cu[i].total_time-cu[i].wait_time);
		avg_work[1]+=cu[i].total_time-cu[i].wait_time;
		avg_idl[1]+=cu[i].wait_time;
		avg_tot[1]+=cu[i].total_time;
	}
	avg_work[1]=avg_work[1]/cus_num;
	avg_idl[1]=avg_idl[1]/cus_num;
	avg_tot[1]=avg_tot[1]/cus_num;
	fprintf(fo,"Averages(sec):\n service time: %d	wait time: %d	total time: %d\n",avg_work[1],avg_idl[1],avg_tot[1]);

	free(cl);//free clerks
	free(cu);//free Customers
	free(mutex);
	free(sem_q);
	free(cnt_cus);
	free(flag);
	for (i=0; i<stand_num; i++){
		free(stands[i].name);
		free(stands[i].q);
	}
	free(stands);
	return 0;
}



void *treatment(void *num){
	int i,cl_index=*(int*)num,ser_index=cl[cl_index].service,cus_index,first_cus=1;
	int elapsedTime;
	time_t current_time;
    char* c_time_string;
	//printf ("%d\n",cl_index);
	struct timeval	enter_rest;
    struct timeval	exit_rest;
	while (flag[ser_index]==1){ //exit loop when no customers left
		if(!first_cus) //start resting time
			gettimeofday(&enter_rest, NULL);
		sem_wait(&mutex[ser_index]);//critical section
		if (stands[ser_index].cu_num != 0 && stands[ser_index].treated >= stands[ser_index].cu_num){//(flag[ser_index]==0)-- prevent deadlock (for the last customer)
			sem_post(&mutex[ser_index]);
		 	break;
		}
		sem_wait(&sem_q[ser_index]); //will be blocked until customers will be in queue
		if (!first_cus){ // calculate clerk idleness time
			gettimeofday(&exit_rest, NULL);
			elapsedTime = (exit_rest.tv_sec - enter_rest.tv_sec) * 1000.0; 
    		elapsedTime += (exit_rest.tv_usec - enter_rest.tv_usec) / 1000.0; 
			cl[cl_index].idleness_time+=elapsedTime;
		}
		if (first_cus){
			gettimeofday(&cl[cl_index].enter_time, NULL); //clerk start to work
			first_cus=0;
		}
		cus_index=deQueue(stands[ser_index].q); //saving index of client to treat
		//printing the time
		current_time = time(NULL);
   		c_time_string = ctime(&current_time);
		printf("Client no.%d [id : %s] is now in %s stand | %s\n",cus_index+1,cu[cus_index].id,stands[ser_index].name,c_time_string);
		stands[ser_index].treated++;
		if ( stands[ser_index].cu_num != 0 && stands[ser_index].treated >= stands[ser_index].cu_num)
			flag[ser_index]=0;
		sem_post(&mutex[ser_index]);//exit critical section
		//treatment
		if (ser_index==getIndex(stands,"Modiin",stand_num)){//modiin
			usleep(cu[cus_index].sort_time*TM_CLOCK_TICK);
			enQueue(stands[cu[cus_index].service].q,cus_index); //adding customer to service queue
			sem_post(&sem_q[cu[cus_index].service]);
		}
		else
			usleep(cu[cus_index].service_time*TM_CLOCK_TICK); //service
			

		//customer finish treat -- calculate total time
		current_time = time(NULL);
   		c_time_string = ctime(&current_time);
		printf("Client no.%d [id : %s] is leaving %s stand | %s\n",cus_index+1,cu[cus_index].id,stands[ser_index].name,c_time_string);
		gettimeofday(&cu[cus_index].exit_time, NULL);
		cu[cus_index].total_time=(cu[cus_index].exit_time.tv_sec - cu[cus_index].enter_time.tv_sec) * 1000.0; 
    	cu[cus_index].total_time += (cu[cus_index].exit_time.tv_usec - cu[cus_index].enter_time.tv_usec) / 1000.0; 
		cu[cus_index].wait_time=cu[cus_index].total_time-(cu[cus_index].service_time+cu[cus_index].sort_time);// waiting time
		
	}
		//finish working
		gettimeofday(&cl[cl_index].exit_time, NULL); //clerk finish working
		cl[cl_index].total_time=(cl[cl_index].exit_time.tv_sec - cl[cl_index].enter_time.tv_sec) * 1000.0; 
    	cl[cl_index].total_time += (cl[cl_index].exit_time.tv_usec - cl[cl_index].enter_time.tv_usec) / 1000.0; 
		cl[cl_index].work_time=cl[cl_index].total_time-cl[cl_index].idleness_time;
}


/*
*Queue implementation
*/
QNode* newNode(int k)
{ 
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode)); 
    temp->cu_index = k; 
    temp->next = NULL; 
    return temp; 
} 

Queue* createQueue() 
{ 
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue)); 
    q->front = q->rear = NULL; 
    return q; 
} 
void enQueue(struct Queue* q, int k) 
{ 
    // Create a new LL node 
    struct QNode* temp = newNode(k); 
  
    // If queue is empty, then new node is front and rear both 
    if (q->rear == NULL) { 
        q->front = q->rear = temp; 
        return; 
    } 
  
    // Add the new node at the end of queue and change rear 
    q->rear->next = temp; 
    q->rear = temp; 
} 
int deQueue(struct Queue* q) 
{ 
	int index;
    // If queue is empty, return -1. 
    if (q->front == NULL) 
        return -1; 
  
    // Store previous front and move front one node ahead 
    struct QNode* temp = q->front; 
	index=q->front->cu_index;
    q->front = q->front->next; 
  
    // If front becomes NULL, then change rear also as NULL 
    if (q->front == NULL) 
        q->rear = NULL; 
  
    free(temp); 
	return index;
} 

