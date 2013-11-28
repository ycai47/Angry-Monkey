/**********************************
 * Author: Seungcheol Baek
 * Institution: Georgia Tech
 *
 * Title: MAIN
 * Class: ECE2035
 * Assignment: Project 2
 **********************************/

//includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <termios.h>

//defines
#define BAUDRATE            9600
#define BUFFSIZE            8192
#define GRAVITY             1.0
#define PHIGH               10
#define PLOW                5
#define PI                  3.141592653589793238462643f
#define ACK                 "ACK"
#define ACK_TIMEOUT         5
#define COM_ADDR            "socks/monkeys_socket"
#define UNIX_PATH_MAX       100


// rootNode def
 typedef struct Node
 {

    int row;
    int col;
    int con;
    Node* parent;
    Node* childA;
    Node* childB;
    Node* childC;

}Node;

typedef struct rootNode
{
    Node *data;
    rootNode *next; 

}rootNode;

// rootList define
typedef struct rootList
{
    rootNode *Head;
    rootNode *Tail;

}rootList;




//function prototypes
int  invert(int value);
void startGame(void);
int  waitForAck(void);
char get_pb_zxcvqr(void);
void pb1_hit_callback(void);
void pb2_hit_callback(void);
void pb3_hit_callback(void);
void pb4_hit_callback(void);
void getworld (int**world, unsigned char *World);
void updateShot(int row, int column, int del);
void colorTile(int row, int column, int strength);
void deleteTile(int row, int column);
void paaUpdate(int power, int angle);
void hint(int row, int column, int power, int angle);
void run_test_trajectory(int *world);
int* getTreeBoundry(int *world);
rootList* getRootList(int *world, int *RootColArray);
// void getTreeBoundry(int *world);
// Global variables for push buttons

char volatile power=PHIGH, angle=45, fire;
int connection_fd;

//main
int main() {

    START:    
    //variables
    unsigned char World[BUFFSIZE];

    //socket connection
    struct sockaddr_un address;
    int socket_fd;
    socklen_t address_length;
    pid_t child;

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0){
        printf("socket() failed\n");
        return 1;
    }

    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    snprintf(address.sun_path, UNIX_PATH_MAX, COM_ADDR);
    address_length = sizeof(address);
    connect(socket_fd, (struct sockaddr *) &address, address_length);
    connection_fd=socket_fd;

    struct timeval timeout;
    timeout.tv_sec = ACK_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(connection_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    /******** Project 2 *********/    
    //loop
    while(1) {
        //synchronize front end display  
        startGame();                            

        //receive World
        int result= recv(connection_fd,World,BUFFSIZE,0); 
        printf("recv: %d",result);
        sleep(1);
        fflush(stdout);    
        printf("received\n");
        
        //get world that will be used for your work
        int *world;
        getworld(&world, World);
        //clear the terminal
        fflush(stdout);
        rewind(stdout);
        printf("\033[2J\033[1;1H");
            printf("Angry Monkeys\n");
            printf("Push the buttons.\n");
            printf("Z - fire cannon\nX - decrease angle    C - increase angle\nV - toggle power\nR - reset    Q - quit\n");

/* initialize enviroment start from here   */

            int* RootColArray = getTreeBoundry(world);  
           // for debug

            int m_size=sizeof(RootColArray)/sizeof(int);
            printf("[DEBUG] m_size :  %d\n", m_size);
            for (int i = 0; i < m_size; ++i)
            {

                printf("[DEBUG] one tree @ col : %d\n", *(RootColArray+i) );

            }


            rootList* m_rootlist=getRootList(world,RootColArray);
/************** debug purpuse to check rootlist ******************/

            rootNode* log_rootNode=m_rootlist->Head;

            while(log_rootNode!=NULL){

                Node* log_node=log_rootNode->data;
                printf("[DEBUG] current Root, row @ %d, col @ %d. \n", log_node->row, log_node->col );

                log_rootNode=log_rootNode->next;

            }

            
        /****   BEGIN - your code goes here for project 2  ****/

            int i, num_cannon=10;
            char pb;

        //get pb
            while(1){
            // it's basically impossible to get a keyboard to function in the same way as mbed pushbuttons so...
            // the get_pb_zxcvqr() function returns the character of the next keyboard button pressed
                pb=get_pb_zxcvqr();
            // and then based on that character, you can do something useful!
                if(pb=='z'){
                    printf("Z was pressed: FIRE!!!\n");
                    pb4_hit_callback(); 
                    printf("Trajectory not yet computed...showing a canned one.\n");
                    run_test_trajectory(world);
                } else if(pb=='x'){
                    printf("X was pressed: decreasing angle\n");
                    pb3_hit_callback(); 
                    if(power==PHIGH)
                        printf("Angle:%d PHIGH\n", angle);
                    else
                        printf("Angle:%d PLOW\n", angle);
                } else if(pb=='c'){
                    printf("C was pressed: increasing angle\n");
                    pb2_hit_callback(); 
                    if(power==PHIGH)
                        printf("Angle:%d PHIGH\n", angle);
                    else
                        printf("Angle:%d PLOW\n", angle); 
                } else if(pb=='v'){
                    printf("V was pressed: toggling power\n");
                    pb1_hit_callback(); 
                    if(power==PHIGH)
                        printf("Angle:%d PHIGH\n", angle);
                    else
                        printf("Angle:%d PLOW\n", angle); 
                } else if(pb=='q'){
                   printf("EXIT\n");
                   free(world);
                   close(socket_fd);
                   exit(1);
               }  else if(pb=='r'){
                printf("RESTART\n");
                free(world);
                close(socket_fd);
                goto START;         
            }  else {
               printf("testing\n");
               printf("string1: %s\nstring2: %s\n", "hello", "world");
               printf("int: %d, int: %d\n", world[2], world[3]); 
               printf("Shots left:%d\n", num_cannon);
               if(power==PHIGH)
                printf("Angle:%d PHIGH\n", angle);
            else
                printf("Angle:%d PLOW\n", angle);
        }
    }
    

        //have fun... 

        /****    END - your code stops here   ****/
    free(world);  
    close(socket_fd);
}
    //end loop

}

