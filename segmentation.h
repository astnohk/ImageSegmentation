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

#include "ImgClass/RGB.h"
#include "ImgClass/Lab.h"
#include "ImgClass/Vector.h"
#include "ImgClass/ImgClass.h"

#include "PNM/pnm.h"




/* Escape sequence "\x1b[nC" moves cursor right.
 * Its length is 2byte for '\x1b[', 1byte each for n digits and 1byte for 'C'.
 * Hence here the maximum length is 5. */
extern const char Progress[NUM_PROGRESS][6];
extern const char Progress_End[];




// prototype
void SequenceProcessor(const std::string& OutputName, const std::string& InputName, const int Start, const int End, const OPTIONS& Options);

ImgVector<RGB> segmentation(const ImgVector<RGB>& img, const double MaxInt, const int Mode);

