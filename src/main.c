#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define NEW_CUSTOMER "s1"          //These values denotes the type of event. This value denotes event of a customer who has arrived in the bank
#define SERVED_CUSTOMER "s2"       //Event of a customer whose service has been completed
#define TELLER_IDLE "s3"           //Event of the teller who came back from idle time
#define TELLER_SERVING "s4"        //Teller event who came back from serving a customer


float globalTime = 0;                               //All the variables containing statistics of the simulation are made global
int numCustomersServed = 0, func_pointer_log = 0;
float totalTimeSpentByCustomers = 0;
float totalTellerService = 0, totalTellerIdle = 0;
float maxWaitTime = 0, standardDeviation = 0;

struct Customer;
struct TQNode;
struct tellerQueue;
struct Teller;
struct Event;
struct eventQueue;
struct EQNode;

struct Customer{
    int queueNumber;                //The queue in which the customer went
    int tellerNumber;               //The identity of the number by which the customer was served
    float arrivalTime;
    float serviceStartTime;
    float serviceEndTime;
};
struct TQNode{                          //Node for the teller queue, each node will contains a pointer to a customer, and the next node
    struct Customer *customer;
    struct TQNode *next;
};
struct tellerQueue{                     //The tellerQueue stores a pointer to it's head, and the current length
    int length;
    struct TQNode *head;
};
struct Teller{
    int tellerNumber, customersServed;
    float idleTime, totalIdleTime;
    float serviceTime, totalServiceTime;
};

void addAtTailTQ(struct tellerQueue *list, struct Customer *customer){          //Adds a customer at the end of the queue
    if(list->head == NULL){                             //Empty queue, make a new node and make it the head
        list->length = 1;
        list->head = malloc(sizeof(struct TQNode));
        list->head->customer = customer;
        list->head->next = NULL;
    }
    else{                                           //Else traverse to the last node, and add a new node at the end
        list->length = list->length + 1;
        struct TQNode* last = list->head;
        while(last->next != NULL){
            last = last->next;
        }
        struct TQNode* tmp = malloc(sizeof(struct TQNode));
        tmp->next = NULL;
        tmp->customer = customer;
        last->next = tmp;
    }
}
struct Customer* removeHeadTQ(struct tellerQueue *list){        //Removes the customer at the head, and returns the pointer to it.
    if(list->head == NULL){
        printf("Teller Queue Already Empty\n");
        return NULL;
    }
    else{
        list->length = list->length - 1;
        struct Customer* customer = list->head->customer;
        list->head = list->head->next;
        return customer;
    }
}

struct Event{
    float eventTime;
    char* eventType;                                   
    struct Customer *customer;
    struct Teller *teller;
    void (*action)(struct eventQueue*, struct tellerQueue* [], struct Event*, int , float);         //This is the action method of the event. It's signature is same irrespective of type of the event
};
struct EQNode{              //Each node of the event queue stores a pointer to the event, and the next node
    struct Event *event;
    struct EQNode *next;
};
struct eventQueue{                  //The event queue is a priority queue, and maintains the events in ascending order of the event time
    int length;
    struct EQNode* head;
};

void addEQ(struct eventQueue *eq, struct Event *event){             //Adds a event in a eventQueue
     if(eq->head == NULL){                                          //Empty Queue, make a new Node and make it the head
         eq->length = 1;
         eq->head = malloc(sizeof(struct EQNode));
         eq->head->event = event;
         eq->head->next = NULL;
     }
     else{
         struct EQNode* prev = NULL;
         eq->length = eq->length + 1;
         struct EQNode *tmp = eq->head;
         while(tmp != NULL){
             if(tmp->event->eventTime > event->eventTime){
                 break;
             }
             prev = tmp;
             tmp = tmp->next;
         }
         if(prev == NULL){
             struct EQNode* newNode = malloc(sizeof(struct EQNode));
             newNode->event = event;
             newNode->next = eq->head;
             eq->head = newNode;
         }
         else{
            struct EQNode* newNode = malloc(sizeof(struct EQNode));
            newNode->event = event;
            newNode->next = prev->next;
            prev->next = newNode;
         }
     }
}
struct Event* popEQ(struct eventQueue *eq){                    //Removes the first event in the event Queue, which is the event with least event time value.
    if(eq->head == NULL){
        printf("The event Queue is already empty\n");
        return NULL;
    }
    else{
        eq->length = eq->length - 1;
        struct Event* event = eq->head->event;
        eq->head = eq->head->next;
        return event;
    }
}

