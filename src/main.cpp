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
#include "gpsReader.h"

#define portNum 4001

#define obdDongleCOMPort "/dev/rfcomm0"
#define obdDongleIP "192.168.1.129"
#define obdDonglePortNumber 35000

#define gpsCOMPort "/dev/ttyACM0"


using namespace std;

bool webThreadCreated = false;
bool gpsThreadCreated = false;
bool obdThreadCreated = false;
pthread_t webThread;
pthread_t gpsThread;
pthread_t obdThread;

void* webAccept(void * webSocketDataPointer);
void* gpsMeasure(void * webSocketDataPointer);
void* obdMeasure(void * webSocketDataPointer);

struct webSocketData
{
	int socketFileDescriptor;
	obdDataReader * obdRef;
	bool currentlyAccepting = false;
	bool gpsMeasuring = false;
	bool obdMeasuring = false;
	gpsReader * gpsRef;
};

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main() {


	cout << "Welcome to the DLCarHUD project...." << endl;

	obdDataReader carOBDReader(obdDongleCOMPort);

    cout << "OBD initialization complete...." << endl;

    gpsReader myGPS(gpsCOMPort);

    cout << "GPS initialization complete...." << endl;

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
    webData.gpsRef = &myGPS;

    while (true) {

    	usleep(1000);


    	if (!obdThreadCreated&&!webData.obdMeasuring) {
			webData.obdMeasuring = true;
    		pthread_create(&obdThread, NULL, obdMeasure, &webData);
    		obdThreadCreated = true;
			printf("OBD thread created\n");
    	}

    	if (obdThreadCreated&&!webData.obdMeasuring)
    	{
    		pthread_join(obdThread, NULL);
    		obdThreadCreated = false;
    	}

    	if (!gpsThreadCreated&&!webData.gpsMeasuring) {
			webData.gpsMeasuring = true;
    		pthread_create(&gpsThread, NULL, gpsMeasure, &webData);
    		gpsThreadCreated = true;
			//printf("gps thread created\n");
    	}

    	if (gpsThreadCreated&&!webData.gpsMeasuring)
    	{
    		pthread_join(gpsThread, NULL);
    		gpsThreadCreated = false;
    	}

    	if (!webThreadCreated&&!webData.currentlyAccepting) {
			webData.currentlyAccepting = true;
    		pthread_create(&webThread, NULL, webAccept, &webData);
    		webThreadCreated = true;
			//printf("thread created\n");
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
	//printf("Here is the message: %s\n", buffer);

	obdDataReader * obdPointer;
	obdPointer = (obdDataReader *)myWebSocket->obdRef;

	gpsReader * gpsPointer;
	gpsPointer = (gpsReader *)myWebSocket->gpsRef;

	double rpm = (obdPointer->rpm)/1000.0;
	double engineThrottle = (obdPointer->engineThrottle);
	double speed = (gpsPointer->speed);
	double airIntakeTemp = (obdPointer->airIntakeTemperature);
	double engineCoolantTemp = (obdPointer->engineCoolantTemperature);
	double manifoldABS = (obdPointer->manifoldABSPressure);
	double latitude = (gpsPointer->latitude);
	double longitude = (gpsPointer->longitude);
	double trackingAngle = (gpsPointer->trackHeading);



	char response[1024];
	bzero(response, sizeof(response));
	snprintf(response, sizeof(response), "RPM: %1.2f, Speed: %.1f, Throttle: %2.1f, IntakeTemp: %.0f, EngineCoolantTemp: %.0f, ManifoldABS: %.0f, Latitude: %lf, Longitude: %lf, TrackingAngle: %lf\n",
			rpm, speed, engineThrottle, airIntakeTemp, engineCoolantTemp, manifoldABS,latitude,longitude,trackingAngle);

	try{
	n = write(connectionFileDescriptor, response, sizeof(response));
	}catch(...){
		myWebSocket->currentlyAccepting = false;
		close(connectionFileDescriptor);
		return 0;
	}

	myWebSocket->currentlyAccepting = false;

	close(connectionFileDescriptor);

	return 0;
}

void* gpsMeasure(void * webSocketDataPointer) {

	struct webSocketData * myData;
	myData = (webSocketData*)webSocketDataPointer;

	gpsReader * gpsPointer;
	gpsPointer = (gpsReader *)myData->gpsRef;
	gpsPointer->readRMC();


	myData->gpsMeasuring = false;

	return 0;
}

void* obdMeasure(void * webSocketDataPointer) {

	struct webSocketData * myData;
	myData = (webSocketData*)webSocketDataPointer;

	obdDataReader * obdPointer;
	obdPointer = (obdDataReader *)myData->obdRef;
	obdPointer->updateAll();


	myData->obdMeasuring = false;

	return 0;
}


