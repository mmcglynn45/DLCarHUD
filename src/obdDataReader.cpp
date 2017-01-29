/*
 * obdDataReader.cpp
 *
 *  Created on: Oct 30, 2016
 *      Author: matt
 */

#include "obdDataReader.h"



obdDataReader::obdDataReader(char portName[]) {
	tcpOBDPortSocket = 0;
	obdPortNumber = 0;


	if (openPort(portName) > -1){
		std::cout << "Received port initialization success..." << std::endl;

	}

	//Send device reset
	unsigned char requestPID[] = "ATSP3\r";
	writePort(requestPID);
	usleep(1000000);


	unsigned char request2PID[] = "ATZ\r";
	writePort(request2PID);
	usleep(1000000);



}

obdDataReader::obdDataReader(char hostname[], int port){
	tcpOBDPortSocket = 0;
	obdPortNumber = 0;

	if (openTCPPort(hostname,port) > -1){
			std::cout << "Received port initialization success..." << std::endl;
	}

	//Send device reset
	unsigned char requestPID[] = "ATZ\r";
	writePort(requestPID);
	usleep(300000);


	unsigned char request2PID[] = "ATD\r";
	writePort(request2PID);
	usleep(300000);

	unsigned char request3PID[] = "ATE1\r";
	writePort(request3PID);
	usleep(300000);

	unsigned char request4PID[] = "ATL1\r";
	writePort(request4PID);
	usleep(300000);

	unsigned char request5PID[] = "0100\r";
	writePort(request5PID);
	usleep(300000);
}

obdDataReader::~obdDataReader() {

	if(obdPortNumber){
		std::cout << "Now closing port" << std::endl;
		closePort();
		std::cout << "Port closed" << std::endl;
	}

	if(tcpOBDPortSocket){
	    std::cout << "Now closing TCP port" << std::endl;
	    closeTCPPort();
	    std::cout << "TCP port closed" << std::endl;
	}

}

int obdDataReader::openTCPPort(char hostname[], int port){
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0){
		cout<<"Unable to open TCP socket" << endl;
		return -1;
	}

	server = gethostbyname(hostname);
	if (server == NULL) {
		cout<<"Unable to retrieve host from hostname " << hostname <<endl;
		return -2;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(port);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		cout<<"Unable to connect to port "<< port <<" of host " << hostname <<endl;
		return -3;
	}

	//Connection successful
	tcpOBDPortSocket = sockfd;

	return 0;
}

void obdDataReader::closeTCPPort(){
	close(tcpOBDPortSocket);
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
	int readPort = 0;
	if(obdPortNumber){
		readPort = obdPortNumber;
	}else{
		readPort = tcpOBDPortSocket;
	}

	int n = 0,
			spot = 0;
	char buf = '\0';

	/* Whole response*/
	char response[bufferLength];
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
		std::cout << "Response: " << response << std::endl;

	}

	if (response[0] == '>'){
		std::cout << "Read a command start!" << std::endl;
		//response++;
	}
	return response;
}

