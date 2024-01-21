#include <stdio.h>

#include "MatterManager.h"

int main(int argc, char * argv[])
{
    matterMgr_init();

    matterMgr_isReady();

    matterMgr_deInit();

    return 0;
}