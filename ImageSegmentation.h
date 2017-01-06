#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <list>
#include <string>
#if defined(_OPENMP)
#include <omp.h>
#endif
#include <unistd.h>
#include <vector>


#include "Error.h"
#include "Option.h"

#define OUTPUT_IMG_CLASS
#include "ImgClass/Color.h"
#include "ImgClass/Vector.h"
#include "ImgClass/ImgClass.h"
#include "ImgClass/Segmentation.h"

#include "pnm_lib_cpp/pnm.h"




/* Escape sequence "\x1b[nC" moves cursor right.
 * Its length is 2byte for '\x1b[', 1byte each for n digits and 1byte for 'C'.
 * Hence here the maximum length is 5. */
#define NUM_PROGRESS 64
extern const char Progress[NUM_PROGRESS][6];
extern const char Progress_End[];




// prototype
void SequenceProcessor(const std::string& InputName, const int Start, const int End, const OPTIONS& Options);
size_t count_format_length(const std::string& str);

ImgClass::Segmentation<ImgClass::Lab>* ImageSegmentation(const ImgVector<ImgClass::RGB>& img, const double& MaxInt, const unsigned long Mode, const std::string& newest_filename);

