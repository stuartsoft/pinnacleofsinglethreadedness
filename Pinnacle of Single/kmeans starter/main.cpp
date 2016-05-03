//includes
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <math.h>

#include "StopWatch.h"
#include "FreeImagePlus.h"

#include <emmintrin.h>
#include <xmmintrin.h>
#include <mmintrin.h>

using namespace std;

//structs...centroid and pixel
struct centroid {
	float r;
	float g;
	float b;
	float z;
};

struct pixel {
	float r;
	float g;
	float b;
	int cluster;
};

//hardcode centroid count here
const int CENTROID_COUNT = 5;

//function prototypes
//void ac(vector<pixel>&, const vector<centroid>&);	//this step should not alter the centroids
//void mc(const vector<pixel>&, vector<centroid>&);	//this step should not alter the pixels
//void fpc(vector<pixel>& pixels, const vector<centroid>& centroids);
float getdist(int x1, int y1, int z1, int x2, int y2, int z2);

inline static __m128 sseCentroidPixelDist(pixel *a,  centroid *b){
	__m128 mA = _mm_load_ps(reinterpret_cast<float*>(a));
	__m128 mB = _mm_load_ps(reinterpret_cast<float*>(b));

	__m128 res = _mm_sub_ps(mB, mA);//centroid.x,y,z - pixel.x,y,z
	__m128 res2 = _mm_mul_ps(res,res);//square results
	return res2;

}

//any other functions...assigning centroids and moving centroids
inline void ac(vector<pixel>& pixels,  vector<centroid>& centroids) {
	for (int i = 0;i<pixels.size();i++){
		int minIndex;
		float minVal = 255<<2;
		for (int j = 0;j<centroids.size();j++){
			//calc centroid pixel distance
			float *res = reinterpret_cast<float*>(&sseCentroidPixelDist(&pixels[i], &centroids[j]));

			//run sqrt to get final distance
			float diff = sqrt(res[0] + res[1] + res[2]);
				
			if (diff < minVal){
				minIndex = j;
				minVal = diff;
			}
		}

		//closest match is centroids[minIndex]
		pixels[i].cluster = minIndex;
	}
}

inline void mc(const vector<pixel>& pixels, vector<centroid>& centroids) {
	for(int j = 0;j<centroids.size();j++){
		//first, find the avg of the pixels assigned to this centroid
		float rAvg = 0;
		float gAvg = 0;
		float bAvg = 0;
		int clusterSize = 0;
		for (int i = 0;i<pixels.size();i++){
			if (pixels[i].cluster == j){
				rAvg += pixels[i].r;
				gAvg += pixels[i].g;
				bAvg += pixels[i].b;
				clusterSize++;
			}
		}

		rAvg = rAvg/clusterSize;
		gAvg = gAvg/clusterSize;
		bAvg = bAvg/clusterSize;

		centroids[j].r = rAvg;
		centroids[j].g = gAvg;
		centroids[j].b = bAvg;
	}
}

inline void fpc(vector<pixel>& pixels, const vector<centroid>& centroids){
	int index = 0;
	for (int i = 0;i<pixels.size();i++){
		index = pixels[i].cluster;
		pixels[i].r = centroids[index].r;
		pixels[i].g = centroids[index].g;
		pixels[i].b = centroids[index].b;
	}
}


//main function
int main(int argc, char** argv) {
	//initialize FreeImage library if needed
	#ifdef FREEIMAGE_LIB
	FreeImage_Initialise();
	#endif

	//hardcode the image names, create vectors and FreeImage library object
	char* file_in = "test1.jpg";
	char* file_out = "test_result.jpg";
	vector<centroid> centroids(CENTROID_COUNT);
	vector<pixel> pixels;
	fipImage input;

	StopWatch timer;

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

	//create pixel structs
	//access raw data
	//float* pixelData = reinterpret_cast<float*>(input.accessPixels());
	pixels.resize(input.getWidth() * input.getHeight());
	for(unsigned int i = 0; i < input.getWidth(); ++i) {
		for(unsigned int j = 0; j < input.getHeight(); ++j) {
			pixel temp;
			byte colors[4];
			input.getPixelColor(i, j, reinterpret_cast<RGBQUAD*>(colors));
			temp.b = colors[0] / 255.0f;
			temp.g = colors[1] / 255.0f;
			temp.r = colors[2] / 255.0f;
			temp.cluster = -1;

			pixels[j * input.getWidth() + i] = temp;
		}
	}

	timer.start();//start the timer

	//generate centroids randomly
	mt19937 generator(chrono::system_clock::now().time_since_epoch().count());
	uniform_real_distribution<float> distro(0.0f, 1.0f);
	for(int i = 0; i < CENTROID_COUNT; ++i) {
		centroid temp;
		temp.r = distro(generator);
		temp.g = distro(generator);
		temp.b = distro(generator);
		centroids[i] = temp;
	}

	for(int z = 0;z<250;z++){
		//loop unrolling
		ac(pixels, centroids);
		mc(pixels, centroids);

		ac(pixels, centroids);
		mc(pixels, centroids);

		ac(pixels, centroids);
		mc(pixels, centroids);

		ac(pixels, centroids);
		mc(pixels, centroids);

		//cout<<z<<endl;
	}
	fpc(pixels, centroids);
	double result = timer.stop();

	//write image
	//allocate output image
	fipImage output(FIT_BITMAP, input.getWidth(), input.getHeight(), 24);

	for(unsigned int i = 0; i < output.getWidth(); ++i) {
		for(unsigned int j = 0; j < output.getHeight(); ++j) {
			byte colors[4];
			int index = j * output.getWidth() + i;
			colors[0] = static_cast<byte>(pixels[index].r * 255);
			colors[1] = static_cast<byte>(pixels[index].g * 255);
			colors[2] = static_cast<byte>(pixels[index].b * 255);

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

	cout<<"Elapsed Time: "<<result/1000.0f<<" sec."<<endl;
	system("pause");

	#ifdef FREEIMAGE_LIB
	FreeImage_Uninitialise();
	#endif
	
	return 0;
}


