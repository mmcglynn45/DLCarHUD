/*
 * obdDataReader.cpp
 *
 *  Created on: Oct 30, 2016
 *      Author: matt
 */

#include "obdDataReader.h"



obdDataReader::obdDataReader(char portName[]) {
	if (openPort(portName) > -1){
		std::cout << "Received port initialization success..." << std::endl;

	}
}

obdDataReader::~obdDataReader() {
	std::cout << "Now closing port" << std::endl;
	closePort();
	std::cout << "Port closed" << std::endl;
}

int obdDataReader::openPort(char portName[]){
	obdPortNumber = open(portName, O_RDWR);

	/* Error Handling */
	if ( obdPortNumber < 0 )
	{
		cout << "Error " << errno << " opening " << portName << ": " << strerror (errno) << endl;
		return -1;
	}else{
		cout << "Successfully opened " << portName << endl;

		struct termios tty;
		struct termios tty_old;
		memset (&tty, 0, sizeof tty);

		/* Error Handling */
		if ( tcgetattr ( obdPortNumber, &tty ) != 0 ) {
			std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
		}

		/* Save old tty parameters */
		tty_old = tty;

		/* Set Baud Rate */
		cfsetospeed (&tty, (speed_t)B9600);
		cfsetispeed (&tty, (speed_t)B9600);

		/* Setting other Port Stuff */
		tty.c_cflag     &=  ~PARENB;            // Make 8n1
		tty.c_cflag     &=  ~CSTOPB;
		tty.c_cflag     &=  ~CSIZE;
		tty.c_cflag     |=  CS8;

		tty.c_cflag     &=  ~CRTSCTS;           // no flow control
		tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
		tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

		/* Make raw */
		cfmakeraw(&tty);

		/* Flush Port, then applies attributes */
		tcflush( obdPortNumber, TCIFLUSH );
		if ( tcsetattr ( obdPortNumber, TCSANOW, &tty ) != 0) {
			std::cout << "Error " << errno << " from tcsetattr" << std::endl;
		}

		return 0;
	}


}

char* obdDataReader::readPort(int bufferLength){
	int n = 0,
		spot = 0;
	char buf = '\0';

	/* Whole response*/
	char response[bufferLength];
	memset(response, '\0', sizeof response);

	do {
		n = read( obdPortNumber, &buf, 1 );
		sprintf( &response[spot], "%c", buf );
		spot += n;
	} while( buf != '\n' && n > 0);

	if (n < 0) {
		std::cout << "Error reading: " << strerror(errno) << std::endl;
	}
	else if (n == 0) {
		std::cout << "Read nothing!" << std::endl;
	}
	else {
		//std::cout << "Response: " << response << std::endl;
	}
	return response;
}

int obdDataReader::writePort(unsigned char cmd[]){
	int n_written = 0,
			spot = 0;

	do {
		n_written = write(obdPortNumber, &cmd[spot], 1 );
		spot += n_written;
	} while (cmd[spot-1] != '\n' && n_written > 0);

	char* response;
	response = readPort(1024);
    char* signedCommand = (char*)cmd;

	if(strcmp(response,signedCommand) == 0){
		cout << "Transmission Successful" << endl;
		readPort(1024); //Clear out the new line
		return 0;
	}else{
		cout << "Transmission Failed" << endl;
		return -1;
	}


}

int obdDataReader::readRPM(){
	unsigned char requestPID[] = "010C \n";
	writePort(requestPID);
	string response = readPort(1024);
	cout << "Engine RPM response: " << response << endl;
	return 100;

}


void obdDataReader::closePort(){
	close(obdPortNumber);
}

