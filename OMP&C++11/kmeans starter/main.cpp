#define TESTING_SINGLE
#define TESTING_OMP
#define TESTING_CPP

//includes
#include<thread>
#include<future>
#include<mutex>
#include<atomic>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include "FreeImagePlus.h"
#include <limits>
#include "StopWatch.h"
#include<deque>


using namespace std;

//structs...centroid and pixel
struct colorPoint {
	colorPoint():r(0),g(0),b(0){}
	colorPoint(float r, float g,float b):r(r),g(g),b(b){}
	float r;
	float g;
	float b;
	colorPoint operator- (const colorPoint &o)const {
		return colorPoint(r-o.r,g-o.g,b-o.b);
	}
	void operator+= (const colorPoint &o) {
		r+=o.r;
		g+=o.g;
		b+=o.b;
	}
	colorPoint operator/ (const float &o)const {
		return colorPoint(r/o,g/o,b/o);
	}
	float sum(){
		return r+g+b;
	}
};

struct centroid: public colorPoint{
	int cluster;
	centroid& operator=(const colorPoint& d){
		r=d.r;g=d.g;b=d.b;
		return *this;
	}
};

struct pixel: public colorPoint {
	int cluster;
};

//hardcode centroid count here
const int CENTROID_COUNT = 5;

//function prototypes
void assignCentroids(vector<pixel>&, const vector<centroid>&);
void moveCentroids(const vector<pixel>&, vector<centroid>&);

void assignCentroidsOMP(vector<pixel>&, const vector<centroid>&);
void moveCentroidsOMP(const vector<pixel>&, vector<centroid>&);

void assignCentroidsC11(vector<pixel>&, const vector<centroid>&);
void moveCentroidsC11(const vector<pixel>&, vector<centroid>&);

bool didCenteroidsMove(const vector<centroid>&,const vector<centroid>&,float);
float manhattanDist(const colorPoint&,const colorPoint &);
void printImg(const char*, const vector<pixel>&, const vector<centroid>&, const fipImage&, const FREE_IMAGE_TYPE&);

