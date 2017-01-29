//============================================================================
// Name        : DLCarHUD.cpp
// Author      : Matthew McGlynn
// Version     :
// Copyright   : 
// Description : DLCarHUD - OBD2 Communications
//============================================================================

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include "obdDataReader.h"

#define portNum 4001

#define obdDongleCOMPort "/dev/pts/6"
#define obdDongleIP "192.168.1.129"
#define obdDonglePortNumber 35000

using namespace std;

bool webThreadCreated = false;
pthread_t webThread;

void* webAccept(void * webSocketDataPointer);

struct webSocketData
{
	int socketFileDescriptor;
	obdDataReader * obdRef;
	bool currentlyAccepting = false;
};

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main() {


	cout << "Welcome to the DLCarHUD project...." << endl; // prints !!!Hello World!!!

	obdDataReader carOBDReader(obdDongleCOMPort);

    cout << "OBD initialization complete...." << endl; // prints !!!Hello World!!!


    int sockfd, portno;
    struct sockaddr_in serv_addr;

    printf("Opening TCP/IP socket...\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    	error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = portNum;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
    		sizeof(serv_addr)) < 0)
    	error("ERROR on binding");
    listen(sockfd, 5);
    printf("TCP socket created on port %i",portno);

    struct webSocketData webData;
    webData.socketFileDescriptor = sockfd;
    webData.obdRef = &carOBDReader;

    while (true) {

    	//Update car readings
    	//carOBDReader.updateAll();

    	if (!webThreadCreated&&!webData.currentlyAccepting) {
    		pthread_create(&webThread, NULL, webAccept, &webData);
    		webThreadCreated = true;
    	}

    	if (webThreadCreated&&!webData.currentlyAccepting)
    	{
    		pthread_join(webThread, NULL);
    		webThreadCreated = false;
    	}

    }

    close(webData.socketFileDescriptor);

	return 0;
}

void* webAccept(void * webSocketDataPointer) {

	char buffer[256];
	int connectionFileDescriptor;
	struct webSocketData * myWebSocket;
	myWebSocket = (webSocketData*)webSocketDataPointer;

	myWebSocket->currentlyAccepting = true;

	sockaddr_in cli_addr;
	socklen_t clilen;

	clilen = sizeof(cli_addr);

	connectionFileDescriptor = accept(myWebSocket->socketFileDescriptor,
		(struct sockaddr *) &cli_addr,
		&clilen);

	if (connectionFileDescriptor < 0)
		error("ERROR on accept");

	int n;

	bzero(buffer, 256);
	n = read(connectionFileDescriptor, buffer, 255);

	if (n < 0) error("ERROR reading from socket");
	printf("Here is the message: %s\n", buffer);

	obdDataReader * obdPointer;
	obdPointer = (obdDataReader *)myWebSocket->obdRef;
	double rpm = (obdPointer->rpm)/1000.0;
	double engineThrottle = (obdPointer->engineThrottle);
	double speed = (obdPointer->speed);
	double airIntakeTemp = (obdPointer->airIntakeTemperature);
	double engineCoolantTemp = (obdPointer->engineCoolantTemperature);
	double manifoldABS = (obdPointer->manifoldABSPressure);


	char response[100];
	bzero(response, sizeof(response));
	snprintf(response, sizeof(response), "RPM: %1.2f, Speed: %.1f, Throttle: %2.1f, IntakeTemp: %.0f, EngineCoolantTemp: %.0f, ManifoldABS: %.0f\n", rpm, speed, engineThrottle, airIntakeTemp, engineCoolantTemp, manifoldABS);

	n = write(connectionFileDescriptor, response, sizeof(response));

	myWebSocket->currentlyAccepting = false;

	close(connectionFileDescriptor);

	return 0;
}


