#include "header.h"

using namespace std;

void ListRunningServices()
{
    // Open a handle to the service control manager
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (NULL == hSCManager)
    {
        std::cerr << "OpenSCManager failed with error: " << GetLastError() << std::endl;
        return;
    }

    DWORD dwBytesNeeded = 0;
    DWORD dwServicesCount = 0;
    DWORD dwResumeHandle = 0;
    DWORD dwServiceType = SERVICE_WIN32; // Services of type SERVICE_WIN32

    // First call to EnumServicesStatus to get the buffer size
    EnumServicesStatus(hSCManager, dwServiceType, SERVICE_STATE_ALL, NULL, 0, &dwBytesNeeded, &dwServicesCount, &dwResumeHandle);

    // Allocate buffer to store service information
    LPENUM_SERVICE_STATUS lpServiceStatus = (LPENUM_SERVICE_STATUS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);
    if (NULL == lpServiceStatus)
    {
        std::cerr << "HeapAlloc failed with error: " << GetLastError() << std::endl;
        CloseServiceHandle(hSCManager);
        return;
    }

    // Second call to EnumServicesStatus to get the actual service information
    if (!EnumServicesStatus(hSCManager, dwServiceType, SERVICE_STATE_ALL, lpServiceStatus, dwBytesNeeded, &dwBytesNeeded, &dwServicesCount, &dwResumeHandle))
    {
        std::cerr << "EnumServicesStatus failed with error: " << GetLastError() << std::endl;
        HeapFree(GetProcessHeap(), 0, lpServiceStatus);
        CloseServiceHandle(hSCManager);
        return;
    }

    // Print the list of services
    for (DWORD i = 0; i < dwServicesCount; i++)
    {
        std::wcout << L"Service Name: " << lpServiceStatus[i].lpServiceName << std::endl;
        std::wcout << L"Display Name: " << lpServiceStatus[i].lpDisplayName << std::endl;

        // Determine the current state of the service
        switch (lpServiceStatus[i].ServiceStatus.dwCurrentState)
        {
        case SERVICE_STOPPED:
            std::wcout << L"State: Stopped" << std::endl;
            break;
        case SERVICE_START_PENDING:
            std::wcout << L"State: Start Pending" << std::endl;
            break;
        case SERVICE_STOP_PENDING:
            std::wcout << L"State: Stop Pending" << std::endl;
            break;
        case SERVICE_RUNNING:
            std::wcout << L"State: Running" << std::endl;
            break;
        case SERVICE_CONTINUE_PENDING:
            std::wcout << L"State: Continue Pending" << std::endl;
            break;
        case SERVICE_PAUSE_PENDING:
            std::wcout << L"State: Pause Pending" << std::endl;
            break;
        case SERVICE_PAUSED:
            std::wcout << L"State: Paused" << std::endl;
            break;
        default:
            std::wcout << L"State: Unknown" << std::endl;
            break;
        }

        std::wcout << L"-----------------------------------" << std::endl;
    }

    // Clean up
    HeapFree(GetProcessHeap(), 0, lpServiceStatus);
    CloseServiceHandle(hSCManager);
}
