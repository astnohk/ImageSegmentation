#include "ImageSegmentation.h"


const char Progress[NUM_PROGRESS][6] = {
    "\0\0\0\0\0", "\x1b[1C\0", "\x1b[2C\0", "\x1b[3C\0", "\x1b[4C\0",
    "\x1b[5C\0", "\x1b[6C\0", "\x1b[7C\0", "\x1b[8C\0", "\x1b[9C\0",
    "\x1b[10C", "\x1b[11C", "\x1b[12C", "\x1b[13C", "\x1b[14C",
    "\x1b[15C", "\x1b[16C", "\x1b[17C", "\x1b[18C", "\x1b[19C",
    "\x1b[20C", "\x1b[21C", "\x1b[22C", "\x1b[23C", "\x1b[24C",
    "\x1b[25C", "\x1b[26C", "\x1b[27C", "\x1b[28C", "\x1b[29C",
    "\x1b[30C", "\x1b[31C", "\x1b[32C", "\x1b[33C", "\x1b[34C",
    "\x1b[35C", "\x1b[36C", "\x1b[37C", "\x1b[38C", "\x1b[39C",
    "\x1b[40C", "\x1b[41C", "\x1b[42C", "\x1b[43C", "\x1b[44C",
    "\x1b[45C", "\x1b[46C", "\x1b[47C", "\x1b[48C", "\x1b[49C",
    "\x1b[50C", "\x1b[51C", "\x1b[52C", "\x1b[53C", "\x1b[54C",
    "\x1b[55C", "\x1b[56C", "\x1b[57C", "\x1b[58C", "\x1b[59C",
    "\x1b[60C", "\x1b[61C", "\x1b[62C", "\x1b[63C"};
const char Progress_End[] = "\x1b[64C|";




