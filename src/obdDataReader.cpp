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

	unsigned char requestPID[] = "ATZ\n";
	writePort(requestPID);
	usleep(300000);
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
		usleep(200);
	} while( (buf != '\n' && n > 0) );

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
		usleep(1000);
	} while (cmd[spot-1] != '\n' && n_written > 0);

	char* response;
	/*
	usleep(50000);

	response = readPort(1024);
	cout << "Immediate response: " <<response<<endl;
	usleep(500000);
	response = readPort(1024);
	cout << "Second response: " <<response<<endl;
	char* signedCommand = (char*)cmd;
	if(strcmp(response,signedCommand) == 0){
		cout << "Transmission Successful" << endl;
		usleep(50000);
		return 0;
	}else{
		cout << "Transmission Failed" << endl;
		usleep(50000);
		return -1;
	}
	*/
	return 0;



}

int obdDataReader::readRPM(){

	unsigned char requestPID[] = "010C\n";
	writePort(requestPID);
	usleep(1000);
	char * response = "\0";
	while(response[0]!='4'){
		response = readPort(1024);
		usleep(1000);
	}

	cout << "Engine RPM response: [" << response << "]"<< endl;
	char byteA[3] = {(char)response[6],(char)response[7],'\0'};
	//cout << "Byte A: [" << byteA << "]"<< endl;

	char byteB[3] = {(char)response[9],(char)response[10],'\0'};
	//cout << "Byte B: [" << byteB << "]"<< endl;


	int a = (int)(response[0]);
	std::stringstream ss;
	ss << std::hex << byteA;
	ss >> a;

	int b = (int)(response[1]);
	std::stringstream ss2;
	ss2 << std::hex << byteB;
	ss2 >> b;
	int rpm = (a*256 + b)/4;

	cout << "Engine RPM is " << rpm << endl;


	return 100;

}


void obdDataReader::closePort(){
	close(obdPortNumber);
}

