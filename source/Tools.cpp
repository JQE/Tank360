#include "Tools.h"

#include <xenon_soc/xenon_power.h>
#include <xenon_smc/xenon_smc.h>


void xTools::ActionShutdown(void * unused) {
    // shutdown
    printf("Shutdown ...\r\n");
    xenon_smc_power_shutdown();
}

void xTools::ActionRestart(void * unused) {
    // restart
    xenon_smc_power_reboot();
}