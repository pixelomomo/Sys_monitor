#ifndef HEADER_H_
	#define HEADER_H_
#include <iostream>
#include <Windows.h>
#include <fstream>
#include <string>
#include <iphlpapi.h>
using namespace std;
void read_file(int& warningThreshold, int& alertThreshold);
void disk_free_space(const char* driveLetter, int warningThreshold, int alertThreshold);
void find_and_display_all_drives(int warningThreshold, int alertThreshold);
int network_checker();
void list_removable_drives();
void ListRunningServices();
#endif
