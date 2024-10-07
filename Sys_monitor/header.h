#ifndef HEADER_H_
	#define HEADER_H_
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <locale>
#include <codecvt>
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
#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WAny.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WComboBox.h>
#include <Wt/WFileUpload.h>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/backend/MySQL.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WSplitButton.h>
#include <Wt/WStringListModel.h>
#include <Wt/WMemoryResource.h>
#include <Wt/WAnchor.h>
#include <Wt/Http/Response.h>
#include <Wt/Http/Request.h>
#include <Wt/WLink.h>
#include <memory_resource>
#include <Wt/WProgressBar.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WFileResource.h>
#include <Wt/WText.h>

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

void read_file(int& warningThreshold);
void disk_free_space(const wchar_t* driveLetter, int warningThreshold, const std::string& logFileName);
void find_and_display_all_drives(int warningThreshold, const std::string& logFileName);
int network_checker(const std::string& adapterName, const std::string& logFileName);
void list_removable_drives(const std::string& logFileName);
void ListRunningServices(const std::string& logFileName);
void executeTest(const std::string& testID, const std::string& params, int warningThreshold, const std::string& logFileName);
void logToFile(const std::string& message, const std::string& fileName);
std::string wcharToString(const wchar_t* wcharStr);
void logMachineInfo(const std::string& logFileName);
std::wstring utf8_to_utf16(const std::string& utf8_str);
std::string utf16_to_utf8(const std::wstring& utf16_str);

#endif
