#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "robots.h"
#include <sys/time.h>
#include <signal.h>

#define PI 3.141
char *robotID;
char *serverIP;
int L, N;
unsigned short port;
int requestID = 0; 

void getArguments(char **argv);
void takeSnapshot();
requestMsg *makeRequest(char *command);

void AlarmHandler(int sig) {
    signal(SIGALRM, SIG_IGN);
    printf("FUCK\n");
    signal(SIGALRM, AlarmHandler);
}

void moveRobot();
void stopRobot();
void turnRobot(double angle);
void sendRequest(requestMsg *request);
char *getGPS();
char *getDGPS();
char *getLasers();
void getImage();
int sock;

struct sockaddr_in middlewareAddr;  // Local Address
struct sockaddr_in fromAddr;       // Client Address
void DieWithError(char *errMsg) {
    fprintf(stderr, "%s\n", errMsg);
    exit(1);
}

int main(int argc, char *argv[]) 
{
    struct hostent *thehost;         // Hostent from gethostbyname()
    serverIP = argv[1];
    port = atoi(argv[2]);
    robotID = argv[3];
    L = atoi(argv[4]);
    N = atoi(argv[5]);

    // If we don't have all of the required command line arguments

    if (argc < 6) {
        printf("Usage: %s IP/Host_name serverPort robotID L N\n", argv[0]);
        exit(1);
    }
    //
    // Construct the server address structure
    // zero out address structure
    memset(&middlewareAddr, 0, sizeof(middlewareAddr));
    // Set the internet address family
    middlewareAddr.sin_family = AF_INET;
    // Server IP Address
    middlewareAddr.sin_addr.s_addr = inet_addr(serverIP);
    // Server port
    middlewareAddr.sin_port = htons(port);
    
    // Get the actual name of the host if given an IP address
    if (middlewareAddr.sin_addr.s_addr == -1){
        thehost = gethostbyname(serverIP);
        middlewareAddr.sin_addr.s_addr = *((unsigned int *) thehost->h_addr_list[0]);
    }

    
    if(sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP) < 0){
     perror("socket() failed");
     return -1;
    }

    double angle = (PI * (N-2))/N; 
    
    int turn = 0;
    takeSnapshot(turn);
    while (true) {
        // Move the robot L meters in the current direction
        moveRobot(L);
        // Turn the robot angle radians
        turnRobot(angle);
        // Get all of the data from the robot
        requestID++;
        takeSnapshot(turn);
    }
    return 0;
}

void takeSnapshot(int turn) {
    char imageFilename[15];
    sprintf(imageFilename, "image-%d.png", turn);
    char textFilename[15];
    sprintf(textFilename, "position-%d.txt", turn);

    FILE *textFile = fopen(textFilename, "w");
    char *GPS = getGPS();
    char *DGPS = getDGPS();
    char *lasers = getLasers();
    
    fprintf(textFile, "%s\n%s\n%s", GPS, DGPS, lasers);
    fclose(textFile);
    getImage();
}
void moveRobot(int meters) {
    // Compute the speed required for moving L meters in 7 seconds
    int moveTime = 7;
    double speed = meters/moveTime;
    char command[15];

    sprintf(command, "MOVE %f", speed);    

    requestMsg *request = makeRequest(command);
    sendRequest(request);
        
    // Wait for 5 seconds to timeout
    sleep(5); //should wait for response instead??? 
    // Wait for 2 more seconds 
    sleep(2);   
    stopRobot(); 
}

void turnRobot(double angle) {
    char command[15];
    sprintf(command, "TURN %f", angle);
    int moveTime = 7;
    double speed = angle/moveTime;
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    stopRobot();
}

void stopRobot() {
    char command[15];
    sprintf(command, "STOP");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
}

char *getGPS() {
    char *gpsData = malloc(30);
    char command[15];
    sprintf(command, "GET GPS");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    return gpsData;
}

char *getDGPS() {
    char *dgpsData = malloc(30);
    char command[15];
    sprintf(command, "GET DGPS");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    return dgpsData;
}

char *getLasers() {
    char *laserData = malloc(30);
    char command[15];
    sprintf(command, "GET LASERS");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
    return laserData;
}

// Gets an image from the robot
void getImage() {
    char command[15];
    sprintf(command, "GET IMAGE");
    requestMsg *request = makeRequest(command);
    sendRequest(request);
}

void sendRequest(requestMsg *request) {
    // Send a UDP message to the middleware
    int sent;
    size_t bufSize = sizeof(unsigned int) + strlen(request->robotID) + 1 + strlen(request->command + 1);
    char *requestBuffer = malloc(bufSize);
    memcpy(requestBuffer, &request->commID, 1);
    memcpy(requestBuffer + 1, request->robotID, strlen(request->robotID) + 1);
    memcpy(requestBuffer + strlen(request->robotID) + 1, request->command, strlen(request->command) + 1);

    int requestLen = bufSize; 
   // Send the guess
   if ((sent = sendto(sock, requestBuffer, requestLen, 0, (struct sockaddr *)
    &middlewareAddr, sizeof(middlewareAddr))) != requestLen)
   {
         DieWithError("sendto() sent a different number of bytes than expected");
   }

}

// Creates a new request.
requestMsg *makeRequest(char *command) {
    requestMsg *newRequest = malloc(sizeof(requestMsg)); 
    newRequest->commID = requestID++;
    strcpy(newRequest->robotID, robotID);  
    strcpy(newRequest->command, command);
    return newRequest;
}

#ifdef MY_CONTROL_MACRO


//return array of response msg instead???
void recvLarge(){
  //--------variables-----------//
  int order; //message number we should be on or count
  int recvSize; //size of received message
  time_t start_t, end_t;
  double diff_t;
  responseMsg messages[100];  //array of response messages from server
  //----------------------------//
  memset(messages,0,sizeof(responseMsg)*100); //zero out array and make space
  //
  //time(&start_t);
  while(1){
    memset(/*buffer*/, 0, /*bufferSize*/);
    if (recvSize = recvfrom(sock, /*buffer*/,....) < 0){
      fprintf(stderr, "recv() less than 0 bytes error or done");
      break;
    }
    else{
      //convert buffer into struct and add to the array
      //time(&end_t);
      //diff_t = difftime(end_t, start_t);
      
    }
  }
}

void recvAckno(int timeout){}

char* recvSmall(){
  int recvSize;
  responseMsg messy;
  char buff[1000];
  time_t start_t, end_t;
  double diff_t; 

  time(&start);
  time(&end);
  while((diff_t = difftime(end_t, start_t)) < timeout){
    if (recvSize = recvfrom(sock, buff, 1000, 0, 
        (struct sockaddr *) &fromAddr) < 0){
      fprintf(stderr, "recv() less than 0 bytes error or done");
      //break;
    }
    else {
      //insert buffer into struct and take data into string and return
      break;
    }
    time(&end);
  }
}

#endif

