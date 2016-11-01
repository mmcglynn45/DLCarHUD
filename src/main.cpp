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

#define obdDonglePort "/dev/pts/12"


using namespace std;

int main() {

	char portName[] = obdDonglePort;

	cout << "Welcome to the DLCarHUD project...." << endl; // prints !!!Hello World!!!

	obdDataReader carOBDReader(portName);

    for(int i = 0; i < 10; i++){
    	carOBDReader.readRPM();
    }






	return 0;
}


