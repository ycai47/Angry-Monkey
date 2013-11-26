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
#define COM_ADDR	        "socks/monkeys_socket"
#define UNIX_PATH_MAX       100


#include <iostream>
//using namespace std;

//define a node with same structure as given world array.
typedef struct Node{
    int column;
    int row;
    int type;
    int strength;
    Node *next;
}Node;
//define a node for list of branches
typedef struct nodeHeadList{
    Node *oneHead;
    nodeHeadList *next;
}nodeHeadList;

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
void run_trajectory(int *world,nodeHeadList *branches);
int Length(int* array);
int* searchtree(int *world);
int* treeboundaries(int *treearray);
Node* Brancharray(int columns,int columne,int *world);
nodeHeadList* connectnode(int *world , Node **rootList, int *Left, int *Right);
void detection(int x, int y, nodeHeadList *branches);
Node* spotmokeyroot(nodeHeadList *branches, int *world);



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
        recv(connection_fd,World,BUFFSIZE,0); 
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
     
        int i, num_cannon=10;
        int *treearray;
        int *treeBoundaries;
        treearray = searchtree(world);
        treeBoundaries = treeboundaries(treearray);
        int *treeLeft;
        int *treeRight;
        int l = 0;
        //store the column left/right of tree trunk as treeLeft/treeRight including left/right margins
        i=0;
        treeRight[0] = 0;
        while (treeBoundaries[i]!= NULL){
                treeLeft[l] = treeBoundaries[i]-1;
                treeRight[l+1] = treeBoundaries[i]+1;
            i++;
        }
        treeLeft[i+1] = sqrt(world[0])-1;
        Node **rootList;
        //create a linked list of branch roots connected to tree trunk
        int j,indexin = 0;
        while (indexin < world[1]){
            for (j=0; j<i; j++){
                if ((world[indexin*4+3] == treeLeft[j]) || (world[indexin*4+3] == treeRight[j]) && (world[indexin*4+4] == 66)){
                    Node *m_Root= (Node *) malloc(sizeof(Node));
                    m_Root->column = world[indexin*4+3];
                    m_Root->row = world[indexin*4+2];
                    m_Root->type = world[indexin*4+4];
                    m_Root->strength = world[indexin*4+5];
                    m_Root->next = NULL;
                    *rootList = m_Root;
                }
            }
            indexin++;
        }
        nodeHeadList* branches = connectnode(world,rootList,treeLeft,treeRight);
        Node* root = spotmokeyroot(branches,world);
        int x = root->row;
        int y = root->column;
        //estimate shooting angle
        int Angle = y/x;
        angle = floor(Angle);
        //calculate power based on estimation
        double Power = sqrt(GRAVITY*x*x/(cos(Angle)*cos(Angle))/2/(tan(Angle)*x-y));
        int r = 0;
        if (Power > 7.5){
            power = PHIGH;
            while(r == 0){
                if (y == floor(sin(angle)/cos(angle)*x - GRAVITY*(x/(power*cos(angle)))*(x/(power*cos(angle)))/2)){
                    r = 1;
                }
                else {
                    angle++;
                }
            }
        }
        else {power = PLOW;
            while(r == 0){
                if (y == floor(sin(angle)/cos(angle)*x - GRAVITY*(x/(power*cos(angle)))*(x/(power*cos(angle)))/2)){
                    r = 1;
                }
                else {
                    angle--;
                }
            }
        }
        //get accurate angle and feed into hint fuction
        hint(x,y,power,angle);
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
		            run_trajectory(world,branches);
            } else if(pb=='x'){
                    printf("X was pressed: decreasing angle\n");
                    pb3_hit_callback(); 
                    paaUpdate(power,angle);
                    if(power==PHIGH)
                            printf("Angle:%d PHIGH\n", angle);
                    else
                            printf("Angle:%d PLOW\n", angle);
            } else if(pb=='c'){
                    printf("C was pressed: increasing angle\n");
                    pb2_hit_callback(); 
                    paaUpdate(power,angle);
                    if(power==PHIGH)
                            printf("Angle:%d PHIGH\n", angle);
                    else
                            printf("Angle:%d PLOW\n", angle); 
            } else if(pb=='v'){
                    printf("V was pressed: toggling power\n");
                    pb1_hit_callback(); 
                    paaUpdate(power,angle);
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
        free(*rootList);
        free(branches);
        free(world);  
        close(socket_fd);
    }
    //end loop
}

