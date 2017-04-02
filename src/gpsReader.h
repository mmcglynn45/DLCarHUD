/*
 * gpsReader.h
 *
 *  Created on: Mar 30, 2017
 *      Author: matt
 */

#ifndef GPSREADER_H_
#define GPSREADER_H_

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

class gpsReader {

private:
	int gpsPortNumber;

	//Serial Port Functions
	int openPort(char portName[]);
	void closePort();

	char* readPort(int bufferLength);
	int writePort(unsigned char cmd[]);

	char response[1024];

public:

	double latitude;
	double longitude;
	double speed;
	double trackHeading;

	gpsReader(char portName[]);
	gpsReader();
	virtual ~gpsReader();
	void readRMC();
};

#endif /* GPSREADER_H_ */
