#CC=g++
CC=g++ -std=c++11
WARNING=-Wall -Wextra
LIBES=-lm -lX11
OPTION=-O2 -fopenmp
MACROS=


MAIN_CFILES = main.cpp ImageSegmentation.cpp
LIBRARY_CFILES = Error.cpp Option.cpp
IMGCLASS_CFILES = ImgClass/Color.cpp ImgClass/Lab.cpp ImgClass/RGB.cpp ImgClass/Segmentation.cpp
PNM_CFILES = pnm_lib_cpp/pnm.cpp pnm_lib_cpp/pnm_double.cpp pnm_lib_cpp/pnm_library.cpp

CFILES = $(MAIN_CFILES) $(LIBRARY_CFILES) $(IMGCLASS_CFILES) $(MEANINGFUL_CFILES) $(OPTICALFLOW_CFILES) $(HOG_CFILES) $(PLOT_CFILES) $(PNM_CFILES)


MAIN_OFILES = main.o ImageSegmentation.o
LIBRARY_OFILES = Error.o Option.o
IMGCLASS_OFILES = Color.o Lab.o RGB.o Segmentation.o
PNM_OFILES = pnm.o pnm_double.o pnm_library.o

OFILES = $(MAIN_OFILES) $(LIBRARY_OFILES) $(IMGCLASS_OFILES) $(MEANINGFUL_OFILES) $(OPTICALFLOW_OFILES) $(HOG_OFILES) $(PLOT_OFILES) $(PNM_OFILES)

OUTNAME = ImageSegmentation




ImageSegmentation: $(OFILES)
	$(CC) $(WARNING) $(LIBES) $(OPTION) -o $(OUTNAME) $^


main.o: main.cpp
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^

ImageSegmentation.o: ImageSegmentation.cpp
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^

Error.o: Error.cpp
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^

Color.o: ImgClass/Color.cpp
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^

Lab.o: ImgClass/Lab.cpp
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^

RGB.o: ImgClass/RGB.cpp
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^

Segmentation.o: ImgClass/Segmentation.cpp
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^


pnm.o: pnm_lib_cpp/pnm.cpp
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^

pnm_double.o: pnm_lib_cpp/pnm_double.cpp
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^

pnm_library.o: pnm_lib_cpp/pnm_library.cpp
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^




debug: $(CFILES)
	$(CC) $(WARNING) $(LIBES) -g -O2 $(MACROS) -o $(OUTNAME) $^

debugmp: $(CFILES)
	$(CC) $(WARNING) $(LIBES) $(OPTION) $(MACROS) -g -O2 -o $(OUTNAME) $^

clean:
	rm -f $(OFILES)
	find -name "*.gch" -exec rm {} +

