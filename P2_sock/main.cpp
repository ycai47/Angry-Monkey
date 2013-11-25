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
void run_trajectory(int *world);
int* searchtree(int *world);
int* treeboundaries(int *treearray);
void Brancharray(int columns,int columne,int *world);
void connectnode(int branchArray);

typedef struct Node{
    int column;
    int row;
    int type;
    int strength;
    Node *next;
}Node;

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
		            run_trajectory(world);
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
        int *treearray;
        int *treeBoundaries;
        treearray = searchtree(world);
        treeBoundaries = treeboundaries(treearray);
        int numtree;
        i=0;
        while (i<(sizeof(treeBoundaries)/sizeof(int))){
            if (i%2 ==0){
                numtree++;
            }
            i++;
        }
        int treeLeft[numtree];
        int treeRight[numtree];
        int l = 0;
        //find the column left/right of tree trunk as treeLeft/treeRight
        i=0;
        while (i<(sizeof(treeBoundaries)/sizeof(int))){
            if (i%2 ==0){
                treeLeft[l] = treeBoundaries[i]-1;
                l++;
            }
            else{
                treeRight[l] = treeBoundaries[i]+1;
                l++;
            }
            i++;
        }
        Node **rootList;
        int indexin = 0;
        while (indexin < world[1]){
            int length=sizeof(treeBoundaries)/sizeof(int);
            for (i=0; i<length; i++){
                if (world[indexin*4+3] == treeLeft[i] || world[indexin*4+3] == treeRight[i]){
                    Node *m_Root=(Node *)malloc(sizeof(Node));
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
        free(world);  
        close(socket_fd);
    }
    //end loop
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
    int treearray[length];
    index = 0;
    int l=0;
    while(index<=world[1]){
        //store column numbers of tree squares in an array
        if (world[index*4+4] == 84){
            treearray[l] = world[index*4+3];
            l++;
        }
        index++;
    }
    int c,d,swap;
    for (c=0;c<(length-1);c++){
        //bubble sort the array in ascending order
        for (d=0;d<(length-c-1);d++){
            if (treearray[d]>treearray[d+1]){
                swap = treearray[d];
                treearray[d] = treearray[d+1];
                treearray[d+1] = swap;
            }
        }
    }
    return treearray;
}

int* treeboundaries(int *treearray){
    //find the boudaries(column) of trees
    int i,treeBound = 1;
    int length = sizeof(treearray)/sizeof(int);
    for (i=0;i<(length);i++){
        //find how many sides are there
        if (treearray[i+1] != treearray[i]){
            treeBound++;            
        }
    }
    int treeBoundaries[treeBound];
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

void Brancharray(int columns,int columne, int *world){
    //sort objects between two columns (including those two column) according to ascending column
    int length = 0;
    int i,j;
    for (i=columns; i<=columne; i++){
        //total objects in the range
        while (j<=world[1]){
            if (world[j*4+3]==i){
                length++;
            }
            j++;
        }
    }
    Node *branchArray[length];
    //create an node array
    int k = 0;
    for (i=columns; i<=columne; i++){
        //store and sort all location pairs in array
        int l=k;
        while (j<world[1]){
            //store pairs
            if (world[j*4+3]==i){
                branchArray[k]->row = world[4*j+2];
                branchArray[k]->column = i;
                k++;
            }
            j++;
        }
        int m,n;
        Node *swap;
        //bubble sort row numbers under each desired column
        for (m=l;m<(k-1);m++){
            for (n=l;n<(k-m-1);n++){
                if (branchArray[n]->row > branchArray[n+1]->row){
                    swap->row = branchArray[n]->row;
                    branchArray[n]->row = branchArray[n+1]->row;
                    branchArray[n+1]->row = swap->row;
                }
            }
        }
    }
}

void connectnode(int branchArray){

}

void run_trajectory(int *world){
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
                    deleteTile(k,j);
                }
            //if strenth is greater than 1 when it is hit, decrement strenth,
            //or just delete the block
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
        k = floor(sin(angle)/cos(angle)*j-GRAVITY*(j/(power*cos(angle)))*(j/(power*cos(angle)))/2);
        updateShot(k,j,1);
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