//int shortestQueue(struct tellerQueue *arr[], int num_tellers){
//    int index = 0, min = arr[0]->length;
//    for(int i = 0; i < num_tellers; i++){
//        if(arr[i]->length <= min){
//            min = arr[i]->length;
//            index = i;
//        }
//    }
//    return index;
//}
int shortestRandomQueue(struct tellerQueue *arr[], int num_tellers){            //Given an array of tellerQueues, this function returns a random queue out of all the shortest queues.
    int min = arr[0]->length;
    for(int i = 0; i < num_tellers; i++){               //First find the length of the shortest queue in the array
        if(arr[i]->length < min){
            min = arr[i]->length;
        }
    }
    int count = 0;
    int tmp[num_tellers];
    for(int i = 0; i < num_tellers; i++){           //Then store the indices of all the queue whose length is same as the minimum length
        if(arr[i]->length == min){
            tmp[count] = i;
            count++;
        }
    }
    int lo = 0, hi = count - 1;
    int x = rand() % (hi - lo + 1) + lo; //(rand()%(hi - lo + 1)) will lie in [0,hi-lo], and x lies in [lo,hi]
    return tmp[x];                      //Return the index of the shortest random queue
}

//int giveNonEmptyQueue(struct tellerQueue *arr[],int num_tellers){
//    for(int i = 0; i < num_tellers; i++){
//        if(arr[i]->length != 0){
//            return i;           //Returns the first non empty queue encountered
//        }
//    }
//    return -1; //All queues are empty
//}
int giveRandomNonEmptyQueue(struct tellerQueue *arr[],int num_tellers){         //Given an array of teller queue, return the index of a random non empty queue
    int tmp[num_tellers], count = 0;
    for(int i = 0; i < num_tellers; i++){           //Store the indices of all the non empty queues in an array
        if(arr[i]->length != 0){
            tmp[count] = i;
            count++;
        }
    }
    if(count == 0){     
        return -1;              //All queues are empty
    }
    else{
        int lo = 0, hi = count - 1;
        int x = rand() % (hi - lo + 1) + lo;    //else return the index of a random non empty queue
        return tmp[x];
    }
}

void newCustomerActionOPT(struct eventQueue *eq, struct tellerQueue *arr[], struct Event *event, int num_tellers,float avg_service_time){
    //This is the action for a new customer, in the "One queue per teller" regime
    
    globalTime = event->eventTime;
    //give a random teller queue with shortest length and put the customer in there
    int shortest = shortestRandomQueue(arr,num_tellers);
    event->customer->queueNumber = shortest;
    addAtTailTQ(arr[shortest],event->customer);     //Add the customer to the queue
}
void newCustomerActionC(struct eventQueue *eq, struct tellerQueue *arr[], struct Event *event, int num_tellers, float avg_service_time){
    //This is the action for a new customer in the "Common Queue" regime

    globalTime = event->eventTime;
    event->customer->queueNumber = 0;
    addAtTailTQ(arr[0],event->customer);
}

void servedCustomerAction(struct eventQueue *eq, struct tellerQueue *arr[], struct Event *event, int num_tellers, float avg_service_time){
    numCustomersServed++;
    globalTime = event->eventTime;
    totalTimeSpentByCustomers += event->customer->serviceEndTime - event->customer->arrivalTime;
    free(event);        //The event object is deleted after customer service completion
}

