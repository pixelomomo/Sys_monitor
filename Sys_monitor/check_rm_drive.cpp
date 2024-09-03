#include "header.h"

using namespace std;

void list_removable_drives()
{
    char driveStrings[256];
    DWORD length = GetLogicalDriveStringsA(sizeof(driveStrings), driveStrings);

    if (length == 0) {
        cerr << "Error getting logical drive strings: " << GetLastError() << endl;
        return;
    }

    cout << "Logical drives found: " << endl;

    for (char* drive = driveStrings; *drive != '\0'; drive += 4) {
        UINT driveType = GetDriveTypeA(drive);
        cout << "Checking drive: " << drive << endl;  // For debugging

        if (driveType == DRIVE_REMOVABLE) {
            cout << "Removable Drive: " << drive << endl;
        }
        else {
            cout << "Not a removable drive: " << drive << endl;  // For debugging
        }
    }
}