void run_test_trajectory(int *world){
  //Just for test...
  int i;
  for(i=1;i<10;i++){
    updateShot(i-1,i,0);
    sleep(1);
    hint(i+2, i+3,2,1);
} 

for(i=0;i<5;i++){       
    deleteTile(world[4*i+2],world[4*i+3]);
    updateShot(i-1,i,1);
}
}

//fcn to send update
void updateShot(int row, int column, int del){
    //temp variables
    char buffer[BUFFSIZE];
    
    //construct message
    sprintf(buffer, "%s-%d-%d-%d;", "update", row, column, del);

    //send message
    send(connection_fd, buffer, strlen(buffer),0);
    waitForAck();
}

//fcn to send color
void colorTile(int row, int column, int strength){
    //temp variables
    char buffer[BUFFSIZE];
    
    //construct message
    sprintf(buffer, "%s-%d-%d-%d;", "color", row, column, strength);

    //send message
    send(connection_fd, buffer, strlen(buffer),0);
    waitForAck();
}    

//fcn to send delete
void deleteTile(int row, int column){
    //temp variables
    char buffer[BUFFSIZE];
    
    //construct message
    sprintf(buffer, "%s-%d-%d;", "delete", row, column);

    //send message
    send(connection_fd, buffer, strlen(buffer),0);
    waitForAck();
} 

//fcn to send power and angle
void paaUpdate(int power, int angle){
    //temp variables
    char buffer[BUFFSIZE];
    
    //construct message
    sprintf(buffer, "%s-%d-%d;", "paa", power, angle);

    //send message
    send(connection_fd, buffer, strlen(buffer),0);
    waitForAck();
} 

//fcn to send hint
void hint(int row, int column, int power, int angle){
    //temp variables
    char buffer[BUFFSIZE];
    
    //construct message
    sprintf(buffer, "%s-%d-%d-%d-%d;", "hint", row, column, power, angle);

    //send message
    send(connection_fd, buffer, strlen(buffer),0);
    waitForAck();
}

//fcn to get acknowledgement from serial peripheral
int waitForAck(void) {
    //get acknowlegement
    char buffer[BUFFSIZE];
    double elapsed;
    time_t start;
    time_t now;
    time(&start);
    while(1) {
        memset(&buffer[0],0,strlen(buffer));  
        recv(connection_fd,buffer,BUFFSIZE-1,0);
        if(strncmp(ACK, buffer, strlen(ACK)) == 0) {
            break;
        }
        memset(&buffer[0],0,strlen(buffer));     
        time(&now);
        elapsed = difftime(now, start);
        //printf("%.f, ", elapsed);
        fflush(stdout);
        if(elapsed >= ACK_TIMEOUT)
            return 1;
    }
    return 0;
}

