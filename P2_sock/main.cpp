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
    int visit;
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

// this NodeContainer can replace above def
typedef struct nodeContainer
{
    Node *data;
    nodeContainer *next; 

}nodeContainer;

typedef struct nodeList
{
    nodeContainer *Head;
    nodeContainer *Tail;

}nodeList;


typedef struct listContainer
{
    nodeList *data;
    listContainer *next; 

}listContainer;


typedef struct listList
{
    listContainer *Head;
    listContainer *Tail;

}listList;

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
int*  getContent(int *world);
listList* getBranchList(Node* root, int* content);
listList* conbineBranchHead(rootList* m_rootlist, int* ptr_Content);

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




            // printf("start the test Content\n");

            int* ptr_Content=getContent(world);
            int* RootColArray = getTreeBoundry(world); 

            // int tmp_size=ptr_Content[0];
            // printf("[DEBUG] contentSize %d\n", tmp_size);
            // for (int i = 0; i < tmp_size; ++i)
            // {
            //     printf("[DEBUG] One Object  row @ %d, col @ %d , type is %d \n", ptr_Content[4*i+1], ptr_Content[4*i+2], ptr_Content[4*i+3] );
            // }

            // for debug
            // int m_size= RootColArray[0];
            // printf("[DEBUG] m_size :  %d\n", m_size);
            // for (int i = 1; i <= m_size; ++i)
            // {

            //     printf("[DEBUG] one tree @ col : %d\n", *(RootColArray+i) );

            // }

/************** debug purpose to check rootlist ******************/
            rootList* m_rootlist=getRootList(world,RootColArray);
            rootNode* log_rootNode=m_rootlist->Head;

            while(log_rootNode!=NULL){

                Node* log_node=log_rootNode->data;
                printf("[DEBUG] current Root, row @ %d, col @ %d. \n", log_node->row, log_node->col );

                log_rootNode=log_rootNode->next;

            }

/************* debug purpose to check branchlist *****************/

        listList* m_llist = conbineBranchHead(m_rootlist,  ptr_Content);

        //listList* m_llist = getBranchList(m_rootlist->Head->data, ptr_Content);

        // listContainer* m_loglist = m_llist->Head;
        // while(m_loglist!=NULL){
            
        //         nodeContainer* m_debug = m_loglist->data->Head;
        //         while(m_debug!=NULL){

        //             printf("[DEBUG] Connection @ row: %d, col: %d\n", m_debug->data->row, m_debug->data->col);
        //             m_debug=m_debug->next;
        //         }

        //         m_loglist = m_loglist->next; 

        // }


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


    int *ptr_root = (int *)malloc( (2*treeColContainerSize) * sizeof(int));    
    for (int i = 0; i < treeColContainerSize; ++i)
    {

        int m_cur=treeColContainer[i];
        
        ptr_root[2*i+0] = m_cur - 1;
        ptr_root[2*i+1] = m_cur + 1;


    }

    int *m_returnPointer = (int *)malloc( (2*treeColContainerSize) * sizeof(int)); 

    if(!m_returnPointer){
        printf("malloc failed\n");
        return NULL;
    }

    int index= 0;
    
    for (int i = 0; i < 2*treeColContainerSize; ++i)
    {
        int m_current= ptr_root[i];
        bool dirty = false;
        for (int j = 0; j < index; ++j)
        {   
            if (m_returnPointer[j]==m_current)
            {
                dirty=true;
                break;
            }

        }

        for (int i = 0; i < treeColContainerSize; ++i)
        {

            if (m_current==treeColContainer[i])
            {
                dirty=true;
                break;
            }
        }


        if (dirty==false)
        {
            m_returnPointer[index]=m_current;
            index++;
        }

    }

    int *returnPointer = (int * )malloc( (index+1) * sizeof(int ));
    returnPointer[0] = index;
    memcpy(returnPointer+1, m_returnPointer, index*sizeof(int));
    printf("[DEBUG] total tree boundary size : %d \n", returnPointer[0]);
    for (int i = 1; i <= index; ++i)
    {
        printf("[DEBUG] one tree @ col : %d\n", returnPointer[i] );
    }
    // // until here, we have a array
    //free treecontainer, avoid memery leak
    free(treeColContainer);
    free(ptr_root);
    free(m_returnPointer);

    return returnPointer;
}



// in here, we have Root Col, so we need to search to world, 
// identify the root item, which its col# matched 
// store them into a struct, and link it into list. 

