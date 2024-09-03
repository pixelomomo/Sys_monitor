#include "header.h"
using namespace std;

int main()
{
    int warningThreshold;
    int alertThreshold;

    read_file(warningThreshold, alertThreshold);
    find_and_display_all_drives(warningThreshold, alertThreshold);
    network_checker();

    return 0;
}
