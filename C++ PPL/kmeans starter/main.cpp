//includes
#include <chrono>
#include <iostream>
#include "FreeImagePlus.h"
#include "KMeans.h"
#include "StopWatch.h"

using namespace std;

//function prototypes
float getdist(int x1, int y1, int z1, int x2, int y2, int z2);

int main(int argc, char** argv) {
	//initialize FreeImage library if needed
	#ifdef FREEIMAGE_LIB
	FreeImage_Initialise();
	#endif

	//hardcode the image names, create vectors and FreeImage library object
	char* file_in = "test.jpg";
	char* file_out = "test_result.jpg";
	vector<centroid> centroids(CENTROID_COUNT);

	// Use a standard array instead of vector
	centroid centroid_array[CENTROID_COUNT];

	vector<pixel> pixels;
	fipImage input;

	KMeans img;

	//generate centroids randomly
	mt19937 generator(chrono::system_clock::now().time_since_epoch().count());
	uniform_real_distribution<float> distro(0.0f, 1.0f);
	for(int i = 0; i < CENTROID_COUNT; ++i) {
		centroid temp;
		temp.r = distro(generator);
		temp.g = distro(generator);
		temp.b = distro(generator);

		img.centroids[i] = temp;
	}

	//open and load image as per convention from freeimage
	if(!input.load(file_in)) {
		cout << "Could not load file with name " << file_in << endl;
		return 1;
	}

	FREE_IMAGE_TYPE originalType = input.getImageType();

	if(!input.convertTo24Bits()) {
		cout << "Error occurred when converting pixels to float values." << endl;
		return 1;
	}

	// Assign common method results to variables to save access times
	unsigned int width = input.getWidth();
	unsigned int height = input.getHeight();

	//create pixel structs
	//access raw data
	//float* pixelData = reinterpret_cast<float*>(input.accessPixels());
	img.pixels.resize(width * height);

	for (unsigned int i = 0; i < width; ++i) {
		for (unsigned int j = 0; j < height; ++j) {
			pixel temp;
			byte colors[4];
			input.getPixelColor(i, j, reinterpret_cast<RGBQUAD*>(colors));
			temp.b = colors[0] / 255.0f;
			temp.g = colors[1] / 255.0f;
			temp.r = colors[2] / 255.0f;
			temp.cluster = -1;

			img.pixels[j * width + i] = temp;
		}
	}
	
	StopWatch timer;

	timer.start();
	for(int z = 0;z<1000;z++){
		img.assignCentroids();
		img.moveCentroids();
		//cout<<z<<endl;
	}
	img.assignFinalPixelColors();
	timer.stop();

	//write image
	//allocate output image
	fipImage output(FIT_BITMAP, width, height, 24);

	unsigned int outWidth = output.getWidth();
	unsigned int outHeight = output.getHeight();

	for(unsigned int i = 0; i < outWidth; ++i) {
		for(unsigned int j = 0; j < outHeight; ++j) {
			byte colors[4];
			int index = j * outWidth + i;
			colors[0] = static_cast<byte>(img.pixels[index].b * 255);
			colors[1] = static_cast<byte>(img.pixels[index].g * 255);
			colors[2] = static_cast<byte>(img.pixels[index].r * 255);

			output.setPixelColor(i, j, reinterpret_cast<RGBQUAD*>(colors));
		}
	}

	if(!output.convertToType(originalType)) {
		cout << "Could not convert back to 24 bits for image saving." << endl;
		return 1;
	}

	if(!output.save(file_out)) {
		cout << "Something went wrong with filesaving" << endl;
		return 1;
	}

	#ifdef FREEIMAGE_LIB
	FreeImage_Uninitialise();
	#endif
	
	return 0;
}

