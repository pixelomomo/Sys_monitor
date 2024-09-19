#include "header.h"

using namespace std;

std::string WideCharToString(LPCWSTR wideString) {
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideString, -1, nullptr, 0, nullptr, nullptr);
    std::string result(bufferSize, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideString, -1, &result[0], bufferSize, nullptr, nullptr);
    return result;
}

void ListRunningServices(const std::string& logFileName) {
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
    if (hSCManager == nullptr) {
        std::string errorMessage = "OpenSCManager a échoué avec l'erreur: " + std::to_string(GetLastError());
        logToFile(errorMessage, logFileName);
        return;
    }

    DWORD dwBytesNeeded = 0, dwServicesCount = 0, dwResumeHandle = 0;
    EnumServicesStatus(hSCManager, SERVICE_WIN32, SERVICE_STATE_ALL, nullptr, 0, &dwBytesNeeded, &dwServicesCount, &dwResumeHandle);

    LPENUM_SERVICE_STATUS lpServiceStatus = (LPENUM_SERVICE_STATUS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);
    if (lpServiceStatus == nullptr) {
        std::string errorMessage = "HeapAlloc a échoué avec l'erreur: " + std::to_string(GetLastError());
        logToFile(errorMessage, logFileName);
        CloseServiceHandle(hSCManager);
        return;
    }

    if (!EnumServicesStatus(hSCManager, SERVICE_WIN32, SERVICE_STATE_ALL, lpServiceStatus, dwBytesNeeded, &dwBytesNeeded, &dwServicesCount, &dwResumeHandle)) {
        std::string errorMessage = "EnumServicesStatus a échoué avec l'erreur: " + std::to_string(GetLastError());
        logToFile(errorMessage, logFileName);
        HeapFree(GetProcessHeap(), 0, lpServiceStatus);
        CloseServiceHandle(hSCManager);
        return;
    }

    std::string message = "Services en cours d'exécution :\n";
    for (DWORD i = 0; i < dwServicesCount; i++) {
        // Conversion des LPCWSTR en std::wstring puis en std::string
        std::wstring wsServiceName(lpServiceStatus[i].lpServiceName);
        std::wstring wsDisplayName(lpServiceStatus[i].lpDisplayName);

        std::string serviceName(wsServiceName.begin(), wsServiceName.end());
        std::string displayName(wsDisplayName.begin(), wsDisplayName.end());

        message += "Service Name: " + serviceName + "\n";
        message += "Display Name: " + displayName + "\n";

        switch (lpServiceStatus[i].ServiceStatus.dwCurrentState) {
        case SERVICE_STOPPED:
            message += "State: Stopped\n";
            break;
        case SERVICE_RUNNING:
            message += "State: Running\n";
            break;
        default:
            message += "State: Other\n";
            break;
        }
        message += "-----------------------------------\n";
    }

    logToFile(message, logFileName);

    HeapFree(GetProcessHeap(), 0, lpServiceStatus);
    CloseServiceHandle(hSCManager);
}