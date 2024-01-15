#include <MatterManager.h>

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char * argv[])
{
    uint32_t ret = 0;

    Mtmgr_init();

    ret = Mtmgr_DiscoverCommissionables();
    printf("Mt_DiscoverCommissionables ret:%d\n", ret);

    ret = Mtmgr_DiscoverCommissionables();
    printf("Mt_DiscoverCommissionables ret:%d\n", ret);

    // ret = Mtmgr_PairOnNetworkLong();
    // printf("Mtmgr_PairOnNetworkLong ret:%d\n", ret);

    return 0;
}