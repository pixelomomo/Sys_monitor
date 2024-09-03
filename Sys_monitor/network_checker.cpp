#include "header.h"

#pragma comment(lib, "iphlpapi.lib")
#include <iostream>
#include <Windows.h>
#include <iphlpapi.h>

using namespace std;

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x)) 
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

int network_checker() // Corrected the typo in the function name
{
    PIP_INTERFACE_INFO pInfo = NULL;
    ULONG ulOutBufLen = 0;
    DWORD dwRetVal = 0;

    // First call to GetInterfaceInfo to get the size needed for ulOutBufLen
    dwRetVal = GetInterfaceInfo(NULL, &ulOutBufLen);
    if (dwRetVal == ERROR_INSUFFICIENT_BUFFER) {
        pInfo = (IP_INTERFACE_INFO*)MALLOC(ulOutBufLen);
        if (pInfo == NULL) {
            cerr << "Unable to allocate memory needed to call GetInterfaceInfo" << endl;
            return 1;
        }
    }
    else {
        cerr << "GetInterfaceInfo failed with error: " << dwRetVal << endl;
        return 1;
    }

    // Second call to GetInterfaceInfo to get the actual data
    if ((dwRetVal = GetInterfaceInfo(pInfo, &ulOutBufLen)) == NO_ERROR) {
        cout << "Number of Adapters: " << pInfo->NumAdapters << endl;

        // Loop through the adapters and print their status
        for (int i = 0; i < (int)pInfo->NumAdapters; i++) {
            cout << "Adapter Index: " << pInfo->Adapter[i].Index << endl;

            // Use the GetIfEntry function to get more details about the adapter
            MIB_IFROW ifRow;
            ifRow.dwIndex = pInfo->Adapter[i].Index;

            // Fetch the row corresponding to the adapter index
            if (GetIfEntry(&ifRow) == NO_ERROR) {
                // Check if the adapter is up and running
                if (ifRow.dwOperStatus == IF_OPER_STATUS_OPERATIONAL) {
                    cout << "Adapter Status: Active" << endl;
                }
                else {
                    cout << "Adapter Status: Inactive" << endl;
                }
            }
            else {
                cerr << "GetIfEntry failed for adapter " << pInfo->Adapter[i].Index << endl;
            }

            cout << endl;
        }
    }
    else {
        cerr << "GetInterfaceInfo failed with error: " << dwRetVal << endl;
    }

    // Free the allocated memory
    if (pInfo != NULL) {
        FREE(pInfo);
    }

    return 0;
}