int Length(int *array){
    //find length of pointer-defined arrays
    int length = 0;
    while (array[length]!= NULL){
            length++;
    }
    return length;
}

int* searchtree(int *world){
    //find all columns of squares of tree trunk in ascending order
    int index=0;
    int length=0;
    while(index<=world[1]){
        //find total number of tree trunk squares
        if (world[index*4+4] == 84){
            length +=1;
        }
        index++;
    }
    int *treeArray;
    index = 0;
    int l=0;
    while(index<=world[1]){
        //store all column numbers of tree squares in an array
        if (world[index*4+4] == 84){
            treeArray[l] = world[index*4+3];
            l++;
        }
        index++;
    }
    int c,d,swap;
    for (c=0;c<(length-1);c++){
        //bubble sort the array in ascending order
        for (d=0;d<(length-c-1);d++){
            if (treeArray[d]>treeArray[d+1]){
                swap = treeArray[d];
                treeArray[d] = treeArray[d+1];
                treeArray[d+1] = swap;
            }
        }
    }
    return treeArray;
}

int* treeboundaries(int *treearray){
    //find the boudaries(columns) of trees
    int i,treeBound = 1;
    int length = Length(treearray);
    int *treeBoundaries;
    int l=0;
    for (i=0;i<(length);i++){
        //store boundaries of trees to array treeBoundaries
        if (treearray[i+1] != treearray[i]){
            treeBoundaries[l] = treearray[i+1];
            l++;            
        }
    }
    return treeBoundaries;
}

Node* Brancharray(int columns,int columne, int *world){
    //sort objects between two columns (including those two column) according to ascending column. ascending row
    int length = 0;
    int i,j;
    if (columns<columne){
        for (i=columns; i<=columne; i++){
            //total objects(branches and monkeys) in the range stored in length
            while (j<=world[1]){
                if ((world[j*4+3]==i) && ((world[j*4+4]==66) || (world[j*4+4]==77))){
                    length++;
                }
                j++;
            }
        }
        Node *branchArray = (Node *) malloc(length*sizeof(Node));
        //create an node array
        int m,n,k = 0;
        Node *swap;
        for (i=columns; i<=columne; i++){
            //store and sort all location pairs in array
            int l=k;
            while (j<world[1]){
                //store pairs
                if ((world[j*4+3]==i) && ((world[j*4+4]==66) || (world[j*4+4]==77))){
                    (branchArray+k)->row = world[4*j+2];
                    (branchArray+k)->column = i;
                    (branchArray+k)->type = world[4*j+4];
                    (branchArray+k)->strength = world[4*j+5];
                    k++;
                }
                j++;
            }
            //bubble sort row numbers under each desired column
            for (m=l;m<(k-1);m++){
                for (n=l;n<(k-m-1);n++){
                    if ((branchArray+n)->row > (branchArray+n+1)->row){
                        memcpy(swap, (branchArray+n),sizeof(Node));
                        memcpy((branchArray+n),(branchArray+n+1),sizeof(Node));
                        memcpy((branchArray+n+1),swap,sizeof(Node));
                    }
                }
            }
            for (j=l;j<k;j++){
                (branchArray+j)->next = (branchArray+j+1);
            }
        }
        return branchArray;
    }
    else{
        return NULL;
    }
}