void tellerActionOPT(struct eventQueue *eq, struct tellerQueue *arr[], struct Event *event, int num_tellers, float avg_service_time){
     //This is the teller event action in the "One queue per teller" regime

     globalTime = event->eventTime;
     if(strcmp(event->eventType,TELLER_IDLE) == 0){         //Teller which came back after idle time
         totalTellerIdle += event->teller->idleTime;
         event->teller->totalIdleTime += event->teller->idleTime;
     }
     else{
         totalTellerService += event->teller->serviceTime;  //Teller which came back after serving a customer
         event->teller->totalServiceTime += event->teller->serviceTime;
         event->teller->customersServed++;
     }
     int NonEmptyQueue;
     if(arr[event->teller->tellerNumber]->length != 0){             //The queue in front of this teller is non empty, then just pick a customer from that queue
         NonEmptyQueue = event->teller->tellerNumber;
     }
     else{
         NonEmptyQueue = giveRandomNonEmptyQueue(arr,num_tellers);              //Else pick a random non empty queue
     }
     if(NonEmptyQueue < 0){                                                     //All queues are empty, the teller will go into idle state
         event->eventType = TELLER_IDLE;
         event->teller->idleTime = (float)150 * rand()/(float)RAND_MAX;           //random idle time in range 1-150 seconds
         event->eventTime = globalTime + event->teller->idleTime;
         event->action = tellerActionOPT;
         addEQ(eq,event);                                                   //Add the teller event back to the queue
     }
     else{

         struct Event *newCustEvent = malloc(sizeof(struct Event));             //A non empty queue exists, so a customer is picked from there, and put to serve
         struct Customer *customerToServe = removeHeadTQ(arr[NonEmptyQueue]);
         customerToServe->serviceStartTime = globalTime;
         float serviceTime = 2 * avg_service_time * rand()/(float)RAND_MAX;
         customerToServe->serviceEndTime = customerToServe->serviceStartTime + serviceTime;
         customerToServe->tellerNumber = event->teller->tellerNumber;

         newCustEvent->customer = customerToServe;                          //A new customer serve event
         newCustEvent->teller = NULL;
         newCustEvent->eventTime = customerToServe->serviceEndTime;
         newCustEvent->eventType = SERVED_CUSTOMER;
         newCustEvent->action = servedCustomerAction;

         event->eventType = TELLER_SERVING;                             //Teller service completion event
         event->eventTime = customerToServe->serviceEndTime;
         event->action = tellerActionOPT;
         event->teller->serviceTime = serviceTime;
         addEQ(eq,event);
         addEQ(eq,newCustEvent);

         float waitTime = customerToServe->serviceStartTime - customerToServe->arrivalTime;
         if(maxWaitTime < waitTime){
             maxWaitTime = waitTime;
         }
     }
}
void tellerActionC(struct eventQueue *eq, struct tellerQueue *arr[], struct Event *event, int num_tellers, float avg_service_time){
    //This is the teller action in the "Common queue" regime

    globalTime = event->eventTime;
    if(strcmp(event->eventType,TELLER_IDLE) == 0){
        totalTellerIdle += event->teller->idleTime;
        event->teller->totalIdleTime += event->teller->idleTime;
    }
    else{
        totalTellerService += event->teller->serviceTime;
        event->teller->totalServiceTime += event->teller->serviceTime;
        event->teller->customersServed++;
    }
    if(arr[0]->length == 0){
        //No customer in queue
        event->eventType = TELLER_IDLE;
        event->teller->idleTime = (float)150 * rand()/(float)RAND_MAX;
        event->eventTime = globalTime + event->teller->idleTime;
        event->action = tellerActionC;
        addEQ(eq,event);
    }
    else{
        struct Event *newCustEvent = malloc(sizeof(struct Event));
        struct Customer *customerToServe = removeHeadTQ(arr[0]);
        customerToServe->serviceStartTime = globalTime;
        float serviceTime = 2 * avg_service_time * rand()/(float)RAND_MAX;
        customerToServe->serviceEndTime = customerToServe->serviceStartTime + serviceTime;
        customerToServe->tellerNumber = event->teller->tellerNumber;

        newCustEvent->customer = customerToServe;
        newCustEvent->teller = NULL;
        newCustEvent->eventTime = customerToServe->serviceEndTime;
        newCustEvent->eventType = SERVED_CUSTOMER;
        newCustEvent->action = servedCustomerAction;

        event->eventType = TELLER_SERVING;
        event->eventTime = customerToServe->serviceEndTime;
        event->action = tellerActionC;
        event->teller->serviceTime = serviceTime;
        addEQ(eq,event);
        addEQ(eq,newCustEvent);

        float waitTime = customerToServe->serviceStartTime - customerToServe->arrivalTime;
        if(maxWaitTime < waitTime){
            maxWaitTime = waitTime;
        }
    }
}

