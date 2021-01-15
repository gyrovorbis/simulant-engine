#include "memory.h"
#include "../logging.h"

#ifdef __DREAMCAST__
#include <malloc.h>
#include <kos.h>

static unsigned long systemRam = 0x00000000;
static unsigned long elfOffset = 0x00000000;
static unsigned long stackSize = 0x00000000;

extern unsigned long end;
extern unsigned long start;

#define _end end
#define _start start

void set_system_ram() {
   systemRam = 0x8d000000 - 0x8c000000;
   elfOffset = 0x8c000000;

   stackSize = (long)&_end - (long)&_start + ((long)&_start - elfOffset);
}

unsigned long get_system_ram() {
   return systemRam;
}

unsigned long get_free_ram() {
    struct mallinfo mi = mallinfo();
    return systemRam - (mi.usmblks + stackSize);
}

#endif

namespace smlt {

void print_available_ram() {       
#ifdef __DREAMCAST__
    if(!systemRam) {
        set_system_ram();
    }

    L_DEBUG(_F("Free RAM: {0}").format(get_free_ram()));
#endif
}


}
