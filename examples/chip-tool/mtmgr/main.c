#include <MatterManager.h>

#include <stdint.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
    uint32_t ret = 0;

    ret = Mtmgr_setUpStack();
    printf("ret: %d\n", ret);

    Mtmgr_tearDownStack();

    return 0;
}