void initialiseCustomersOPT(int num_customers, float simulation_time, struct eventQueue *eq, struct Customer *customerArray[]){
    //This function takes an array of customer pointers, and initialises new customer events, and puts them in the queue
    for(int i = 0; i < num_customers; i++){
        struct Customer *customer = malloc(sizeof(struct Customer));
        customer->arrivalTime = simulation_time * rand()/(float)RAND_MAX;
        struct Event *event = malloc(sizeof(struct Event));
        event->eventTime = customer->arrivalTime;
        event->eventType = NEW_CUSTOMER;
        event->action = newCustomerActionOPT;           //Function pointer is assigned the function value
        event->customer = customer;
        event->teller = NULL;
        addEQ(eq,event);
        customerArray[i] = customer;
    }
}
void initialiseCustomersC(int num_customers, float simulation_time, struct eventQueue *eq, struct Customer *customerArray[]){
    for(int i = 0; i < num_customers; i++){
        struct Customer *customer = malloc(sizeof(struct Customer));
        customer->arrivalTime = simulation_time * rand()/(float)RAND_MAX;
        struct Event *event = malloc(sizeof(struct Event));
        event->eventTime = customer->arrivalTime;
        event->eventType = NEW_CUSTOMER;
        event->action = newCustomerActionC;
        event->customer = customer;
        event->teller = NULL;
        addEQ(eq,event);
        customerArray[i] = customer;
    }
}

void initialiseTellersOPT(int num_tellers, struct eventQueue *eq, struct Teller* tellerArray[]){
    //Takes an array of teller pointers, and initialises the tellers, as well as teller events

    for(int i = 0; i < num_tellers; i++){
        struct Teller *teller = malloc(sizeof(struct Teller));
        teller->idleTime = (float)600 * rand()/(float)RAND_MAX;         //Initial idel time is from 1-600 seconds
        teller->tellerNumber = i;
        struct Event *event = malloc(sizeof(struct Event));
        event->eventTime = globalTime + teller->idleTime;
        event->customer = NULL;
        event->teller = teller;
        event->action = tellerActionOPT;
        event->eventType = TELLER_IDLE;                             //Initially assumed to be in idle state
        teller->totalIdleTime = 0;
        teller->totalServiceTime = 0;
        teller->customersServed = 0;
        addEQ(eq,event);
        tellerArray[i] = teller;
    }
}
void initialiseTellersC(int num_tellers, struct eventQueue *eq, struct Teller* tellerArray[]){
    for(int i = 0; i < num_tellers; i++){
        struct Teller *teller = malloc(sizeof(struct Teller));
        teller->idleTime = (float)600 * rand()/(float)RAND_MAX;
        teller->tellerNumber = i;
        struct Event *event = malloc(sizeof(struct Event));
        event->eventTime = globalTime + teller->idleTime;
        event->customer = NULL;
        event->teller = teller;
        event->action = tellerActionC;
        event->eventType = TELLER_IDLE;
        teller->totalIdleTime = 0;
        teller->totalServiceTime = 0;
        teller->customersServed = 0;
        addEQ(eq,event);
        tellerArray[i] = teller;
    }
}

