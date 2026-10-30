#include "../include/imu.h"

#if defined(PICORB_VM_MRUBY)

#include "mruby/imu.c"

#elif defined(PICORB_VM_MRUBYC)

#include "mrubyc/imu.c"

#endif
