#ifndef HEADER_H_
	#define HEADER_H_
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/exception.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iphlpapi.h>
#include <tchar.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <functional>
#include <ctime>

using namespace std;

struct ConfigEntry {
    std::string testID;
    std::string ctrlName;
    std::string ctrlDesc;
    std::string alertType;
    std::string params;
};

// Déclaration de la fonction
std::vector<ConfigEntry> readConfigFile(const std::string& filePath);

void read_file(int& warningThreshold, int& alertThreshold);
void disk_free_space(const wchar_t* driveLetter, int warningThreshold, int alertThreshold, const std::string& logFileName);
void find_and_display_all_drives(int warningThreshold, int alertThreshold, const std::string& logFileName);
int network_checker(const std::string& adapterName, const std::string& logFileName);
void list_removable_drives(const std::string& logFileName);
void ListRunningServices(const std::string& logFileName);
void executeTest(const std::string& testID, const std::string& params, int warningThreshold, int alertThreshold, const std::string& logFileName);
void logToFile(const std::string& message, const std::string& fileName);
std::string wcharToString(const wchar_t* wcharStr);
void logMachineInfo(const std::string& logFileName);

#endif