int
main(int argc, char *argv[])
{
	ERROR Error("main");
	const char help[] =
	    "\n"
	    "     - Image Segmentation by using Fixed Mean Shift method -\n"
	    "\n"
	    "  Process Image Segmentation by using Mean Shift method.\n"
	    "  This program can read PNM image format.\n"
	    "\n"
	    "\n"
	    "  format : segmentation -i input_%04d.pgm -o output_%04d.pgm -s [start_num] -e [end_num] [option [argument]]\n"
	    "\n"
	    "\n"
	    "    General option:\n"
	    "      -h, --help                       : show this manual\n"
	    "      -i [input]                       : set input filename\n"
	    "      -o [output]                      : set output filename\n"
	    "      -s [start_num]                   : set output filename\n"
	    "      -e [end_num]                     : set output filename\n"
	    "      --resample [(W)x(H)]             : resampling the input image to size of [WxH] before processing (use as : --resample 128x128)\n"
	    "      --resample_method [method]       : set resampling method (z-hold, bicubic)\n"
	    "      --plot_as_resampled              : output size is same as resampled image\n"
	    "      --plot_resampled_only            : output the resampled image without any other processing\n"
	    "      --x11_plot                       : show image and detected lines in 3D world by X11 library\n"
	    "\n";
	int errors = 0;

	char *delimiter = nullptr;
	char c_tmp = 0;
	std::string InputName;
	std::string OutputName;
	int Start = 0;
	int End = 0;
	OPTIONS Options;

	int inf = 0, outf = 0;
	struct {unsigned int width; unsigned int height;} tmpsize;
	bool bval = false;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strlen(argv[i]) > 2) {
				if (strcmp(argv[i], "--help") == 0) {
					printf("%s\n", help);
					return EXIT_SUCCESS;
				} else if (strcmp(argv[i], "--resample") == 0) {
					if (i + 1 >= argc) {
						fprintf(stderr, "*** Please input Width, Height or both after '--resample' option (e.g. : --resample 128x128) ***\n");
						errors |= OPTION_INSUFFICIENT;
					} else {
						i++;
						if ((delimiter = strchr(argv[i], 'x')) == nullptr) {
							if (sscanf(argv[i], "%7d", &tmpsize.height) != 1) {
								fprintf(stderr, "*** Cannot read value for ResampleSize.height ***\n");
								errors |= OPTION_INCORRECT;
							}
							tmpsize.width = tmpsize.height;
						} else {
							c_tmp = *delimiter;
							*delimiter = '\0'; // set end point of width
							if (sscanf(argv[i], "%7d", &tmpsize.width) != 1) {
								fprintf(stderr, "*** Cannot read value for ResampleSize.width ***\n");
								errors |= OPTION_INCORRECT;
							}
							if (sscanf(delimiter + 1, "%7d", &tmpsize.height) != 1) {
								fprintf(stderr, "*** Cannot read value for ResampleSize.height ***\n");
								errors |= OPTION_INCORRECT;
							}
							*delimiter = c_tmp; // recover original character
						}
						Options.set_value("ResampleSize", &tmpsize);
					}
				} else if (strcmp(argv[i], "--resample_method") == 0) {
					if (i + 1 >= argc) {
						fprintf(stderr, "*** Please input method name after '--resample_method' option ***\n");
						errors |= OPTION_INSUFFICIENT;
					} else {
						i++;
						if (strlen(argv[i]) < 1) {
							fprintf(stderr, "*** '%s' is NOT method name ***\n", argv[i]);
							errors |= OPTION_INCORRECT;
						} else {
							Options.ChangeResampleMethod(argv[i]);
						}
					}
				} else if (strcmp(argv[i], "--x11_plot") == 0) { // X11 plotting mode
					bval = true;
					Options.set_value("x11_plot", &bval);
				} else {
					fprintf(stderr, " *** Unknown option \"%s\" ***\n      - Please begin with \"--\" for long name options\n", argv[i]);
					errors |= OPTION_UNKNOWN;
				}
			} else {
				switch (argv[i][1]) {
					case 'e': // End Number
						if (i + 1 >= argc) {
							fprintf(stderr, "*** Please input value after '-e' option ***\n");
							errors |= OPTION_INSUFFICIENT;
						} else {
							i++;
							sscanf(argv[i], "%10d", &End);
							if (End <= 0) {
								End = 0;
							}
						}
						break;
					case 'h':
						printf("%s\n", help);
						return EXIT_SUCCESS;
					case 'i':
						if (i + 1 >= argc) {
							fprintf(stderr, "*** Please input INPUT FILENAME after '-i' option ***\n");
							errors |= OPTION_INSUFFICIENT;
						} else {
							i++;
							inf = i;
						}
						break;
					case 'o':
						if (i + 1 >= argc) {
							fprintf(stderr, "*** Please input OUTPUT FILENAME after '-o' option ***\n");
							errors |= OPTION_INSUFFICIENT;
						} else {
							i++;
							outf = i;
						}
						break;
					case 's': // Start Number
						if (i + 1 >= argc) {
							fprintf(stderr, "*** Please input value after '-s' option ***\n");
							errors |= OPTION_INSUFFICIENT;
						} else {
							i++;
							sscanf(argv[i], "%10d", &Start);
							if (Start <= 0) {
								Start = 0;
							}
						}
						break;
					default:
						fprintf(stderr, "*** Unknown option \"%s\" ***\n", argv[i]);
						errors |= OPTION_UNKNOWN;
				}
			}
		}
	}
	if (End == 0) {
		End = Start;
	}
	printf("\n");
	if ((errors & OPTION_INSUFFICIENT) != 0) {
		fprintf(stderr, "*** FATAL main error - the last option needs argument ***\n");
		exit(EXIT_FAILURE);
	} else if ((inf == 0) || (outf == 0)) {
	 	fprintf(stderr, "*** FATAL main error - Cannot find INPUT or OUTPUT Filename ***\n");
		exit(EXIT_FAILURE);
	} else if (Start > End) {
		fprintf(stderr, "*** FATAL main error - Start number exceeds End number (Start : %d, End : %d) ***\n", Start, End);
		exit(EXIT_FAILURE);
	}
	if ((errors & (OPTION_INCORRECT | OPTION_UNKNOWN)) != 0) {
		if ((errors & OPTION_INCORRECT) != 0) {
			fprintf(stderr, "*** main error - Some options are used incorrectly ***\n");
		}
		if ((errors & OPTION_UNKNOWN) != 0) {
			fprintf(stderr, "*** main error - Found some unknown options ***\n");
		}
		exit(EXIT_FAILURE);
	}

	// Regular expression
	InputName = argv[inf];
	OutputName = argv[outf];
	// main routine
	try {
		SequenceProcessor(OutputName, InputName, Start, End, Options);
	}
	catch (const std::logic_error& logic) {
		std::cerr << logic.what() << std::endl;
		goto ExitError;
	}
	catch (const std::runtime_error& runtime) {
		std::cerr << runtime.what() << std::endl;
		goto ExitError;
	}
	return EXIT_SUCCESS;
// Error
ExitError:
	std::cout << std::endl
	    << "        *** FATAL main error ***" << std::endl;
	return EXIT_FAILURE;
}

