/*
 * Robust Image Segmentation with Fixed Mean Shift algorithm
 */
#include "ImageSegmentation.h"

using namespace ImgClass;



// This function will compute INVERSE Optical Flow it points the previous frame which will come to the current (next) frame.
Segmentation<Lab>
ImageSegmentation(const ImgVector<RGB>& img, const double& MaxInt, const unsigned int Mode)
{
	std::bad_alloc except_bad_alloc;

	ImgVector<RGB> img_normalize;
	ImgVector<Lab> img_Lab_normalize;
	Segmentation<Lab> segmentation;

	if (img.isNULL()) {
		throw std::invalid_argument("segmentation(const ImgVector<RGB>&, const double, const int) : const ImgVector<RGB>& img");
	} else if (MaxInt < 0) {
		throw std::invalid_argument("segmentation(const ImgVector<RGB>&, const double, const int) : const double MaxInt");
	}

	img_normalize.copy(img);
	for (size_t i = 0; i < img_normalize.size(); i++) {
		// sRGB
		img_normalize[i] /= MaxInt;
	}
	// Convert sRGB to CIE Lab
	img_Lab_normalize.reset(img_normalize.width(), img_normalize.height());
	for (size_t i = 0; i < img_normalize.size(); i++) {
		img_Lab_normalize[i].set(img_normalize[i]); // Convert RGB to L*a*b*
	}

	// Segmentation
#ifdef MEANSHIFT_KERNEL_SPATIAL
	// for images under about HD resolution
	double kernel_spatial = MEANSHIFT_KERNEL_SPATIAL;
	double kernel_intensity = 16.0 / 255.0;
#else
	// for 4K Film kernel(spatial = 64.0, intensity = 12.0 / 255.0)
	//double kernel_spatial = 64.0;
	//double kernel_intensity = 16.0 / 255.0;
	// for images under about HD resolution
	double kernel_spatial = 20.0;
	double kernel_intensity = 16.0 / 255.0;
#endif

	printf("* * Compute Segmentation by Mean Shift\n");
	segmentation.reset(img_Lab_normalize, kernel_spatial, kernel_intensity); // Set and compute segmentation

	PNM pnm;
	std::string::size_type found = 1 + newest_filename.find_last_not_of("0123456789", newest_filename.find_last_of("0123456789", newest_filename.find_last_of(".")));
	if (found == std::string::npos) {
		found = newest_filename.find_last_of(".");
	}
	std::string newest_filename_segmentation = newest_filename.substr(0, found) + "segmentation_" + newest_filename.substr(found);
	printf("* Output The Segmentation result to '%s'(binary)\n\n", newest_filename_segmentation.c_str());
	{
		ImgVector<int> tmp_vector(segmentation.width(), segmentation.height());
		for (size_t i = 0; i < segmentation.size(); i++) {
			tmp_vector[i] = static_cast<int>(segmentation[i]);
		}
		pnm.copy(PORTABLE_GRAYMAP_BINARY, segmentation.width(), segmentation.height(), int(tmp_vector.max()), tmp_vector.data());
		pnm.write(newest_filename_segmentation.c_str());
		pnm.free();
	}

	{
		int width = img_normalize.width();
		int height = img_normalize.height();

		int *img_quantized = new int[3 * segmentation.width() * segmentation.height()];
		for (const std::vector<VECTOR_2D<int> >& region : segmentation.ref_regions()) {
			RGB sum_sRGB(.0, .0, .0);
			for (const VECTOR_2D<int>& r : region) {
				sum_sRGB += img_normalize.get(r.x, r.y);
			}
			sum_sRGB *= 255.0 / region.size();
			sum_sRGB.R = sum_sRGB.R > 255.0 ? 255 : sum_sRGB.R;
			sum_sRGB.G = sum_sRGB.G > 255.0 ? 255 : sum_sRGB.G;
			sum_sRGB.B = sum_sRGB.B > 255.0 ? 255 : sum_sRGB.B;
			for (const VECTOR_2D<int>& r : region) {
				img_quantized[width * r.y + r.x] = int(sum_sRGB.R);
				img_quantized[width * height + width * r.y + r.x] = int(sum_sRGB.G);
				img_quantized[2 * width * height + width * r.y + r.x] = int(sum_sRGB.B);
			}
		}
		std::string newest_filename_quantized = newest_filename.substr(0, found) + "color-quantized_" + newest_filename.substr(found);
		printf("* Output The color quantized image '%s'(binary)\n\n", newest_filename_quantized.c_str());
		pnm.copy(PORTABLE_PIXMAP_BINARY, segmentation.width(), segmentation.height(), 255, img_quantized);
		delete[] quantized;
		quantized = nullptr;
		pnm.write(newest_filename_quantized.c_str());
		pnm.free();
	}
	// Output vectors
	std::string newest_filename_vector = newest_filename.substr(0, found) + "shift-vector_" + newest_filename.substr(found);
	FILE *fp;
	fp = fopen(newest_filename_vector.c_str(), "w");
	fprintf(fp, "%d %d\n", segmentation.width(), segmentation.height());
	for (int y = 0; y < segmentation.height(); y++) {
		for (int x = 0; x < segmentation.width(); x++) {
			VECTOR_2D<double> v;
			v.x = segmentation.ref_shift_vector_spatial().get(x, y).x - x;
			v.y = segmentation.ref_shift_vector_spatial().get(x, y).y - y;
			fwrite(&v.x, sizeof(double), 1, fp);
			fwrite(&v.y, sizeof(double), 1, fp);
		}
	}
	fclose(fp);
	return segmentation;
}

