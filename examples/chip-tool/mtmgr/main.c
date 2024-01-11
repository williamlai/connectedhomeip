#include <MatterManager.h>

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char * argv[])
{
    uint32_t ret = 0;

    ret = Mtmgr_setUpStack();
    printf("Mtmgr_setUpStack() ret: %d\n", ret);

    ret = Mtmgr_discoverCommissionables();
    printf("Mtmgr_discoverCommissionables() ret: %d\n", ret);

    Mtmgr_tearDownStack();

    return 0;
}