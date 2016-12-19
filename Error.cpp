#include "Error.h"

ERROR::ERROR(void)
    : FunctionName("[undefined]")
{
}

ERROR::ERROR(const char *name)
    : FunctionName(name)
{
}

const char *
ERROR::OutputFunctionName(void)
{
	return FunctionName.c_str();
}

void
ERROR::Function(const char *name)
{
	ErrorFunctionName = name;
}

void
ERROR::Value(const char *name)
{
	ValueName = name;
}

void
ERROR::File(const char *name)
{
	FileName = name;
}

void
ERROR::Others(const char *error)
{
	fprintf(stderr, "*** %s() error - %s ***\n", FunctionName.c_str(), error);
}

void
ERROR::OthersWarning(const char *error)
{
	fprintf(stderr, "*** %s() warning - %s ***\n", FunctionName.c_str(), error);
}

void
ERROR::Malloc(void)
{
	fprintf(stderr, "*** %s() error - Cannot allocate memory for (*%s) ***\n", FunctionName.c_str(), ValueName.c_str());
}

void
ERROR::FunctionFail(void)
{
	fprintf(stderr, "*** %s() error - %s() failed to compute (%s) ***\n", FunctionName.c_str(), ErrorFunctionName.c_str(), ValueName.c_str());
}

void
ERROR::PointerNull(void)
{
	fprintf(stderr, "*** %s() error - The pointer (*%s) is NULL ***\n", FunctionName.c_str(), ValueName.c_str());
}

void
ERROR::ValueIncorrect(void)
{
	fprintf(stderr, "*** %s() error - The value (%s) is invalid value ***\n", FunctionName.c_str(), ValueName.c_str());
}

void
ERROR::ImageSize(void)
{
	fprintf(stderr, "*** %s() error - The size of image is varied from First Frame ***\n", FunctionName.c_str());
}

void
ERROR::FileRead(void)
{
	fprintf(stderr, "*** %s() error - Failed to read the file \"%s\" by %s() ***\n", FunctionName.c_str(), FileName.c_str(), ErrorFunctionName.c_str());
}

void
ERROR::FileWrite(void)
{
	fprintf(stderr, "*** %s error - Failed to write the file \"%s\" by %s() ***\n", FunctionName.c_str(), FileName.c_str(), ErrorFunctionName.c_str());
}