void initialiseTQOPT(struct tellerQueue *arr[], int num_tellers){
    //Initialises an array of empty teller queues
    for(int i = 0; i < num_tellers; i++){
        arr[i] = malloc(sizeof(struct tellerQueue));
        arr[i]->length = 0;
        arr[i]->head = NULL;
    }
}
void initialiseTQC(struct tellerQueue *arr[]){
    arr[0] = malloc(sizeof(struct tellerQueue));
    arr[0]->length = 0;
    arr[0]->head = NULL;
}

void print_stats(int num_customers, int num_tellers, char* queueingType){
    printf("Function pointer was used %d times in this simulation\n",func_pointer_log);
    printf("Total number of customers served: %d\n",num_customers);
    printf("Total Time taken to serve all the customers: %lf seconds\n",globalTime);
    printf("Number of tellers: %d. Queueing Type : %s\n",num_tellers,queueingType);
    printf("Average amount of time spent by a customer in the bank: %lf seconds\n",totalTimeSpentByCustomers/(float)num_customers);
    printf("Standard deviation of time spent by the customers: %lf seconds\n",standardDeviation);
    printf("Maximum Wait Time: %lf seconds\n",maxWaitTime);
    printf("Total teller service time: %lf seconds\n",totalTellerService);
    printf("Total teller idle time : %lf seconds\n",totalTellerIdle);
}

void OneQueuePerTeller(int num_customers, int num_tellers, float simulation_time, float avg_service_time){
    globalTime = 0, numCustomersServed = 0, totalTimeSpentByCustomers = 0, totalTellerIdle = 0, totalTellerService = 0, maxWaitTime = 0;
    standardDeviation = 0,func_pointer_log = 0;         //Re initialising all the global variables back to zero
    struct Customer* customerArray[num_customers];
    struct Teller* tellerArray[num_tellers];
    struct tellerQueue* TQArrayOPT[num_tellers];
    struct eventQueue *EQ = malloc(sizeof(struct eventQueue));
    EQ->head = NULL;
    EQ->length = 0;
    initialiseCustomersOPT(num_customers,simulation_time,EQ,customerArray);    //Customer events are initialised and put in the event queue
    initialiseTellersOPT(num_tellers,EQ,tellerArray);                                      //Teller events are initialised and put in the event queue
    initialiseTQOPT(TQArrayOPT,num_tellers);                                    //All tellerQueue pointers are initialised. They are empty lists as of now

    while(numCustomersServed < num_customers){                          //SImulation ends when all the customers are served
        struct Event *event = popEQ(EQ);
        //printf("At time %lf\n",event->eventTime);
        event->action(EQ,TQArrayOPT,event,num_tellers,avg_service_time); //Invoking the corresponding action of the event
        func_pointer_log++;
    }
    free(EQ);

//    for(int i = 0; i < num_tellers; i++){
//        printf("Teller Number %d:\n",i);
//        printf("Total Customers served: %d\n",tellerArray[i]->customersServed);
//        printf("Idle time: %lf seconds, Service Time: %lf seconds.\n",tellerArray[i]->totalIdleTime,tellerArray[i]->totalServiceTime);
//    }

    float avg_customer_time = totalTimeSpentByCustomers/(float)num_customers;
    float var = 0;
    for(int i = 0; i < num_customers; i++){
        float time_spent = customerArray[i]->serviceEndTime - customerArray[i]->arrivalTime;
        var += (avg_customer_time - time_spent) * (avg_customer_time - time_spent);
    }
    var = var/(float)num_customers;
    standardDeviation = sqrtf(var);
}
void CommonQueue(int num_customers, int num_tellers, float simulation_time, float avg_service_time){
    globalTime = 0, numCustomersServed = 0, totalTimeSpentByCustomers = 0, totalTellerIdle = 0, totalTellerService = 0, maxWaitTime = 0;
    func_pointer_log = 0;
    standardDeviation = 0;
    struct Customer* customerArray[num_customers];
    struct Teller* tellerArray[num_tellers];
    struct tellerQueue *TQArrayC[1];
    struct eventQueue *EQ = malloc(sizeof(struct eventQueue));
    EQ->length = 0;
    EQ->head = NULL;
    initialiseCustomersC(num_customers,simulation_time,EQ,customerArray);
    initialiseTellersC(num_tellers,EQ,tellerArray);
    initialiseTQC(TQArrayC);                        //The teller array in this case contains only the common queue
    while(numCustomersServed < num_customers){
        struct Event *event = popEQ(EQ);
        //printf("At time %lf\n",event->eventTime);
        event->action(EQ,TQArrayC,event,num_tellers,avg_service_time);
        func_pointer_log++;
    }
    free(EQ);

//    for(int i = 0; i < num_tellers; i++){
//        printf("Teller Number %d:\n",i);
//        printf("Total Customers served: %d\n",tellerArray[i]->customersServed);
//        printf("Idle time: %lf seconds, Service Time: %lf seconds.\n",tellerArray[i]->totalIdleTime,tellerArray[i]->totalServiceTime);
//    }

    float avg_customer_time = totalTimeSpentByCustomers/(float)num_customers;
    float var = 0;
    for(int i = 0; i < num_customers; i++){
        float time_spent = customerArray[i]->serviceEndTime - customerArray[i]->arrivalTime;
        var += (avg_customer_time - time_spent) * (avg_customer_time - time_spent);
    }
    var = var/(float)num_customers;
    standardDeviation = sqrtf(var);
}