int obdDataReader::writePort(unsigned char cmd[]){
	int writePort = 0;
	if(obdPortNumber){
		writePort = obdPortNumber;
	}else{
		writePort = tcpOBDPortSocket;
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

int obdDataReader::readRPM(){

	unsigned char requestPID[] = "010C\r";
	writePort(requestPID);
	usleep(1000);
	char * response = "\0";
	while(response[0]!='4'&&response[0]!='>'){
		response = readPort(1024);
		if (response[0]=='>'){
			response++;
		}
		usleep(1000);
	}

	cout << "Engine RPM response: [" << response << "]"<< endl;
	char byteA[3] = {(char)response[6],(char)response[7],'\0'};
	cout << "Byte A: [" << byteA << "]"<< endl;

	char byteB[3] = {(char)response[9],(char)response[10],'\0'};
	cout << "Byte B: [" << byteB << "]"<< endl;


	int a = (int)(response[0]);
	std::stringstream ss;
	ss << std::hex << byteA;
	ss >> a;

	int b = (int)(response[1]);
	std::stringstream ss2;
	ss2 << std::hex << byteB;
	ss2 >> b;
	int rpmCalc = (a*256 + b)/4;
	rpm = rpmCalc/1.0;

	cout << "Engine RPM is " << rpm << endl;


	return rpm;

}

int obdDataReader::readSpeed(){

	unsigned char requestPID[] = "010D\r";
	writePort(requestPID);
	usleep(1000);
	char * response = "\0";
	while(response[0]!='4'&&response[0]!='>'){
		response = readPort(1024);
		if (response[0]=='>'){
			response++;
		}
		usleep(1000);
	}

	cout << "Engine Speed response: [" << response << "]"<< endl;
	char byteA[3] = {(char)response[6],(char)response[7],'\0'};
	cout << "Byte A: [" << byteA << "]"<< endl;


	int a = (int)(response[0]);
	std::stringstream ss;
	ss << std::hex << byteA;
	ss >> a;

	int speedCalc = a;
	speed = speedCalc*0.621371192;

	cout << "Engine Speed is " << speed << endl;


	return speed;

}

int obdDataReader::readThrottlePos(){

	unsigned char requestPID[] = "0111\r";
	writePort(requestPID);
	usleep(1000);
	char * response = "\0";
	while(response[0]!='4'&&response[0]!='>'){
		response = readPort(1024);
		if (response[0]=='>'){
			response++;
		}
		usleep(1000);
	}

	cout << "Engine Throttle response: [" << response << "]"<< endl;
	char byteA[3] = {(char)response[6],(char)response[7],'\0'};
	cout << "Byte A: [" << byteA << "]"<< endl;


	int a = (int)(response[0]);
	std::stringstream ss;
	ss << std::hex << byteA;
	ss >> a;

	int throttlePos = a;
	engineThrottle = throttlePos;

	cout << "Engine Throttle Position is " << throttlePos << endl;

	return throttlePos;

}

int obdDataReader::readIntakeAirTemp(){

	unsigned char requestPID[] = "010F\r";
	writePort(requestPID);
	usleep(1000);
	char * response = "\0";
	while(response[0]!='4'&&response[0]!='>'){
		response = readPort(1024);
		if (response[0]=='>'){
			response++;
		}
		usleep(1000);
	}

	cout << "Intake Airtemp Response response: [" << response << "]"<< endl;
	char byteA[3] = {(char)response[6],(char)response[7],'\0'};
	cout << "Byte A: [" << byteA << "]"<< endl;


	int a = (int)(response[0]);
	std::stringstream ss;
	ss << std::hex << byteA;
	ss >> a;

	int intakeTempCalc = a - 40;
	airIntakeTemperature = intakeTempCalc;

	cout << "Intake Air Temperature is " << airIntakeTemperature << endl;

	return airIntakeTemperature;

}

int obdDataReader::readEngineCoolantTemp(){

	unsigned char requestPID[] = "0105\r";
	writePort(requestPID);
	usleep(1000);
	char * response = "\0";
	while(response[0]!='4'&&response[0]!='>'){
		response = readPort(1024);
		if (response[0]=='>'){
			response++;
		}
		usleep(1000);
	}

	cout << "Engine Coolant Temp Response response: [" << response << "]"<< endl;
	char byteA[3] = {(char)response[6],(char)response[7],'\0'};
	cout << "Byte A: [" << byteA << "]"<< endl;


	int a = (int)(response[0]);
	std::stringstream ss;
	ss << std::hex << byteA;
	ss >> a;

	int tempCalc = a - 40;
	engineCoolantTemperature = tempCalc;

	cout << "Engine Coolant Temperature is " << engineCoolantTemperature << endl;

	return engineCoolantTemperature;

}

int obdDataReader::readManifoldAbsPressure(){

	unsigned char requestPID[] = "010B\r";
	writePort(requestPID);
	usleep(1000);
	char * response = "\0";
	while(response[0]!='4'&&response[0]!='>'){
		response = readPort(1024);
		if (response[0]=='>'){
			response++;
		}
		usleep(1000);
	}

	cout << "Manifold ABS Pressure response: [" << response << "]"<< endl;
	char byteA[3] = {(char)response[6],(char)response[7],'\0'};
	cout << "Byte A: [" << byteA << "]"<< endl;


	int a = (int)(response[0]);
	std::stringstream ss;
	ss << std::hex << byteA;
	ss >> a;

	int tempCalc = a;
	manifoldABSPressure = tempCalc;

	cout << "Manifold ABS Pressure is " << manifoldABSPressure << endl;

	return manifoldABSPressure;

}

void obdDataReader::updateAll(){
	readRPM();
	readManifoldAbsPressure();
	readIntakeAirTemp();
	readSpeed();
	readThrottlePos();
	readEngineCoolantTemp();
}


void obdDataReader::closePort(){
	close(obdPortNumber);
}

