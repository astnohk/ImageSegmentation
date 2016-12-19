#include "ImgClass/RGB.h"
#include "Segmentation.h"




void
SequenceProcessor(const std::string& OutputName, const std::string& InputName, const int Start, const int End, const OPTIONS& Options)
{
	ERROR Error("SequenceProcessor");
	const char *Bars = "------------------------------------------------";

	int CurrentFileNum;

	const int History_Max = 4;
	std::deque<ImgVector<ImgClass::RGB> > sequence_RGB;

	PNM pnm_in;
	PNM pnm_out;
	PNM pnm_orig;
	PNM pnm_res;
	PNM_DOUBLE pnmd_in;
	PNM_DOUBLE pnmd_out;
	ImgVector<ImgClass::RGB> imgd_prev;
	ImgVector<ImgClass::RGB> imgd_in;
	ImgVector<int> img_orig; // For X11 plotting

	int Initialize = 0;

	SIZE size_orig;
	SIZE size_res;

	double progress;
	int count;

	// Calculate least filename length
	size_t InputNameLength =
	    InputName.length() + 1
	    + std::max(count_format_length(InputName), size_t(ceil(log(End) / log(10.0))));
	size_t OutputNameLength =
	    OutputName.length() + 1
	    + std::max(count_format_length(OutputName), size_t(ceil(log(End) / log(10.0))));
	for (CurrentFileNum = Start; CurrentFileNum <= End; CurrentFileNum++) {
		std::string InputNameNums;
		std::string OutputNameNums;
		std::string OutputNameNums_prev;
		// Substitute number to filename
		if (InputName.find_first_of("%") == std::string::npos) {
			InputNameNums = InputName;
		} else {
			char *char_tmp = nullptr;
			try {
				char_tmp = new char[InputNameLength];
			}
			catch (const std::bad_alloc& bad) {
				std::cerr << bad.what() << std::endl;
				Error.Value("char_tmp");
				Error.Malloc();
				goto ExitError;
			}
			sprintf(char_tmp, InputName.c_str(), CurrentFileNum);
			InputNameNums = char_tmp;
			delete[] char_tmp;
			char_tmp = nullptr;
		}
		if (OutputName.find_first_of("%") == std::string::npos) {
			OutputNameNums = OutputName;
			OutputNameNums_prev = OutputName;
		} else {
			char *char_tmp = nullptr;
			try {
				char_tmp = new char[OutputNameLength];
			}
			catch (const std::bad_alloc& bad) {
				std::cerr << bad.what() << std::endl;
				Error.Value("char_tmp");
				Error.Malloc();
				goto ExitError;
			}
			sprintf(char_tmp, OutputName.c_str(), CurrentFileNum);
			OutputNameNums = char_tmp;
			sprintf(char_tmp, OutputName.c_str(), CurrentFileNum - 1);
			OutputNameNums_prev = char_tmp;
			delete[] char_tmp;
			char_tmp = nullptr;
		}
		// Read PNM Files
		if (pnm_orig.read(InputNameNums.c_str()) == PNM_FUNCTION_ERROR) {
			Error.Function("pnm_orig.read");
			Error.File(InputNameNums.c_str());
			Error.FileRead();
			goto ExitError;
		}
		// END Read
		if (size.height == 0 || size.width == 0) { // Initialize size
			size.height = pnm_orig.Height();
			size.width = pnm_orig.Width();
			size_orig.height = pnm_orig.Height();
			size_orig.width = pnm_orig.Width();
			size_prev = size_orig;
			size_res = Options.ResampleSize;
			if (size_res.width == 0) {
				size_res.width = pnm_orig.Width();
			}
			if (size_res.height == 0) {
				size_res.height = pnm_orig.Height();
			}
			maxMN_res = std::max(size_res.width, size_res.height);
			if ((Options.PlotOptions & PLOT_AS_RESAMPLED) != 0) {
				size_out = size_res;
			} else {
				size_out = size;
			}
		}
		if (size_prev.height != int(pnm_orig.Height()) || size_prev.width != int(pnm_orig.Width())) {
			Error.Others("Image size are not match with previous one");
			goto ExitError;
		}
		printf("- The input image size is %dx%d\n- and bit depth is %d\n", size.width, size.height, int(round(log2(double(pnm_orig.MaxInt())))));
		if (size_res.width > 0 || size_res.height > 0) { // Resample
			size = size_res;
			if (pnmd_in.copy(pnm_orig, 1.0) != PNM_FUNCTION_SUCCESS) {
				Error.Function("pnmd_in.copy");
				Error.Value("pnmd_in <- pnmd_orig");
				Error.FunctionFail();
				goto ExitError;
			}
			try {
				pnm_resize(&pnmd_out, pnmd_in, size_res.width, size_res.height, Options.ResampleMethod);
			}
			catch (const std::bad_alloc& bad) {
				std::cerr << bad.what() << std::endl;
				Error.Function("pnm_resize");
				Error.Value("(pnmd_in -> pnmd_out)");
				Error.FunctionFail();
				goto ExitError;
			}
			catch (const std::out_of_range& range) {
				std::cerr << range.what() << std::endl;
				Error.Function("pnm_resize");
				Error.Value("(pnmd_in -> pnmd_out)");
				Error.FunctionFail();
				goto ExitError;
			}
			catch (const std::invalid_argument& arg) {
				std::cerr << arg.what() << std::endl;
				Error.Function("pnm_resize");
				Error.Value("(pnmd_in -> pnmd_out)");
				Error.FunctionFail();
				goto ExitError;
			}
			if (pnm_res.copy(pnmd_out, 1.0, "round") != PNM_FUNCTION_SUCCESS) {
				Error.Function("pnm_double2int");
				Error.Value("(pnmd_out -> pnm_res)");
				Error.FunctionFail();
				goto ExitError;
			}
			if (pnm_in.copy(pnmd_out, 1.0, "round") != PNM_FUNCTION_SUCCESS) {
				Error.Function("pnm_double2int");
				Error.Value("(pnmd_out -> pnm_in)");
				Error.FunctionFail();
				goto ExitError;
			}
			pnmd_in.free();
			pnmd_out.free();
		} else {
			if (pnm_in.copy(pnm_orig) != PNM_FUNCTION_SUCCESS) {
				Error.Function("pnm_in.copy");
				Error.Value("pnm_in <- pnm_orig");
				Error.FunctionFail();
				goto ExitError;
			}
		}
		if ((Options.PlotOptions & PLOT_RESAMPLED_IMG_ONLY) != 0) {
			// Just output only the resampled image
			if (pnm_out.copy(pnm_in) != PNM_FUNCTION_SUCCESS) {
				Error.Function("pnm_out.copy");
				Error.Value("pnm_out <- pnm_in");
				Error.FunctionFail();
				goto ExitError;
			}
			goto Write;
		}
		// Copy color image to ImgVector<ImgClass::RGB> imgd_in
		sequence_RGB.push_front(ImgVector<ImgClass::RGB>(pnm_in.Width(), pnm_in.Height()));
		if (sequence_RGB.size() >= History_Max) {
			sequence_RGB.pop_back();
		}
		imgd_in.reset(pnm_in.Width(), pnm_in.Height());
		for (size_t i = 0; i < imgd_in.size(); i++) {
			if (pnm_in.isRGB()) {
				sequence_RGB[0][i] = ImgClass::RGB(pnm_in[i], pnm_in[i + pnm_in.Size()], pnm_in[i + 2 * pnm_in.Size()]);
				imgd_in[i] = ImgClass::RGB(pnm_in[i], pnm_in[i + pnm_in.Size()], pnm_in[i + 2 * pnm_in.Size()]);
			} else {
				sequence_RGB[0][i] = ImgClass::RGB(pnm_in[i], pnm_in[i], pnm_in[i]);
				imgd_in[i] = ImgClass::RGB(pnm_in[i], pnm_in[i], pnm_in[i]);
			}
		}
		if (pnm_in.isRGB()) { // Convert to Grayscale
			printf("- The input image is color data\n");
			printf("* Convert the image to grayscale before applying Meaningful Alignments.\n");
			printf("Convert...   ");
			// Convert pnm to pnm_double
			if (pnmd_in.copy(pnm_in, 1.0) != PNM_FUNCTION_SUCCESS) {
				Error.Function("pnmd_in.copy");
				Error.Value("pnmd_in <- pnm_in");
				Error.FunctionFail();
				goto ExitError;
			}
			// Convert Color image to Grayscale
			if (pnmd_out.RGB2Gray(pnmd_in) != PNM_FUNCTION_SUCCESS) {
				Error.Function("pnmd_out.RGB2Gray");
				Error.Value("pnmd_out <- pnmd_in");
				Error.FunctionFail();
				goto ExitError;
			}
			pnmd_in.free();
			pnm_in.free();
			// Convert pnm_double to pnm
			if (pnm_in.copy(pnmd_out, 1.0, "round") != PNM_FUNCTION_SUCCESS) {
				Error.Function("pnm_in.copy");
				Error.Value("pnm_in <- pnmd_out");
				Error.FunctionFail();
				goto ExitError;
			}
			pnmd_out.free();
			printf("Finished\n\n");
		}
		sequence_Grayscale.push_front(ImgVector<double>(pnm_in.Width(), pnm_in.Height()));
		if (sequence_Grayscale.size() >= History_Max) {
			sequence_Grayscale.pop_back();
		}
		imgd_in_gray.reset(pnm_in.Width(), pnm_in.Height());
		for (size_t i = 0; i < imgd_in_gray.size(); i++) {
			sequence_Grayscale[0][i] = double(pnm_in[i]);
			imgd_in_gray[i] = double(pnm_in[i]);
		}

		// Show Parameters
		printf("\n      --- Parameters ---\n  %s\n", Bars);
		if (size_res.width > 0 || size_res.height > 0) {
			printf("  | Resample (%d), 0:z-hold, 1:bicubic\n", Options.ResampleMethod);
			printf("  |   %dx%d -> %dx%d\n", int(pnm_orig.Width()), int(pnm_orig.Height()), size_res.width, size_res.height);
		}
		printf("  | filter type = %s\n", FilterNames[FilterParam.type < NUM_FILTER_TYPE ? FilterParam.type : 0].c_str());
		printf("  | filter size = %dx%d\n", FilterParam.size.width, FilterParam.size.height);
		switch (FilterParam.type) {
			case FILTER_ID_EPSILON:
				printf("  | filter epsilon = %.0f\n", FilterParam.epsilon);
				break;
			case FILTER_ID_GAUSSIAN:
				printf("  | Gaussian filter's standard deviation = %f\n", FilterParam.std_deviation);
		}
		printf("  | Scratch Detection :\n");
		printf("  |   s_med = %d\n", Options.s_med);
		printf("  |   s_avg = %d\n", Options.s_avg);
		printf("  |   p = %f\n", Options.p);
		printf("  | Meaningful Alignments :\n");
		printf("  |   precision = %f [deg]\n", 360.0 * Options.p);
		printf("  |   epsilon = %e\n", Options.ep);
		printf("  |   searching angle = [%f, %f] [deg] (from Vertical line)\n", -0.5 * 180.0 / DIV_ANGLE_VERTICAL, 0.5 * 180.0 / DIV_ANGLE_VERTICAL);
		printf("  |   exclusive radius = %f [px]\n", Options.Exclusive_Max_Radius);
		if (Options.Max_Length > 0) {
			printf("  |   Max length = %d [px]\n", Options.Max_Length);
		} else {
			printf("  |   NO search length limit\n");
		}
		if (Options.Max_Output_Length > 0) {
			printf("  |   Max output length = %d [px]\n", Options.Max_Output_Length);
		} else {
			printf("  |   NO output length limit\n");
		}
		if (Options.ExclusivePrinciple != 0) {
			printf("  |   Turn ON Exclusive Principle\n");
		}
		printf("  %s\n\n", Bars);


		if ((Options.mode & MODE_OUTPUT_FILTERED_IMAGE) != 0) {
			// Output filtered image
			printf("* Filtering\n");
			filtered = DetectScratch(pnm_in, Options.s_med, Options.s_avg, FilterParam, DO_NOT_DETECTION);
			if (filtered == nullptr) {
				Error.Function("DetectScratch");
				Error.Value("filtered");
				Error.FunctionFail();
				goto ExitError;
			}
			printf("* Output Filtered Image\n");
			if (pnm_out.copy(PORTABLE_GRAYMAP_BINARY, pnm_in.Width(), pnm_in.Height(), pnm_in.MaxInt(), filtered->data(), 1.0) != PNM_FUNCTION_SUCCESS) {
				Error.Function("pnm_out.copy");
				Error.Value("pnm_out");
				Error.FunctionFail();
				goto ExitError;
			}
			delete filtered;
			filtered = nullptr;
		} else if ((Options.mode & MODE_OUTPUT_MULTIPLE_MOTIONS_AFFINE) != 0) {
			// Computte and output Multiple Motion Affine Parameters by method of M.J.Black
			if (imgd_prev_gray.isNULL() != false) {
				printf("* Skip Calculate Multiple Motions by Affine while there is NOT any previous frame\n");
			} else {
				printf("* Compute Multiple Motions Affine Parameters by method of Michael J. Black\n");
				MultipleMotion_AffineCoeff = MultipleMotion_Affine(&imgd_prev_gray, &imgd_in_gray, pnm_in.MaxInt(), Options.MultipleMotion_Param);
			}
		} else if ((Options.mode & MODE_OUTPUT_AFFINE_BLOCKMATCHING) != 0) {
			// Computte Block Matching with Multiple Motion Affine Parameters by method of M.J.Black
			if (imgd_prev_gray.isNULL() != false) {
				printf("* Skip Calculate Multiple Motions by Affine while there is NOT any previous frame\n");
			} else {
				printf("* Compute Block Matching with Affine by method of Michael J. Black\n");
				MotionVectors = OpticalFlow_BlockMatching(imgd_prev, imgd_in, pnm_in.MaxInt(), Options.MultipleMotion_Param, OutputNameNums, MODE_OUTPUT_AFFINE_BLOCKMATCHING);
			}
		} else if ((Options.mode & MODE_OUTPUT_OPTICALFLOW) != 0) {
			// Computte Block Matching with Multiple Motion Optical Flow by method of M.J.Black
			if (imgd_prev_gray.isNULL() != false) {
				printf("* Skip Calculate Optical Flow while there is NOT any previous frame\n");
			} else {
				printf("* Compute Optical Flow by method of Michael J. Black\n");
				MotionVectors = OpticalFlow_BlockMatching(imgd_prev, imgd_in, pnm_in.MaxInt(), Options.MultipleMotion_Param, OutputNameNums, MODE_OUTPUT_OPTICALFLOW);
			}
		} else if ((Options.mode & MODE_OUTPUT_HISTOGRAMS_OF_ORIENTED_GRADIENTS) != 0
		    || (Options.mode & MODE_OUTPUT_HISTOGRAMS_OF_ORIENTED_GRADIENTS_RAW_HOG) != 0
		    || (Options.mode & MODE_OUTPUT_HISTOGRAMS_OF_ORIENTED_GRADIENTS_MATCHING_VECTOR) != 0) {
			printf("* Compute HOG\n");
			pnmd_in.copy(pnm_in, 1.0 / pnm_in.MaxInt());
			hog_raw_prv.copy(hog_raw);
			hog_prv.copy(hog);
			hog_raw.free();
			hog.free();
			HistogramsOfOrientedGradients(&hog_raw, &hog, pnmd_in, Options.HOG_Param);
		} else {
			// Scratch Detection
			printf("* Detect Scratch like vertical lines\n");
			scratches = DetectScratch(pnm_in, Options.s_med, Options.s_avg, FilterParam, DO_DETECTION);
			if (scratches == nullptr) {
				Error.Function("DetectScratch");
				Error.Value("scratches");
				Error.FunctionFail();
				goto ExitError;
			}
			pnm_in.copy(pnm_in.Desc(), pnm_in.Width(), pnm_in.Height(), pnm_in.MaxInt(), scratches->data(), 1.0);
			if ((Options.mode & MODE_OUTPUT_BINARY_IMAGE) != 0) {
				// Output Scratch Map without Meaningful Alignments
				printf("* Output Scratch binary image\n");
				if (pnm_out.copy(pnm_in) != PNM_FUNCTION_SUCCESS) {
					Error.Function("pnm_out.copy");
					Error.Value("(pnm_out <- pnm_in)");
					Error.FunctionFail();
					goto ExitError;
				}
				for (size_t i = 0; i < pnm_out.Size(); i++) {
					pnm_out[i] = int(round(scratches->get(i)));
				}
			} else {
				// A Contrario Method : Meaningful Alignments
				if (Initialize == 0) {
					Initialize = 1;
					try {
						Pr_table = new ImgVector<double>(maxMN_res + 1, maxMN_res + 1);
					}
					catch (const std::bad_alloc& bad) {
						std::cerr << bad.what() << std::endl;
						Error.Function("new");
						Error.Value("Pr_table");
						Error.Malloc();
						goto ExitError;
					}
					printf("* Calculate Pr(k, L) table :\n[L =     0]   0.0%% |%s\x1b[1A\n", Progress_End);
					progress = 0;
					count = 0;
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
					for (int L = 1; L <= maxMN_res; L++) {
						for (int k = 0; k <= L; k++) {
							Pr_table->at(k, L) = Pr(k, L, Options.p);
						}
#ifdef _OPENMP
#pragma omp critical
#endif
						{
							count++;
							if (round(double(count) / double(maxMN_res) * 1000.0) > progress) {
								progress = round(double(count) / double(maxMN_res) * 1000.0);
								printf("\r[L = %5d] %5.1f%% |%s#\x1b[1A\n", count, progress * 0.1, Progress[NUM_PROGRESS * count / (1 + maxMN_res)]);
							}
						}
					}
					printf("\nComplete!\n");
					l_min = int(ceil((log(Options.ep) - (log(DIV_ANGLE) + log(double(size_res.height)) + 2.0 * log(double(size_res.width)))) / log(Options.p)));
					if (l_min < 1) {
						l_min = 1;
					}
					printf("* Compute k_list\n");
					k_list = Calc_k_l(size_res, Options.p, Options.ep);
					if (k_list == nullptr) {
						Error.Function("Calc_k_l");
						Error.Value("k_list");
						Error.FunctionFail();
						goto ExitError;
					}
				}

				printf("* Compute Direction Field\n");
				angles = DerivativeAngler(scratches);
				if (angles == nullptr) {
					Error.Function("Derivation");
					Error.Value("angles");
					Error.FunctionFail();
					goto ExitError;
				}
				printf("* Compute Segments and Maximal Meaningfulness\n");
				if (MaximalSegments == nullptr) {
					delete[] MaximalSegments;
				}
				MaximalSegments = AlignedSegment_vertical(angles, k_list, l_min, Pr_table, &Num_Segments, Options.Max_Length, Options.Max_Output_Length);
				if (MaximalSegments == nullptr) {
					Error.Function("AlignedSegment_vertical");
					Error.Value("MaximalSegments");
					Error.FunctionFail();
					goto ExitError;
				}
				printf("- Found (%d) Maximal Meaningful Segments\n", Num_Segments);
				if (Options.ExclusivePrinciple != 0) {
					printf("* Delete Redundant Segments by Exclusive Principle\n");
					EPSegments = ExclusivePrinciple(angles, k_list, Pr_table, MaximalSegments, &Num_Segments, Options.Exclusive_Max_Radius);
					if (EPSegments == nullptr) {
						Error.Function("ExclusivePrinciple");
						Error.Value("EPSegments");
						Error.FunctionFail();
						goto ExitError;
					}
					printf("- Reduced to (%d) EP-Maximal Meaningful Segments\n", Num_Segments);
					delete[] MaximalSegments;
					MaximalSegments = EPSegments;
					EPSegments = nullptr;
				}
				printf("* Plot The Segments");
				if (Options.Max_Output_Length > 0) {
					printf(" that satisfy (length < %d)", Options.Max_Output_Length);
				}
				printf("\n");
				segments = PlotSegment(MaximalSegments, Num_Segments, size_res, size_out, Options.PlotOptions & PLOT_NEGATE);
				if (segments == nullptr) {
					Error.Function("PlotSegment");
					Error.Value("segments");
					Error.FunctionFail();
					goto ExitError;
				}
				if (Options.Superimpose != 0) {
					printf("* Superimpose plot image on original image\n");
					if ((Options.PlotOptions & PLOT_AS_RESAMPLED) != 0) {
						try {
							Superimposer(&pnm_out, pnm_res, segments, size_out, Options.Superimpose, Options.PlotOptions & PLOT_NEGATE);
						}
						catch (const std::invalid_argument& arg) {
							std::cerr << arg.what() << std::endl;
							Error.Function("Superimposer");
							Error.Value("pnm_out");
							Error.FunctionFail();
							throw;
						}
					} else {
						try {
							Superimposer(&pnm_out, pnm_orig, segments, size_out, Options.Superimpose, Options.PlotOptions & PLOT_NEGATE);
						}
						catch (const std::invalid_argument& arg) {
							std::cerr << arg.what() << std::endl;
							Error.Function("Superimposer");
							Error.Value("pnm_out");
							Error.FunctionFail();
							throw;
						}
					}
				} else {
					pnm_out.copy(PORTABLE_GRAYMAP_BINARY, size_out.width, size_out.height, pnm_orig.MaxInt(), segments);
				}
				delete[] segments;
				segments = nullptr;
				delete angles;
				angles = nullptr;
			}
			delete scratches;
			scratches = nullptr;
		}
		// X11 Plotting
		if (Options.x11_plot) {
			img_orig.reset(pnm_orig.Width(), pnm_orig.Height());
			for (size_t i = 0; i < img_orig.size(); i++) {
				img_orig[i] = int(pnm_orig[i]);
			}
			ShowSegments_X11(&img_orig, size, pnm_orig.MaxInt(), MaximalSegments, Num_Segments);
		}
		delete[] MaximalSegments;
		MaximalSegments = nullptr;

Write:
		if ((Options.mode & MODE_OUTPUT_MULTIPLE_MOTIONS_AFFINE) != 0) {
			if (imgd_prev_gray.isNULL() == false) {
				MultipleMotion_Affine_write(MultipleMotion_AffineCoeff, OutputNameNums);
			}
		} else if ((Options.mode & MODE_OUTPUT_AFFINE_BLOCKMATCHING) != 0) {
			if (imgd_prev.isNULL() == false) {
				if (sequence_RGB.size() == 2) {
					MultipleMotion_write(imgd_prev, imgd_in, pnm_orig.MaxInt(), MotionVectors, OutputNameNums);
				} else {
					MultipleMotion_write(sequence_RGB[2], sequence_RGB[1], sequence_RGB[0], pnm_orig.MaxInt(), MotionVectors, OutputNameNums_prev); // Use OutputNameNums_prev because Motion Estimation use ["prev of prev," "prev" and "current"] sequence as ["prev," "current" and "next"]
				}
			}
		} else if ((Options.mode & MODE_OUTPUT_OPTICALFLOW) != 0) {
			if (imgd_prev.isNULL() == false) {
				if (sequence_RGB.size() == 2) {
					MultipleMotion_write(imgd_prev, imgd_in, pnm_orig.MaxInt(), MotionVectors, OutputNameNums);
				} else {
					MultipleMotion_write(sequence_RGB[2], sequence_RGB[1], sequence_RGB[0], pnm_orig.MaxInt(), MotionVectors, OutputNameNums_prev); // Use OutputNameNums_prev because Motion Estimation use ["prev of prev," "prev" and "current"] sequence as ["prev," "current" and "next"]
				}
			}
		} else if ((Options.mode & MODE_OUTPUT_HISTOGRAMS_OF_ORIENTED_GRADIENTS_RAW_HOG) != 0) {
			if (HOG_write(hog_raw, OutputNameNums) == false) {
				Error.Function("HOG_write");
				Error.Value("hog");
				Error.FunctionFail();
				goto ExitError;
			}
		} else if ((Options.mode & MODE_OUTPUT_HISTOGRAMS_OF_ORIENTED_GRADIENTS) != 0) {
			if (HOG_write(hog, OutputNameNums) == false) {
				Error.Function("HOG_write");
				Error.Value("hog");
				Error.FunctionFail();
				goto ExitError;
			}
		} else if ((Options.mode & MODE_OUTPUT_HISTOGRAMS_OF_ORIENTED_GRADIENTS_MATCHING_VECTOR) != 0) {
			if (imgd_prev_gray.isNULL() == false
			    && hog_prv.Bins() == hog.Bins()) {
				printf("* Compute matching each images HOG feature\n");
				hog_vector = HOG_Matching(&hog_prv, &hog);
				if (HOG_vector_write(hog_vector, hog.Width(), hog.Height(), OutputNameNums) == false) {
					Error.Function("HOG_vector_write");
					Error.Value("hog_vector");
					Error.FunctionFail();
					goto ExitError;
				}
				HOG_vector_compensated_write(imgd_prev_gray, imgd_in_gray, hog_vector, hog.Width(), hog.Height(), OutputNameNums);
				delete[] hog_vector;
				hog_vector = nullptr;
			} else {
				printf("There are NO previous image or HOG data\n");
			}
		} else {
			if (pnm_out.write(OutputNameNums.c_str()) == PNM_FUNCTION_ERROR) {
				Error.Function("pnm_out.write");
				Error.File(OutputNameNums.c_str());
				Error.FileWrite();
				goto ExitError;
			}
		}
		delete MultipleMotion_u;
		MultipleMotion_u = nullptr;
		if (imgd_in.isNULL() == false) {
			imgd_prev = imgd_in;
			imgd_prev_gray = imgd_in_gray;
		}
		pnmd_in.free();
		pnm_out.free();
		pnm_in.free();
		pnm_res.free();
		pnm_orig.free();
	}
	delete[] hog_vector;
	delete[] k_list;
	delete Pr_table;
	return;
// Exit Error
ExitError:
	delete[] hog_vector;
	delete[] segments;
	delete[] EPSegments;
	delete[] MaximalSegments;
	delete[] k_list;
	delete MultipleMotion_u;
	delete angles;
	delete scratches;
	delete filtered;
	delete Pr_table;
	throw std::logic_error("void Scratch_MeaningfulMotion(char *OutputName, char *InputName, unsigned int OutputNameLength, unsigned int InputNameLength, int Start, int End, OPTIONS Options, FILTER_PARAM FilterParam) error");
}

