#include "header.h"

using namespace std;

void read_file(int& warningThreshold)
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
        }
        configFile.close();
    }
    else
    {
        cerr << "Unable to open configuration file" << endl;
    }
}

void disk_free_space(const wchar_t* driveLetter, int warningThreshold, const std::string& logFileName) {
    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;

    // Utilisation de GetDiskFreeSpaceEx pour obtenir les informations d'espace disque
    if (GetDiskFreeSpaceEx(driveLetter, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        // Calcul de l'espace utilisé et du pourcentage utilisé
        ULONGLONG usedBytes = totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart;
        int usedPercentage = static_cast<int>((usedBytes * 100) / totalNumberOfBytes.QuadPart);

        // Conversion du wchar_t* en std::string pour les logs
        std::string driveLetterStr = wcharToString(driveLetter);
        std::stringstream logMessage;
        logMessage << "Le lecteur " << driveLetterStr << " est utilisé à " << usedPercentage << "%.";

        // Vérification du seuil d'avertissement
        if (usedPercentage >= warningThreshold) {
            logMessage << " AVERTISSEMENT: Seuil d'avertissement atteint (" << warningThreshold << "%).";
        }

        // Écriture dans le fichier de log
        logToFile(logMessage.str(), logFileName);
    }
    else {
        // Gestion des erreurs en cas d'échec de GetDiskFreeSpaceEx
        std::string driveLetterStr = wcharToString(driveLetter);
        std::stringstream errorMessage;
        errorMessage << "Erreur lors de l'obtention de l'espace libre du lecteur " << driveLetterStr << ": " << GetLastError();

        // Log de l'erreur
        logToFile(errorMessage.str(), logFileName);
    }
}

void find_and_display_all_drives(int warningThreshold, const std::string& logFileName) {
    char driveStrings[256];
    DWORD length = GetLogicalDriveStringsA(sizeof(driveStrings), driveStrings);

    if (length == 0) {
        std::cerr << "Error getting logical drive strings: " << GetLastError() << std::endl;
        return;
    }

    for (char* drive = driveStrings; *drive != '\0'; drive += 4) {
        // Conversion de char* en wchar_t* avec mbstowcs_s
        wchar_t wDrive[4];
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wDrive, drive, 4);  // Utilisation sécurisée

        // Appel de la fonction avec le lecteur converti
        disk_free_space(wDrive, warningThreshold, logFileName);
    }
}