#include "header.h"

using namespace std;

void list_removable_drives(const std::string& logFileName) {
    DWORD driveMask = GetLogicalDrives();
    if (driveMask == 0) {
        std::string errorMessage = "Erreur lors de l'obtention des lecteurs logiques : " + std::to_string(GetLastError());
        logToFile(errorMessage, logFileName);
        return;
    }

    std::stringstream logMessage;
    logMessage << "Lecteurs amovibles d�tect�s :\n";

    for (char driveLetter = 'A'; driveLetter <= 'Z'; ++driveLetter) {
        if (driveMask & (1UL << (driveLetter - 'A'))) {  // Utiliser 1UL pour correspondre � un DWORD
            std::string driveName = std::string(1, driveLetter) + ":\\";

            UINT driveType = GetDriveTypeA(driveName.c_str());  // Utiliser GetDriveTypeA pour des cha�nes ANSI (char*)
            if (driveType == DRIVE_REMOVABLE) {
                logMessage << "P�riph�rique amovible d�tect� : " << driveName << "\n";
            }
        }
    }

    logToFile(logMessage.str(), logFileName);
}