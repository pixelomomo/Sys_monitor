#include "header.h"

using namespace std;

void read_file(int& warningThreshold, int& alertThreshold)
{
    ifstream configFile("configdisk.txt");
    string line;

    if (configFile.is_open())
    {
        while (getline(configFile, line)) {
            size_t delimiterPos = line.find('=');
            string key = line.substr(0, delimiterPos);
            int value = stoi(line.substr(delimiterPos + 1));

            if (key == "Diskspacewarning") {
                warningThreshold = value;
            }
            else if (key == "Diskspacealert") {
                alertThreshold = value;
            }
        }
        configFile.close();
    }
    else
    {
        cerr << "Unable to open configuration file" << endl;
    }
}

void disk_free_space(const char* driveLetter, int warningThreshold, int alertThreshold)
{
    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;

    if (GetDiskFreeSpaceExA(driveLetter, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes))
    {
        ULONGLONG usedBytes = totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart;

        int usedPercentage = static_cast<int>((usedBytes * 100) / totalNumberOfBytes.QuadPart);

        cout << "Drive " << driveLetter << " is " << usedPercentage << "% full." << endl;
        if (usedPercentage >= alertThreshold) {
            cout << "ALERT: Drive " << driveLetter << " usage has reached " << usedPercentage << "%." << endl;
        }
        else if (usedPercentage >= warningThreshold) {
            cout << "WARNING: Drive " << driveLetter << " usage has reached " << usedPercentage << "%." << endl;
        }
    }
    else {
        cerr << "Error getting disk free space for drive " << driveLetter << ": " << GetLastError() << endl;
    }
}

void find_and_display_all_drives(int warningThreshold, int alertThreshold)
{
    char driveStrings[256];
    DWORD length = GetLogicalDriveStringsA(sizeof(driveStrings), driveStrings);

    if (length == 0) {
        cerr << "Error getting logical drive strings: " << GetLastError() << endl;
        return;
    }

    for (char* drive = driveStrings; *drive != '\0'; drive += 4) {
        disk_free_space(drive, warningThreshold, alertThreshold);
    }
}
