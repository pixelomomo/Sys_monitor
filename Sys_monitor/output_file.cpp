#include "header.h"

std::string getCurrentDateTime()
{
    std::time_t now = std::time(nullptr);
    struct tm localTime;
    // Utilisation de localtime_s pour des raisons de sécurité
    localtime_s(&localTime, &now);

    char buf[80];
    std::strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M:%S", &localTime);
    return std::string(buf);
}
void logToFile(const std::string& message, const std::string& fileName) {
    std::ofstream outFile(fileName, std::ios::app); // Ouvre le fichier en mode ajout
    if (outFile.is_open()) {
        outFile << "[" << getCurrentDateTime() << "] " << message << std::endl;
        outFile.close();
    }
    else {
        std::cerr << "Erreur : Impossible d'ouvrir le fichier " << fileName << std::endl;
    }
}

std::string wcharToString(const wchar_t* wcharStr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wcharStr, -1, nullptr, 0, nullptr, nullptr);
    std::string str(size_needed - 1, 0);  // Crée une chaîne de la taille nécessaire, sans inclure le caractère nul
    WideCharToMultiByte(CP_UTF8, 0, wcharStr, -1, &str[0], size_needed, nullptr, nullptr);
    return str;
}

std::wstring utf8_to_utf16(const std::string& utf8_str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &utf8_str[0], (int)utf8_str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &utf8_str[0], (int)utf8_str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string utf16_to_utf8(const std::wstring& utf16_str) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &utf16_str[0], (int)utf16_str.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &utf16_str[0], (int)utf16_str.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}
