#ifndef LIB_Error
#define LIB_Error

#include <cstdio>
#include <string>


class ERROR
{
	private:
		std::string FunctionName;
		std::string ErrorFunctionName;
		std::string ValueName;
		std::string FileName;
	public:
		// Error data set
		ERROR(void);
		explicit ERROR(const char *name);
		const char* OutputFunctionName(void);
		void Function(const char *name);
		void Value(const char *name);
		void File(const char *name);
		// Error output
		void Others(const char *error);
		void OthersWarning(const char *error);
		void Malloc(void);
		void FunctionFail(void);
		void PointerNull(void);
		void ValueIncorrect(void);
		void ImageSize(void);
		void FileRead(void);
		void FileWrite(void);
};

#endif