//fcn to initialize the frontend display
void startGame(void) {
    //temp variables
    char buffer[BUFFSIZE];

    //construct message
    sprintf(buffer, "start");

    //send message
    send(connection_fd, buffer, strlen(buffer),0);

    //wait for acknowledgement
    waitForAck();
}

//function to perform bitwise inversion
int invert(int value) {
    if (value == 0) {
        return 1;
    } else {
        return 0;
    }
}

char get_pb_zxcvqr(void) {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror ("tcsetattr ~ICANON");
    return (buf);
}


// Callback routine is interrupt activated by a debounced pb hit
void pb1_hit_callback (void) {
    if(power==PHIGH)
        power=PLOW;
    else 
        power=PHIGH;
}
void pb2_hit_callback (void) {
    if(angle<90)
        angle++;
    else if(angle>=90) 
        angle=0;
}
void pb3_hit_callback (void) {
    if(angle>0)
        angle--;
    else if(angle<=0)
        angle=90;
}
void pb4_hit_callback (void) {
    fire++;
}

//func. to get world
void getworld (int**world, unsigned char *World){
    int i;
    char temp[3];
    
    //allocate world
    *world = (int*)malloc(sizeof(int)*(((World[2]<<8)+World[3])*4+2));
    
    //get it
    (*world)[0]=(World[0]<<8)+World[1];
    (*world)[1]=(World[2]<<8)+World[3];
    for(i=0;i<((*world)[1]*4);i++){
        temp[0] = World[(2*i)+4];
        temp[1] = World[(2*i)+5];
        temp[2] = '\0';   
        sscanf(temp, "%d", &((*world)[i+2]));            
    }
}


// this is for getting the tree boundry.so we can find the Root object 
// 1.   we need to find out all tree cols, only need the tree cols. 
//      after that, sorting cols, we can find the not continuous cols, 
int* getTreeBoundry(int *world){
//void  getTreeBoundry(int *world){
    // contain not duplicate col # make it a array
    int *treeColContainer=(int *)malloc(20* sizeof(int ));
    int treeColContainerSize=0;

    if (treeColContainer==NULL)
    {
        printf("Cannot access memory at address IN container");
        return NULL;
    }

    int loopSize=world[1];
   // printf("[DEBUG] world size is %d\n", loopSize);
    int CurrentType;
    int CurrentCol;
    bool containerInsideValid;
    // search whole array to find it is a tree type
    for (int i = 0; i < loopSize; ++i)
    {   
        containerInsideValid=false;  // assume it is not in there 
        CurrentType=world[i*4+4];
        //printf("[DEBUG] CurrentType is %d\n", CurrentType);
        // if it is a tree type
        if (CurrentType==84)
        {
            // check it's col whether in the container or not. 
            CurrentCol=world[i*4+3];
           // printf("[DEBUG] CurrentCol is %d \n", CurrentCol);
            if (treeColContainerSize==0)
            {

                treeColContainer[0]=CurrentCol;
                treeColContainerSize++;

            }else {

                // search whole container array, if the col # in here.

                for (int j = 0; j < treeColContainerSize; ++j)
                {
                    int temp=*(treeColContainer+j);
                    if (CurrentCol==temp)
                    {   
                        containerInsideValid=true; // in the container
                        break;  //break for loop
                    }

                }

                // end of search, if in the container, break out and 
                // insidevalid = true; or no in there is false. 

                if (!containerInsideValid)  // if is false, write it in
                {

                    *(treeColContainer+treeColContainerSize)=CurrentCol;
                    treeColContainerSize++;

                }
                // or nothing happend, go to next one. 
            }
        }
        // not tree? nothing happend. 

    }

    // end of world loop, all unique tree col should be here now. 
    // sort it

    // log out for debug purpose
    // work
    // for (int i = 0; i < treeColContainerSize; ++i)
    // {
    //     printf("[DEBUG] one tree @ col : %d\n", *(treeColContainer+i) );
    // }

    // start sort it . 
    // use bubble sort
    int m_swap;
    for (int i = 0; i < treeColContainerSize-1 ; ++i)
    {

        for (int j = 0; j < treeColContainerSize-i-1; ++j)
        {

            if (*(treeColContainer+j) > *(treeColContainer+j+1)){
                m_swap = *(treeColContainer+j);
                *(treeColContainer+j)=*(treeColContainer+j+1);
                *(treeColContainer+j+1) = m_swap;
            }

        }
    }

    int m_BoundrySize = (treeColContainerSize==0) ? 1 : 2 ;
  //      printf("[DEBUG] B size%d\n", m_BoundrySize );

    for (int i = 1; i < treeColContainerSize-1; ++i)
    {
        int m_cur=treeColContainer[i];
 //       printf("[DEBUG] %d\n", m_cur );
        if (m_cur==((treeColContainer[i+1])-1) && (m_cur== ((treeColContainer[i+2])-2)))
        {   
            // if continous, 
            treeColContainer[i]=-1;

        }else {
            m_BoundrySize++;
        }

    }
    
    
    // Answer granted, we get correct tree boundry. 
    // return as
    // return it's treeboundry size. 
 //   printf("size is %d\n", m_BoundrySize);
    int *returnPointer = (int *)malloc( m_BoundrySize * sizeof(int));    

    if(!returnPointer){
        printf("malloc failed\n");
        return NULL;
    }

    for (int i = 0; i < treeColContainerSize; ++i)
    {
        if ( *(treeColContainer+i)!= -1)
        {

            returnPointer[i]=treeColContainer[i];
        }
    }

    // cut the tree, to get  the rootCol. 
    for (int i = 0; i < m_BoundrySize; i+=2)
    {
        returnPointer[i]=returnPointer[i]-1;
    }
    for (int i = 1; i < m_BoundrySize; i+=2)
    {
        returnPointer[i]=returnPointer[i]+1;

    }
    // for (int i = 0; i < m_BoundrySize; ++i)
    // {
    //     printf("[DEBUG] one tree @ col : %d\n", returnPointer[i] );
    // }
    // // until here, we have a array
    return returnPointer;
}