//main function
int main(int argc, char** argv) {

	//initialize FreeImage library if needed
	#ifdef FREEIMAGE_LIB
	FreeImage_Initialise();
	#endif

	//hardcode the image names, create vectors and FreeImage library object
	char* file_in = "rainbow.jpg";
	char* file_out = "rainbow_result.jpg";
	vector<centroid> centroids(CENTROID_COUNT);
	vector<pixel> pixels;
	fipImage input;

	//generate centroids randomly
	mt19937 generator(chrono::system_clock::now().time_since_epoch().count());
	uniform_real_distribution<float> distro(0.0f, 1.0f);
	for(int i = 0; i < CENTROID_COUNT; ++i) {
		centroid temp;
		temp.r = distro(generator);
		temp.g = distro(generator);
		temp.b = distro(generator);
		temp.cluster = i;
		centroids[i] = temp;
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
	

	StopWatch timer;
	vector<centroid> oldCentroid;
	vector<centroid> originalCentroids = centroids;
	vector<pixel> originalPixels = pixels;
	
#ifdef TESTING_SINGLE

	centroids = originalCentroids;
	pixels = originalPixels;
	timer.start();
	do{
		oldCentroid = centroids;
		assignCentroids(pixels,centroids);
		moveCentroids(pixels,centroids);
	}while(didCenteroidsMove(oldCentroid,centroids,0.01));//end loop
	timer.stop();
	cout<<"Single\t"<<timer.stop()<<endl;
	timer.reset();
	printImg("SingleThreaded_Result.jpg",pixels,centroids,input,originalType);

#endif
#ifdef TESTING_OMP

	centroids = originalCentroids;
	pixels = originalPixels;
	timer.start();
	do{
		oldCentroid = centroids;
		assignCentroidsOMP(pixels,centroids);
		moveCentroidsOMP(pixels,centroids);
	}while(didCenteroidsMove(oldCentroid,centroids,0.01));//end loop
	timer.stop();
	cout<<"OMP\t"<<timer.stop()<<endl;
	timer.reset();
	printImg("OMP_Result.jpg",pixels,centroids,input,originalType);
#endif
#ifdef TESTING_CPP
	centroids = originalCentroids;
	pixels = originalPixels;
	timer.start();
	do{
		oldCentroid = centroids;
		assignCentroidsC11(pixels,centroids);
		moveCentroidsC11(pixels,centroids);
	}while(didCenteroidsMove(oldCentroid,centroids,0.01));//end loop
	timer.stop();
	cout<<"C++11\t"<<timer.stop()<<endl;
	timer.reset();
	printImg("C++11_Result.jpg",pixels,centroids,input,originalType);
#endif
	

	

	#ifdef FREEIMAGE_LIB
	FreeImage_Uninitialise();
	#endif
	
	return 0;
}

void printImg(const char* file_out, const vector<pixel>& pixels, const vector<centroid>& centroids, const fipImage& input, const FREE_IMAGE_TYPE& originalType){
	//write image
	//allocate output image
	fipImage output(FIT_BITMAP, input.getWidth(), input.getHeight(), 24);

	for(unsigned int i = 0; i < output.getWidth(); ++i) {
		for(unsigned int j = 0; j < output.getHeight(); ++j) {
			byte colors[4];
			int cluster = pixels[j * output.getWidth() + i].cluster;
			if(cluster==-1)cluster=0;
			colors[0] = static_cast<byte>(centroids[cluster].b * 255);
			colors[1] = static_cast<byte>(centroids[cluster].g * 255);
			colors[2] = static_cast<byte>(centroids[cluster].r * 255);

			output.setPixelColor(i, j, reinterpret_cast<RGBQUAD*>(colors));
		}
	}

	if(!output.convertToType(originalType)) {
		cout << "Could not convert back to 24 bits for image saving." << endl;
	}

	if(!output.save(file_out)) {
		cout << "Something went wrong with filesaving" << endl;
	}
}


//uses manhattan distance for I am lazy
float manhattanDist(const colorPoint& p1, const colorPoint& p2){
	return abs((p1-p2).sum());
}

bool didCenteroidsMove(const vector<centroid>& c1, const vector<centroid>& c2, float eps){
	float sum = 0;
	for(int i = 0 ; i < c1.size(); i++){
		sum += manhattanDist(c1[i],c2[i]);
	}
	return (sum>eps);
}


void assignCentroids(vector<pixel>& pixels, const vector<centroid>& centroids) {
	for(pixel& p : pixels){
		float min=INFINITE;
		for(const centroid& c : centroids){
			float d = manhattanDist(p,c);
			if(min>d){
				p.cluster = c.cluster;
				min = d;
			}
		}
	}
}

void moveCentroids(const vector<pixel>& pixels, vector<centroid>& centroids) {

	vector<int> counts(centroids.size(),0);
	vector<colorPoint> sums(centroids.size(),colorPoint());

	for(const pixel& p : pixels){
		counts[p.cluster]++;
		sums[p.cluster]+=p;
	}
	for(centroid &c : centroids){
		c = sums[c.cluster]/counts[c.cluster];
	}

}



//any other functions...assigning centroids and moving centroids
void assignCentroidsOMP(vector<pixel>& pixels, const vector<centroid>& centroids) {
	//for each pixel
	//find its distance to every centeriod
	//set its value to the minimum centeroid
#pragma omp parallel for
	for(int i=0;i<pixels.size();i++){
		pixel &p = pixels[i];
		float min=INFINITE;
		for(const centroid& c : centroids){
			float d = manhattanDist(p,c);
			if(min>d){
				p.cluster = c.cluster;
				min = d;
			}
		}
	}
}

void moveCentroidsOMP(const vector<pixel>& pixels, vector<centroid>& centroids) {

	vector<int> counts(centroids.size(),0);
	vector<colorPoint> sums(centroids.size(),colorPoint());

#pragma omp parallel
	{
		//thread private members
		vector<int> myCounts(centroids.size(),0);
		vector<colorPoint> mySums(centroids.size(),colorPoint());

		//split for loop around
#pragma omp for
		for(int i=0;i<pixels.size();i++){
			const pixel &p = pixels[i];
			myCounts[p.cluster]++;
			mySums[p.cluster]+=p;
		}

		//reduce
#pragma omp critical
		{
			for(int i=0;i<centroids.size();i++){
				counts[i] += myCounts[i];
				sums[i] += mySums[i];
			}
		}
	}

//#pragma omp parallel for
	for(centroid &c : centroids){
			c = sums[c.cluster]/counts[c.cluster];
		}

}

void assignHelper(int start,int end, vector<pixel>* pixels, const vector<centroid>* centroids){
	for(int i=start;i<end;i++){
		pixel &p = (*pixels)[i];
		float min=INFINITE;
		for(const centroid& c : *centroids){
			float d = manhattanDist(p,c);
			if(min>d){
				p.cluster = c.cluster;
				min = d;
			}
		}
	}
}

//any other functions...assigning centroids and moving centroids
void assignCentroidsC11(vector<pixel>& pixels, const vector<centroid>& centroids) {
	vector<thread> threads;
	int numHwThr = thread::hardware_concurrency();
	int range = pixels.size()/numHwThr;
	for(int i = 0; i < numHwThr;i++){
		if(i+1==numHwThr){
			threads.push_back(thread(assignHelper,i*range,pixels.size(),&pixels,&centroids));
		} else {
			threads.push_back(thread(assignHelper,i*range,(i+1)*range,&pixels,&centroids));
		}
	}
	for(thread& t : threads) t.join();
}

struct moveHelperData{
	const vector<pixel>* pixels;
	vector<centroid>* centroids;
	vector<atomic_int> *counts;
	vector<colorPoint>* sums;
	mutex* sumLocks;
	int size;
};

void moveHelper(int start,int end, moveHelperData d){

	vector<int> counts(d.size);
	vector<colorPoint> sums(d.size);

	//calc
	for(int i=start;i<end;i++){
		const pixel& p = d.pixels->at(i);
		int cluster = p.cluster;
		counts[cluster]++;
		sums[cluster] += p;
	}
	
	//reduce
	for(int i = 0; i<d.size;i++){
		d.counts->at(i)+=counts[i];
		d.sumLocks[i].lock();
		d.sums->at(i)+=sums[i];
		d.sumLocks[i].unlock();
	}
}

void moveCentroidsC11(const vector<pixel>& pixels, vector<centroid>& centroids) {
	int size = centroids.size();
	vector<atomic_int> counts(size);
	vector<colorPoint> sums(size);
	mutex * sumLocks = new mutex[size];

	moveHelperData data = {&pixels,&centroids,&counts,&sums,sumLocks,size};

	vector<thread> threads;
	int numHwThr = thread::hardware_concurrency();
	int range = pixels.size()/numHwThr;
	for(int i = 0; i < numHwThr;i++){
		if(i+1==numHwThr){
			threads.push_back(thread(moveHelper,i*range,pixels.size(),data));
		} else {
			threads.push_back(thread(moveHelper,i*range,(i+1)*range,data));
		}
	}
	for(thread& t : threads) t.join();

	delete [] sumLocks;

	for(centroid &c : centroids){
		c = sums[c.cluster]/counts[c.cluster];
	}
}
