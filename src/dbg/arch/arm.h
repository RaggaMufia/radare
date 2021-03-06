#ifndef _INCLUDE_ARM_H_
#define _INCLUDE_ARM_H_

#ifndef _INCLUDE_CPU_H_
#error Do not include arm.h directly!
#endif

#define CPUREG_PC pc
#define WS_PC() ARM_pc

enum {
	ARMBP_LE,
	ARMBP_BE,
	ARMBP_ARM_LE,
	ARMBP_ARM_BE,
	ARMBP_EABI_LE,
	ARMBP_EABI_BE,
	ARMBP_THUMB_LE,
	ARMBP_THUMB_BE,
	ARMBP_ARM_THUMB_LE,
	ARMBP_ARM_THUMB_BE,
	ARMBP_LAST
} ArmBpType;

#if __linux__
#include "arm-linux.h"
#elif __APPLE__
#include "arm-darwin.h"
#endif

#endif
