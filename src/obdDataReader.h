/*
 * obdDataReader.h
 *
 *  Created on: Oct 30, 2016
 *      Author: Matt
 */

#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <cstring>
#include <sstream>
#include <vector>

//TCP includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>



#ifndef OBDDATAREADER_H_
#define OBDDATAREADER_H_

using namespace std;

class obdDataReader {
private:

	//Variables
	int obdPortNumber;
	int tcpOBDPortSocket;
	bool successfulPortInit;

	//TCP Port Functions (For WiFi based OBD2)
	int openTCPPort(char hostname[], int port);
	void closeTCPPort();

	//Serial Port Functions
	int openPort(char portName[]);
	void closePort();

	char* readPort(int bufferLength);
	int writePort(unsigned char cmd[]);

public:
	//OBD variables
	double rpm = 0.0;
	double speed = 0.0;
	double engineThrottle = 0.0;
	double airIntakeTemperature = 0.0;
	double engineCoolantTemperature = 0.0;
	double manifoldABSPressure = 0.0;




	void updateAll();
	int readEngineLoad();
	int readRPM();
	int readSpeed();
	int readThrottlePos();
	int readIntakeAirTemp();
	int readManifoldAbsPressure();
	int readEngineCoolantTemp();

	obdDataReader(char portName[]);
	obdDataReader(char hostname[], int port);
	virtual ~obdDataReader();
};

#endif /* OBDDATAREADER_H_ */
