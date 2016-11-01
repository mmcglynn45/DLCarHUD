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

	//TCP Port Functions (For WiFi based OBD2)
	int openTCPPort(char hostname[], int port);
	void closeTCPPort();

	//Serial Port Functions
	int openPort(char portName[]);
	void closePort();

	char* readPort(int bufferLength);
	int writePort(unsigned char cmd[]);

public:
	int readEngineLoad();
	int readRPM();

	obdDataReader(char portName[]);
	obdDataReader(char hostname[], int port);
	virtual ~obdDataReader();
};

#endif /* OBDDATAREADER_H_ */
