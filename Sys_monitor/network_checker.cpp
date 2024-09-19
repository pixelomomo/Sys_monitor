#include "header.h"

#pragma comment(lib, "iphlpapi.lib")
#include <iostream>
#include <Windows.h>
#include <iphlpapi.h>

using namespace std;

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x)) 
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

int network_checker(const std::string& adapterName, const std::string& logFileName) {
    // Structure pour stocker les informations des interfaces réseau
    ULONG outBufLen = 15000;
    IP_ADAPTER_INFO* pAdapterInfo = (IP_ADAPTER_INFO*)malloc(outBufLen);

    // Obtenir les informations des adaptateurs réseau
    if (GetAdaptersInfo(pAdapterInfo, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(outBufLen);
    }

    if (GetAdaptersInfo(pAdapterInfo, &outBufLen) == NO_ERROR) {
        IP_ADAPTER_INFO* pAdapter = pAdapterInfo;
        bool adapterFound = false;
        std::stringstream logMessage;

        while (pAdapter) {
            // Vérification de l'adaptateur par son nom
            if (adapterName.empty() || adapterName == pAdapter->AdapterName) {
                logMessage << "Adaptateur réseau trouvé : " << pAdapter->AdapterName << "\n";
                logMessage << "Description : " << pAdapter->Description << "\n";
                logMessage << "Adresse IP : " << pAdapter->IpAddressList.IpAddress.String << "\n";
                logMessage << "Statut : " << ((pAdapter->DhcpEnabled) ? "DHCP activé" : "DHCP désactivé") << "\n";
                adapterFound = true;
            }
            pAdapter = pAdapter->Next;
        }

        if (!adapterFound && !adapterName.empty()) {
            logMessage << "Adaptateur réseau non trouvé : " << adapterName;
        }

        logToFile(logMessage.str(), logFileName);
    }
    else {
        std::string errorMessage = "Erreur lors de la récupération des informations réseau.";
        logToFile(errorMessage, logFileName);
    }

    free(pAdapterInfo);
    return 0;
}