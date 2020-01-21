/*
 * Robust Image Segmentation with Fixed Mean Shift algorithm
 */

#include <stdexcept>
#include "ImageSegmentation.h"


using namespace ImgClass;



// This function will compute INVERSE Optical Flow it points the previous frame which will come to the current (next) frame.
Segmentation<Lab> *
ImageSegmentation(const ImgVector<RGB>& img, const double& MaxInt, const unsigned long Mode, const std::string& current_filename, const int IterMax)
{
	std::bad_alloc except_bad_alloc;

	ImgVector<RGB> img_normalize;
	ImgVector<Lab> img_Lab_normalize;
	Segmentation<Lab> *result = nullptr;

	if (img.isNULL()) {
		throw std::invalid_argument("ImageSegmentation(const ImgVector<RGB>&, const double, const int) : const ImgVector<RGB>& img");
	} else if (MaxInt < 0) {
		throw std::invalid_argument("ImageSegmentation(const ImgVector<RGB>&, const double, const int) : const double MaxInt");
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
	// for 4K Film kernel(spatial = 64.0, intensity = 12.0 / 255.0)
	//double kernel_spatial = 64.0;
	//double kernel_intensity = 16.0 / 255.0;
	// for images under about HD resolution
	double kernel_spatial = 16.0;
	double kernel_intensity = 12.0 / 255.0;

	printf("* * Compute Segmentation by Mean Shift\n");
	try {
		result = new Segmentation<Lab>;
	}
	catch (std::bad_alloc& err) {
		std::cerr << err.what() << std::endl;
		return nullptr;
	}
	result->reset(img_Lab_normalize, IterMax, kernel_spatial, kernel_intensity); // Set and compute segmentation

	PNM pnm;
	std::string::size_type found = 1 + current_filename.find_last_not_of("0123456789", current_filename.find_last_of("0123456789", current_filename.find_last_of(".")));
	if (found == std::string::npos) {
		found = current_filename.find_last_of(".");
	}
	std::string current_filename_segmentation = current_filename.substr(0, found) + "segmentation_" + current_filename.substr(found);
	printf("* Output The Segmentation result to '%s'(binary)\n\n", current_filename_segmentation.c_str());
	{
		ImgVector<int> tmp_vector(result->width(), result->height());
		for (size_t i = 0; i < result->size(); i++) {
			tmp_vector[i] = static_cast<int>(result->at(i));
		}
		pnm.copy(PORTABLE_PIXMAP_BINARY, result->width(), result->height(), 255);
		for (int y = 0; y < result->height(); y++) {
			for (int x = 0; x < result->width(); x++) {
				// 00100100 10010010 01001001
				// 01001001 00100100 10010010
				// 10010010 01001001 00100100
				pnm.at(x, y, 0) = 255 -
				      ((tmp_vector.at(x, y) & 0x000049)
				    | ((tmp_vector.at(x, y) & 0x009200) >> 8)
				    | ((tmp_vector.at(x, y) & 0x240000) >> 16));
				pnm.at(x, y, 1) = 255 -
				      ((tmp_vector.at(x, y) & 0x000092)
				    | ((tmp_vector.at(x, y) & 0x002400) >> 8)
				    | ((tmp_vector.at(x, y) & 0x490000) >> 16));
				pnm.at(x, y, 2) = 255 -
				      ((tmp_vector.at(x, y) & 0x000024)
				    | ((tmp_vector.at(x, y) & 0x004900) >> 8)
				    | ((tmp_vector.at(x, y) & 0x920000) >> 16));
			}
		}
		pnm.write(current_filename_segmentation.c_str());
		pnm.free();
	}

	{
		// Quantize image
		int width = img_normalize.width();
		int height = img_normalize.height();

		int *img_quantized = new int[3 * result->width() * result->height()];
		for (const std::vector<VECTOR_2D<int> >& region : result->ref_regions()) {
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
		std::string current_filename_quantized = current_filename.substr(0, found) + "color-quantized_" + current_filename.substr(found);
		printf("* Output The color quantized image '%s'(binary)\n\n", current_filename_quantized.c_str());
		if (pnm.copy(PORTABLE_PIXMAP_BINARY, size_t(result->width()), size_t(result->height()), 255, img_quantized) != PNM_FUNCTION_SUCCESS) {
			std::cout << "pnm.copy(PORTABLE_PIXMAP_BINARY, size_t(result->width()), size_t(result->height()), 255, img_quantized)" << std::endl;
			throw std::logic_error("ImageSegmentation(): pnm.copy()");
		}
		delete[] img_quantized;
		img_quantized = nullptr;
		if (pnm.write(current_filename_quantized.c_str()) != PNM_FUNCTION_SUCCESS) {
			std::cout << "pnm.write()" << std::endl;
			throw std::logic_error("ImageSegmentation(): pnm.write()");
		}
		pnm.free();
	}
	{
		// Output converge points
		std::string current_filename_converge = current_filename.substr(0, found) + "converge-points_" + current_filename.substr(found);
		if (pnm.copy(PORTABLE_PIXMAP_BINARY, size_t(result->width()), size_t(result->height()), 255) != PNM_FUNCTION_SUCCESS) {
			std::cout << "pnm.copy(PORTABLE_PIXMAP_BINARY, size_t(result->width()), size_t(result->height()), 255)" << std::endl;
			throw std::logic_error("ImageSegmentation(): pnm.copy()");
		}
		for (size_t n = 0; n < result->size(); n++) {
			VECTOR_2D<int> vec = result->ref_vector_converge_list_map().at(n).front();
			Lab lab = result->ref_shift_vector_color().at(vec.x, vec.y);
			RGB rgb = saturate(255.0 * RGB(lab), 0.0, 255.0);
			pnm[n] = pnm_img(rgb.R);
			pnm[n + pnm.Size()] = pnm_img(rgb.G);
			pnm[n + 2 * pnm.Size()] = pnm_img(rgb.B);
		}
		if (pnm.write(current_filename_converge.c_str()) != PNM_FUNCTION_SUCCESS) {
			std::cout << "pnm.write()" << std::endl;
			throw std::logic_error("ImageSegmentation(): pnm.write()");
		}
	}
	{
		// Output vectors
		std::string current_filename_vector = current_filename.substr(0, found) + "shift-vector_" + current_filename.substr(found) + ".dat";
		FILE *fp;
		fp = fopen(current_filename_vector.c_str(), "w");
		fprintf(fp, "%d %d\n", result->width(), result->height());
		double norm_max = 0.0;
		ImgVector<HSV> vector_hsv(result->width(), result->height());
		for (int y = 0; y < result->height(); y++) {
			for (int x = 0; x < result->width(); x++) {
				VECTOR_2D<double> v;
				v.x = result->ref_shift_vector_spatial().get(x, y).x - x;
				v.y = result->ref_shift_vector_spatial().get(x, y).y - y;
				vector_hsv.at(x, y).set_H(arg(v) / M_PI * 0.5);
				vector_hsv.at(x, y).S = 1.0;
				vector_hsv.at(x, y).V = sqrt(1.0 + norm(v)) - 1.0;
				if (norm_max < vector_hsv.at(x, y).V) {
					norm_max = vector_hsv.at(x, y).V;
				}
				fwrite(&v.x, sizeof(double), 1, fp);
				fwrite(&v.y, sizeof(double), 1, fp);
			}
		}
		fclose(fp);
		// Plot vectors by using HSV color space
		if (pnm.copy(PORTABLE_PIXMAP_BINARY, size_t(result->width()), size_t(result->height()), 255) != PNM_FUNCTION_SUCCESS) {
			std::cout << "pnm.copy(PORTABLE_PIXMAP_BINARY, size_t(result->width()), size_t(result->height()), 255)" << std::endl;
			throw std::logic_error("ImageSegmentation(): pnm.copy()");
		}
		for (size_t n = 0; n < vector_hsv.size(); n++) {
			vector_hsv[n].V /= norm_max;
			RGB rgb = saturate(255.0 * RGB(vector_hsv[n]), 0.0, 255.0);
			pnm[n] = pnm_img(rgb.R);
			pnm[n + pnm.Size()] = pnm_img(rgb.G);
			pnm[n + 2 * pnm.Size()] = pnm_img(rgb.B);
		}
		std::string current_filename_vector_image = current_filename.substr(0, found) + "shift-vector-img_" + current_filename.substr(found);
		if (pnm.write(current_filename_vector_image.c_str()) != PNM_FUNCTION_SUCCESS) {
			std::cout << "pnm.write()" << std::endl;
			throw std::logic_error("ImageSegmentation(): pnm.write()");
		}
	}
	return result;
}

