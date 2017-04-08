/*
 * gpsReader.cpp

 *
 *  Created on: Mar 30, 2017
 *      Author: matt
 */

using namespace std;

#include "gpsReader.h"

gpsReader::gpsReader() {
	// TODO Auto-generated constructor stub

}

gpsReader::~gpsReader() {
	// TODO Auto-generated destructor stub
}



gpsReader::gpsReader(char portName[]) {
	speed = 0;
	trackHeading = 0;
	latitude = 0;
	longitude = 0;
	if (openPort(portName) > -1){
		std::cout << "Received port initialization success..." << std::endl;

	}
}


int gpsReader::openPort(char portName[]){
	gpsPortNumber = open(portName, O_RDWR);

	/* Error Handling */
	if ( gpsPortNumber < 0 )
	{
		cout << "Error " << errno << " opening " << portName << ": " << strerror (errno) << endl;
		return -1;
	}else{
		cout << "Successfully opened " << portName << endl;

		struct termios tty;
		struct termios tty_old;
		memset (&tty, 0, sizeof tty);

		/* Error Handling */
		if ( tcgetattr ( gpsPortNumber, &tty ) != 0 ) {
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
		tcflush( gpsPortNumber, TCIFLUSH );
		if ( tcsetattr ( gpsPortNumber, TCSANOW, &tty ) != 0) {
			std::cout << "Error " << errno << " from tcsetattr" << std::endl;
		}

		return 0;
	}


}

char * gpsReader::readPort(int bufferLength){
	int readPort = 0;
	if(gpsPortNumber){
		readPort = gpsPortNumber;
	}

	int n = 0,
	spot = 0;
	char buf = '\0';

	/* Whole response*/
	memset(response, '\0', sizeof(response));

	do {
		n = read(readPort, &buf, 1 );
		sprintf( &response[spot], "%c", buf );
		spot += n;
		usleep(200);
	} while( (buf != '\r' && n > 0) );

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

int gpsReader::writePort(unsigned char cmd[]){
	int writePort = 0;
	if(gpsPortNumber){
		writePort = gpsPortNumber;
	}


	int n_written = 0,
			spot = 0;

	do {
		n_written = write(writePort, &cmd[spot], 1 );
		spot += n_written;
		usleep(1000);
	} while (cmd[spot-1] != '\r' && n_written > 0);

	return 0;

}

void gpsReader::readRMC(){

	int bufferLength = 1024;
	usleep(1000);
	readPort(bufferLength);

	string searchString(response);
	std::size_t found = searchString.find("$GPRMC");

	while(found == string::npos){
		readPort(bufferLength);
		string newSearchString(response);
		found = newSearchString.find("$GPRMC");
		usleep(1000);
	}


    std::string input = response;
    std::istringstream ss(input);
    std::string token;

    std::getline(ss, token, ',');
    std::getline(ss, token, ',');
    std::getline(ss, token, ',');
    std::getline(ss, token, ',');

    double latitudeMinutes;
    std::string latitudeMinutesString(token.c_str()+2);
    sscanf(latitudeMinutesString.c_str(), "%lf", &latitudeMinutes);
    token = token.substr(0,2);
    double latitudeDegrees;
    sscanf(token.c_str(), "%lf", &latitudeDegrees);

    latitude = latitudeDegrees + latitudeMinutes/60;

    std::getline(ss, token, ',');

    found = token.find("S");
    if(found!=string::npos){
    	longitude = -longitude;
    }
    std::getline(ss, token, ',');

    double longitudeMinutes;
    std::string longitudeMinutesString(token.c_str()+3);
    sscanf(longitudeMinutesString.c_str(), "%lf", &longitudeMinutes);
    token = token.substr(0,3);
    double longitudeDegrees;
    sscanf(token.c_str(), "%lf", &longitudeDegrees);

    longitude = longitudeDegrees + longitudeMinutes/60;

    std::getline(ss, token, ',');

    found = token.find("W");
    if(found!=string::npos){
    	longitude = -longitude;
    }

    std::getline(ss, token, ',');
    sscanf(token.c_str(), "%lf", &speed);

    // knots to mph conversion
    speed = speed*1.152;

    std::getline(ss, token, ',');
    sscanf(token.c_str(), "%lf", &trackHeading );

    std::getline(ss, token, ',');
    std::getline(ss, token, ',');
    std::getline(ss, token, ',');
    std::getline(ss, token, ',');
    std::getline(ss, token, ',');
    std::getline(ss, token, ',');

}



void gpsReader::closePort(){
	close(gpsPortNumber);
}

