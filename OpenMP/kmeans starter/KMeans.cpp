#pragma once

#include "KMeans.h"


//any other functions...assigning centroids and moving centroids
void KMeans::assignCentroids()
{
	#pragma omp parallel for
	for (int i = 0; i < pixels.size(); ++i) {
		int minIndex;
		float minVal = 255 * 3;
		pixel at_i = pixels[i];

		for (int j = 0; j < CENTROID_COUNT; j++) {
			centroid at_j = centroids[j];

			float diff = sqrt((at_j.r - at_i.r) * (at_j.r - at_i.r)
				+ (at_j.g - at_i.g) *(at_j.g - at_i.g)
				+ (at_j.b - at_i.b) *(at_j.b - at_i.b));

			if (diff < minVal) {
				minIndex = j;
				minVal = diff;
			}

		}

		//closest match is centroids[minIndex]
		pixels[i].cluster = minIndex;
	}
}

void KMeans::moveCentroids()
{
	#pragma omp parallel for
	for (int j = 0; j < CENTROID_COUNT; ++j) {
		//first, find the avg of the pixels assigned to this centroid
		float rAvg = 0;
		float gAvg = 0;
		float bAvg = 0;
		int clusterSize = 0;
		for (int i = 0; i < pixels.size(); ++i) {
			if (pixels[i].cluster == j) {
				rAvg += pixels[i].r;
				gAvg += pixels[i].g;
				bAvg += pixels[i].b;
				clusterSize++;
			}
		}

		rAvg = rAvg / clusterSize;
		gAvg = gAvg / clusterSize;
		bAvg = bAvg / clusterSize;

		centroids[j].r = rAvg;
		centroids[j].g = gAvg;
		centroids[j].b = bAvg;
	}
}

void KMeans::assignFinalPixelColors()
{
	#pragma omp parallel for
	for (int i = 0; i < pixels.size(); ++i) {
		pixels[i].r = centroids[pixels[i].cluster].r;
		pixels[i].g = centroids[pixels[i].cluster].g;
		pixels[i].b = centroids[pixels[i].cluster].b;
	}
}