rootList* getRootList(int *world, int *RootColArray){

    // in world, store whole raw data, RootColArray contain all rootCol number

    // search all world, find match. 
    printf("[DEBUG] Stepped into getRootList function\n" );
    rootList *m_rootlist=(rootList*)malloc(sizeof(rootList));
    m_rootlist->Head=NULL;
    m_rootlist->Tail=NULL;


    int m_rootBoundSize= RootColArray[0];
    int m_worldSize=world[1];

    for (int i = 1; i < m_rootBoundSize+1; ++i)
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
                m_node->visit=-1;         // let -1 be init
                m_node->con=-1;
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

    //  rootNode* log_rootNode=m_rootlist->Head;

    // while(log_rootNode!=NULL){

    //     Node* log_node=log_rootNode->data;
    //     printf("[DEBUG] current Root, row @ %d, col @ %d. \n", log_node->row, log_node->col );

    //     log_rootNode=log_rootNode->next;

    // }

    return m_rootlist;


}

// this will return anything is not tree, so including branches, and monkeys
// just for easy development

int* getContent(int *world){
// void  getContent(int *world){

    // everything, including tree, branches, and monkeys;
    int allSize=world[1];
    int* ptr_contain=(int *)malloc(4*allSize*sizeof(int));
    int contentSize=0;
    int CurrentType;
    for (int i = 0; i < allSize; ++i)
    {
        CurrentType=world[4*i+4];
        if (CurrentType!=84)
        {
            memcpy((ptr_contain+4*contentSize), (world+4*i+2),4*sizeof(int));
            contentSize++;

        }

    }
    // printf("[DEBUG] allSize %d\n", allSize);
    printf("[DEBUG] contentSize %d\n", contentSize);


    int* ptr_return=(int *)malloc((4*contentSize+1)*sizeof(int));
    if (ptr_return==NULL)
    {
        printf("malloc failed\n");
        return NULL;
    }
    ptr_return[0]=contentSize;
    printf("size is %d \n",  ptr_return[0]);
    memcpy(ptr_return+1,ptr_contain, 4*contentSize*sizeof(int));
    free(ptr_contain);

    // for (int i = 0; i < contentSize; ++i)
    // {
    //     printf("[DEBUG] One Object  row @ %d, col @ %d , type is %d \n", ptr_return[4*i+1], ptr_return[4*i+2], ptr_return[4*i+3] );
    // }

    return ptr_return;


}



// input one root, output a list 

