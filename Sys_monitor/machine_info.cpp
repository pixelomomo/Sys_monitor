#include "header.h"

#pragma comment(lib, "Ws2_32.lib")

void logMachineInfo(const std::string& logFileName) {
    // Initialiser Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed with error: " << result << std::endl;
        return;
    }

    // Obtenir le nom d'h�te
    char hostname[NI_MAXHOST]; // Utilisez NI_MAXHOST pour la taille correcte
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        std::cerr << "Erreur lors de la r�cup�ration du nom d'h�te: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    // Obtenir les informations de l'adresse IP
    addrinfo hints = {};
    hints.ai_family = AF_INET; // Pour IPv4
    hints.ai_socktype = SOCK_STREAM; // Type de socket
    hints.ai_protocol = IPPROTO_TCP; // Protocole TCP

    addrinfo* resultInfo = nullptr;
    if (getaddrinfo(hostname, nullptr, &hints, &resultInfo) != 0) {
        std::cerr << "Erreur lors de la r�cup�ration de l'adresse IP: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    // Boucler sur les r�sultats pour r�cup�rer l'adresse IP
    char ipStr[INET_ADDRSTRLEN];
    bool ipFound = false;
    for (addrinfo* ptr = resultInfo; ptr != nullptr; ptr = ptr->ai_next) {
        sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(ptr->ai_addr);
        if (inet_ntop(AF_INET, &(ipv4->sin_addr), ipStr, INET_ADDRSTRLEN) != nullptr) {
            ipFound = true;
            break; // On prend la premi�re adresse IPv4 trouv�e
        }
    }

    // Ouvrir le fichier log et �crire les informations de la machine
    std::ofstream logFile(logFileName, std::ios::app); // Mode append
    if (logFile.is_open()) {
        logFile << "Nom de l'h�te : " << hostname << std::endl;
        if (ipFound) {
            logFile << "Adresse IP : " << ipStr << std::endl;
        }
        else {
            logFile << "Adresse IP non trouv�e." << std::endl;
        }
        logFile << "-------------------------------------" << std::endl;
        logFile.close();
    }
    else {
        std::cerr << "Impossible d'ouvrir le fichier log." << std::endl;
    }

    freeaddrinfo(resultInfo); // Lib�rer la m�moire des r�sultats d'adresse
    WSACleanup(); // Nettoyage de Winsock
}