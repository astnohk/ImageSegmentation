#ifndef LIB_Option
#define LIB_Option

#include <iostream>
#include <cstring>
#include "Error.h"

typedef struct {
	unsigned int width;
	unsigned int height;
} SIZE;

struct OPTIONS
{
	SIZE ResampleSize;
	unsigned long ResampleMethod;
	unsigned long Mode;
	unsigned long Output;

	OPTIONS(void);
	bool ChangeResampleMethod(const char *name);
	void set_default(const char *name);
	void set_value(const char *name, const void *value);
};

// Mode Options
#define MODE_MEAN_SHIFT 0x0001

// Output Options
#define OUTPUT_NEGATE 0x01
#define OUTPUT_RESAMPLED 0x02

// Errors
#define OPTION_UNKNOWN 0x0001
#define OPTION_INCORRECT 0x0002
#define OPTION_INSUFFICIENT 0x0004

// Resample Methods
#define OPTION_Resample_ZeroOrderHold 0x0001
#define OPTION_Resample_Bicubic 0x0002

#endif

