#include "../include/touch.h"

#if defined(PICORB_VM_MRUBY)

#include "mruby/touch.c"

#elif defined(PICORB_VM_MRUBYC)

#include "mrubyc/touch.c"

#endif
