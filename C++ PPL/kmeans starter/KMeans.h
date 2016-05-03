#pragma once
#include <math.h>
#include <vector>
#include <random>
#include <mmintrin.h>
#include <emmintrin.h>
#include <ppl.h>

using std::vector;

struct centroid;
struct pixel;
struct KMeans;

//hardcode centroid count here
const int CENTROID_COUNT = 3;

//structs...centroid and pixel
struct centroid
{
	float r;
	float g;
	float b;
};

struct pixel
{
	float r;
	float g;
	float b;
	int cluster;
};

struct KMeans
{
	KMeans() {}
	~KMeans() {}
	void assignCentroids();
	void moveCentroids();
	void assignFinalPixelColors();
	vector<pixel> pixels;
	centroid centroids[CENTROID_COUNT];
};
