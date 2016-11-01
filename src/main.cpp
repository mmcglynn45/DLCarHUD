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

#define obdDongleCOMPort "/dev/pts/12"
#define obdDongleIP "127.0.0.1"
#define obdDonglePortNumber 9801

using namespace std;

int main() {

	char hostName[] = obdDongleIP;
	int port = obdDonglePortNumber;

	cout << "Welcome to the DLCarHUD project...." << endl; // prints !!!Hello World!!!

	obdDataReader carOBDReader(hostName,port);

    for(int i = 0; i < 10; i++){
    	carOBDReader.readRPM();
    }






	return 0;
}


