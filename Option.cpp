#include "Option.h"




OPTIONS::OPTIONS(void)
{
	ResampleSize.width = 0;
	ResampleSize.height = 0;
	ResampleMethod = 0;
	Mode = 0;
	Output = 0;
}

bool
OPTIONS::ChangeResampleMethod(const char *name)
{
	ERROR Error("OPTIONS::ChangeResampleMethod");
	char *lower = nullptr;

	try {
		lower = new char[strlen(name) + 1];
	}
	catch (const std::bad_alloc &bad) {
		std::cerr << bad.what() << std::endl;
		Error.Value("lower");
		Error.Malloc();
		return false;
	}
	for (size_t i = 0; i < strlen(name); i++) {
		lower[i] = char(tolower(name[i]));
	}
	lower[strlen(lower) + 1] = '\0';
	if (strcmp(lower, "z-hold") == 0) {
		ResampleMethod = OPTION_Resample_ZeroOrderHold;
	} else if (strcmp(lower, "bicubic") == 0) {
		ResampleMethod = OPTION_Resample_Bicubic;
	}
	delete[] lower;
	return true;
}

void
OPTIONS::set_default(const char *name)
{
	if (strcmp(name, "ResampleSize") == 0) {
		ResampleSize.width = 0;
		ResampleSize.height = 0;
	} else if (strcmp(name, "Mode") == 0) {
		Mode = 0;
	} else if (strcmp(name, "Output") == 0) {
		Output = 0;
	} else {
		std::cerr << "*** OPTIONS::set_default() error - There are NOT such a parameter '" << name << "' ***" << std::endl;
	}
}

void
OPTIONS::set_value(const char *name, const void *value)
{
	if (strcmp(name, "ResampleSize") == 0) {
		ResampleSize = *static_cast<const SIZE*>(value);
	} else if (strcmp(name, "Mode") == 0) {
		Mode = *static_cast<const int*>(value);
	} else if (strcmp(name, "Output") == 0) {
		Output = *static_cast<const int*>(value);
	} else {
		std::cerr << "*** OPTIONS::set_default() error - There are NOT such a parameter '" << name << "' ***" << std::endl;
	}
}

