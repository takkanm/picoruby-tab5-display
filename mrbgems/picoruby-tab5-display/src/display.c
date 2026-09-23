#include "../include/display.h"

#if defined(PICORB_VM_MRUBY)

#include "mruby/display.c"

#elif defined(PICORB_VM_MRUBYC)

#include "mrubyc/display.c"

#endif
