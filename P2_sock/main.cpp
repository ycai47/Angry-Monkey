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
		    
		    run_trajectory(world);
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
        updateShot(row,column,'1');
        
        //have fun... 
        
        /****    END - your code stops here   ****/
        free(world);  
        close(socket_fd);
    }
    //end loop
}

void run_trajectory(int *world){
  //Just for test...
  int i=0,j=0,temp;
  while(temp != '0'){
    j = floor(sin(angle)/cos(angle)*i-GRAVITY*(i/(power*cos(angle)))^2/2);
    i++;
    updateShot(i,j,0);
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