int main(int argc, char *argv[]){
    char *end;
    int num_customers = strtol(argv[1],&end,10);
    int num_tellers = strtol(argv[2],&end,10);
    float simulation_time = 60 * strtof(argv[3],&end);
    float avg_service_time = 60 *strtof(argv[4],&end);              //Extracting the simulation parameters from the arguments given to main
    
    //srand(time(NULL));

    OneQueuePerTeller(num_customers,num_tellers,simulation_time,avg_service_time);
    print_stats(num_customers,num_customers,"One Queue per Teller");
    printf("\n---------------------------------------------------------------------------------------------------------\n");
    printf("---------------------------------------------------------------------------------------------------------\n");
    printf("---------------------------------------------------------------------------------------------------------\n\n");
    CommonQueue(num_customers,num_tellers,simulation_time,avg_service_time);
    print_stats(num_customers,num_tellers,"Common Queue");


    FILE *ptr = fopen("data.txt","w");
    for(num_tellers = 1; num_tellers <= 100; num_tellers++){
        CommonQueue(num_customers,num_tellers,simulation_time,avg_service_time);
        float avg_time = totalTimeSpentByCustomers/(float)num_customers;
        fprintf(ptr,"%d\t%lf\n",num_tellers,avg_time);
    }
    fclose(ptr);

    FILE *gnuplotpipe = popen("gnuplot -persistent","w");
    char *command[20];
    command[0] = "set term qt size 2000,1100 enhanced font \'Verdana,20\'";
    command[1] = "set xlabel \"Number of Tellers\"";
    command[2] = "set ylabel \"Average Time spent by customers (Seconds)\"";
    command[3] = "set autoscale x";
    command[4] = "set autoscale y";
    command[5] = "set grid";
    command[6] = "set style line 1 lc rgb \"red\" lw 1.5";
    command[7] = "set style line 2 lc rgb \"blue\" lw 1.5";
    command[8] = "plot \"data.txt\" using 1:2 with lines ls 1 title \"Average time spent v/s number of tellers\"";
    for(int i = 0; i <= 8; i++){
        if(i != 8){
            fprintf(gnuplotpipe,"%s\n",command[i]);
        }
        else{
            fprintf(gnuplotpipe,"%s, %lf %s\n",command[i],avg_service_time,"ls 2 title \"Average service time\"");
        }
    }
    fflush(gnuplotpipe);
    fclose(gnuplotpipe);
    return 0;
}