// in here, we have Root Col, so we need to search to world, 
// identify the root item, which its col# matched 
// store them into a struct, and link it into list. 

rootList* getRootList(int *world, int *RootColArray){

    // in world, store whole raw data, RootColArray contain all rootCol number

    // search all world, find match. 

    rootList *m_rootlist=(rootList*)malloc(sizeof(rootList));
    m_rootlist->Head=NULL;
    m_rootlist->Tail=NULL;


    int m_rootBoundSize= sizeof(RootColArray)/sizeof(int);
    int m_worldSize=world[1];

    for (int i = 0; i < m_rootBoundSize; ++i)
    {
        int m_colNum = RootColArray[i];

        for (int j = 0; j < m_worldSize; ++j)
        {
            int m_currentItemCol = world[4*j+3];
            //find match
            if (m_currentItemCol==m_colNum)
            {

                // first create a Node,  TODO
                // assume Node is create, call m_node
                Node* m_node=(Node*)malloc(sizeof(Node));
                m_node->row=world[4*j+2];
                m_node->col=world[4*j+3];
                m_node->con=-1;         // let -1 be init
                m_node->parent=NULL;
                m_node->childA=NULL;
                m_node->childB=NULL;
                m_node->childC=NULL;
                // initialize node

                // then create a rootNode, to contain Node
                rootNode* m_curRoot=(rootNode*)malloc(sizeof(rootNode));
                m_curRoot->data=m_node;
                m_curRoot->next=NULL;
                // initial done, 
                
                // link into List

                // at head, 
                if (m_rootlist->Head==NULL)
                {
                    m_rootlist->Head=m_curRoot;
                    m_rootlist->Tail=m_curRoot;
                }else{
                    // not at head, 
                    m_rootlist->Tail->next=m_curRoot;
                    m_rootlist->Tail=m_curRoot;
                }
                // done linking, 
            }
        }        
    }
    // that's it. 
    
    // this part is for debug purpuse

    // rootNode* log_rootNode=m_rootlist->Head;

    // while(log_rootNode!=NULL){

    //     Node* log_node=log_rootNode->data;
    //     printf("[DEBUG] current Root, row @ %d, col @ %d. \n", log_node->row, log_node->col );

    //     log_rootNode=log_rootNode->next;

    // }

    return m_rootlist;


}