nodeHeadList* connectnode(int *world , Node **rootList, int *Left, int *Right){
    //create a list of branches
    //if left of a tree trunk(between previous right and new left), connect to left/up/down
    //if right of a tree trunk(between this right and next left), connect to right/up/down
    int j,length = Length(Left);
    nodeHeadList *branches;
    Node *Array;
    for(j=0;j<length;j++){
        Array = Brancharray(Right[j],Left[j],world); 
        Node *m_current = *rootList;//head
        int i = 0;
        int size = 0;
        int length = sizeof(rootList)/sizeof(void *);
        while(size<length){
                //loop through rootList
            while ((Array+i)->next != NULL){
                if (((Array+i)->row)==(m_current->row) && ((Array+i)->column -1) == (m_current->column) /*left*/ ||
                    ((Array+i)->row)==(m_current->row) && ((Array+i)->column +1) == (m_current->column) /*right*/ ||
                    ((Array+i)->row +1) == (m_current->row) && ((Array+i)->column) == (m_current->column) /*down*/||
                    ((Array+i)->row -1) == (m_current->row) && ((Array+i)->column) == (m_current->column) /*up*/){
                    Node *New = (Node *) malloc(sizeof(Node));
                    New->column = (Array+i)->column;
                    New->row = (Array+i)->row;
                    New->type = (Array+i)->type;
                    New->strength = (Array+i)->strength;
                    New->next = m_current;
                    //find connections, considering front of list is the result
                    if(i==0){
                        Array = (Array+1);
                    }
                    else {(Array+i-1)->next = (Array+i+1);
                    free(Array+i);}
                    m_current = New;
                }
                i++;
            }
            //Add the root of branches to a head list
            nodeHeadList *newBranch = (nodeHeadList *) malloc(sizeof(nodeHeadList));
            newBranch->oneHead = m_current;
            newBranch->next = branches;
            branches = newBranch;
            i = 0;
            size++;
            *m_current = *rootList+1;
        }
    }
    return branches;
}

void detection(int x, int y, nodeHeadList *currentHead){
    //detect branch that been hit and delete all attached objects
    int i;
    Node *Head;
    while (currentHead != NULL){
        Head = currentHead->oneHead;
        i = 0;
        while ((Head+i)->next != NULL){
            if (((Head+i)->row == y) || ((Head+i)->column == x)){
                int j;
                while ((Head+i+j)->next != NULL){
                    //delete all attachments
                    deleteTile((Head+i+j)->row,(Head+i+j)->column);
                    j++;
                }
                exit;
            }
            else{
                i++;
            }
        }
        currentHead = currentHead->next;
    }
}

Node* spotmokeyroot(nodeHeadList *branches, int *world){
    //find the root of leftmost monkey is sitting on
    int r = 0,i = 0;
    int firsttree;
    while(r == 0){
        if (world[i*4+4]==84){
            r = 1;
            firsttree = world[i*4+3];
        }
        i++;
    }
    //locate array before the first tree
    Node* Array = Brancharray(0,firsttree,world);
    r = 0;
    while (r == 0){
        if((Array+i)->type == 77){
            //find monkey in Array
            while ((r==0) && (branches != NULL)){
                Node *Head = branches->oneHead;
                //find same monkey in branch lists
                int j = 0;
                while ((r==0) && ((Head+j)->next != NULL)){
                    if ((Head+j)->type == 77){
                        return Head;
                        r = 1;
                    }
                    else {
                        j++;
                    }
                }
                branches = branches->next;
            }
            r = 0;
        }
        i++;
    }
}

void run_trajectory(int *world,nodeHeadList *branches){
    int i=0,j=0,k=0,temp = 1;
    //j is looping column(x), k is looping row(y)
    while (j<(sqrt(world[0]))){
        while (i<world[1]){
            //std::cout << "test a i " << i << std::endl;
            //std::cout << k << "and " << j << std::endl;
            //std::cout << world[4*i+2] << std::endl;
            //std::cout << world[4*i+3] << std::endl;
            if (world[4*i+2] == k && world[4*i+3] == j && world[4*i+5] != 0){
                temp = 0;
                if (world[4*i+5]>1){
                world[4*i+5] = world[4*i+5]-1;
                colorTile(k,j,world[4*i+5]);
                }
                else{
                    world[4*i+5] = 0;
                    detection(k,j,branches);
                }
            //if strenth is greater than 1 when it is hit, decrement strenth,
            //or just delete the block and all its attachments
                break;
            //if the ball hit something, stop
            }
            else{
                i++;
            //otherwise, keep going till find the possible point of hit         
            }
        }
        if (temp == 0){
            break;
        }
        else{
        i = 0;
        j++;
        k = floor(sin(angle)/cos(angle)*j - GRAVITY*(j/(power*cos(angle)))*(j/(power*cos(angle)))/2);
        updateShot(j,k,1);
        sleep(1);
        }
    }
    //if no hit-point is found, update trajectory to next move
 //   hint(i+2, i+3,2,1);
    updateShot(0,0,1);
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