listList* getBranchList(Node* root, int* content){

    listList* m_llist=(listList*)malloc(sizeof(listList));
    m_llist->Head = NULL;
    m_llist->Tail = NULL;

    int contentSize=content[0];
            
    Node *m_node=root;
            // new object, mark -1 as initialized
    m_node->visit = -1;
    m_node->con = -1;

    do{



                    // if it is return from its children, skip the find children part. 
        if (m_node->con == -1){

            m_node->visit = 0;
            m_node->con = 0;
            //printf("[DEBUG] now Coordinate is at row@ %d, col@ %d\n",m_node->row, m_node->col );
                        // find children. 
            for (int i = 0; i < contentSize; ++i)
            {   
                int m_row = content[4*i+1];
                int m_col = content[4*i+2];
                //printf("[DEBUG] currentPos row@ %d, col@ %d\n", m_row, m_col);
                if (m_row==4 && m_col==19)
                {
                //printf("[DEBUG] currentPos 4,19");
                //printf("testing: %d\n",((m_node->col == m_col+1 ) || (m_node->col == m_col - 1 )) && (m_node->row == m_row) );

                }
                            // find match connection Node
                if (    ( ((m_node->row == m_row+1 ) || (m_node->row == m_row - 1 )) && (m_node->col == m_col) ) || 
                    ( ((m_node->col == m_col+1 ) || (m_node->col == m_col - 1 )) && (m_node->row == m_row) )    ) {

                            // whether connected already or not. 
                printf("[DEBUG] stepped inside \n");
                            // is that parent, 
                    if (m_node->parent != NULL) 
                    {
                         printf("[DEBUG] stepped inside parent \n");
                        if (m_row == m_node->parent->row && m_col == m_node->parent->col)
                        {
                            continue;
                        }
                                // skip this one. 
                    }
                    m_node->visit++;
                    m_node->con++;
                            // create a object and connected into the child
                    Node* m_match =(Node *)malloc(sizeof (Node));
                    m_match->row    = m_row;
                    m_match->col    = m_col;
                    m_match->visit  = -1;
                    m_match->con    = -1;
                    m_match->parent = m_node;

                    switch(m_node->con){

                        case 1:
                        m_node->childA=m_match;
                        break;
                        case 2:
                        m_node->childB=m_match;
                        break;
                        case 3:
                        m_node->childC=m_match;
                        break;
                        default:
                        printf("[DEBUG] con value is not expected %d, at (%d,%d)\n", m_node->con, m_node->row,m_node->col);
                        break;

                    }

                //   break;
                }
            }

                        // now, connected every children done. 
        }


            // wait, if there is no connection, mean need to trace back it's parent, TODO


            // con is null then, no connection, con = null visit all children 

        if (m_node->con==0)
        {
                        //  TODO get a branchHead, traceback the whole thing. 
                        //  create a Single Direction list to store this branch. 
                        //  and NodeContainer.         


                            // [DEBUG] maybe cause the pointer disappear. 
            nodeList* m_branchlist=(nodeList*)malloc(sizeof(nodeList));
            nodeContainer* m_nodeContainer=(nodeContainer*)malloc(sizeof(nodeContainer));                
            m_nodeContainer->data = m_node;
            m_nodeContainer->next = NULL;

            m_branchlist->Head = m_nodeContainer;
            m_branchlist->Tail = m_nodeContainer;

            while(m_branchlist->Tail->data != root){
                nodeContainer* next_nodeContainer=(nodeContainer*)malloc(sizeof(nodeContainer));
                next_nodeContainer->data = m_nodeContainer->data->parent;
                next_nodeContainer->next = NULL;
                m_nodeContainer->next = next_nodeContainer;
                m_nodeContainer = next_nodeContainer;
                m_branchlist->Tail = m_nodeContainer;
            }

                listContainer* m_listHead = (listContainer*)malloc(sizeof(listContainer));
                m_listHead->data = m_branchlist;
                m_listHead->next = NULL;

            if (m_llist->Head==NULL)
            {
                printf("add the first branchlist.\n");
                m_llist->Head = m_listHead;
                m_llist->Tail = m_listHead;

            //     nodeContainer* m_debug = m_llist->Head->data->Head;
            //     while(m_debug!=NULL){

            //     printf("[DEBUG] Connection @ row: %d, col: %d\n", m_debug->data->row, m_debug->data->col);
            //     m_debug=m_debug->next;
            // }

            }else{
                printf("add the another branchlist at Tail.\n");
                m_llist->Tail->next = m_listHead;
                m_llist->Tail = m_listHead;

            //      nodeContainer* m_debug = m_llist->Tail->data->Head;
            //     while(m_debug!=NULL){

            //     printf("[DEBUG] Connection @ row: %d, col: %d\n", m_debug->data->row, m_debug->data->col);
            //     m_debug=m_debug->next;
            // }

            }

                        //  after there, m_branchlist is return one link as a branch head
                        //  so we are going to trace back, for security visit mark as -1 so
                        //  we say job done here. 

                            //m_node->visit --;
            m_node = m_node->parent;

        }else{

            if (m_node->visit == 0)
            {
                                // go back it's parent, 
                                //m_node->visit --;       // for debug
                m_node = m_node->parent;

            }else{
                                    //visit conCount - con +1 parent
                int visitType =m_node->con - m_node->visit;

                switch(visitType){

                    case 0 :
                                                // visit childA
                    m_node->visit--;
                    m_node=m_node->childA;
                    break;
                    case 1 :
                                                // visit childB
                    m_node->visit--;                
                    m_node=m_node->childB;
                    break;
                    case 2:
                                                // visit childC
                    m_node->visit--;
                    m_node=m_node->childC;
                    break;
                    default:
                    printf("[DEBUG] shouldn't happen.\n");
                    break;

                }

            }

        }


    }while(m_node!=root &&  root->visit==0 );
        // until it roll back to root. and visited all root children. 

    // listContainer* m_loglist = m_llist->Head;
    // while(m_loglist!=NULL){
        
    //         nodeContainer* m_debug = m_loglist->data->Head;
    //         while(m_debug!=NULL){

    //             printf("[DEBUG] Connection @ row: %d, col: %d\n", m_debug->data->row, m_debug->data->col);
    //             m_debug=m_debug->next;
    //         }

    //         m_loglist = m_loglist->next; 

    // }

    return m_llist;

}


    //branch condition

// get a rootlist, then this function, 
// use every root, get one list, and combine each list
listList* conbineBranchHead(rootList* m_rootlist, int* ptr_Content){

    // final return 
    listList* m_llist=(listList*)malloc(sizeof(listList));
    m_llist->Head = NULL;
    m_llist->Tail = NULL;

    listList* m_currentList;

    rootNode* m_root=m_rootlist->Head;
    while(m_root!=NULL){

        Node* m_data=m_root->data;
        printf("[DEBUG]  current root @ %d,%d\n",m_data->row,m_data->col );
        m_currentList=getBranchList(m_data,ptr_Content);
        // read everything from head to tail store into m_llist

        listContainer* m_current= m_currentList->Head;
        while(m_current != NULL){

            if (m_llist->Head == NULL)
            {
                m_llist->Head = m_current;
                m_llist->Tail = m_current;
            }else{
                m_llist->Tail->next =m_current;
                m_llist->Tail = m_current;
            }
            m_current=m_current->next;
        }
        m_root=m_root->next;

    }

    return m_llist;

}