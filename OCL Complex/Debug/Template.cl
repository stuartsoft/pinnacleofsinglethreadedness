/*****************************************************************************
 * Copyright (c) 2013-2016 Intel Corporation
 * All rights reserved.
 *
 * WARRANTY DISCLAIMER
 *
 * THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
 * MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Intel Corporation is the author of the Materials, and requests that all
 * problem reports or change requests be submitted to it directly
 *****************************************************************************/

 float dist(float3 p, float3 c){
	return sqrt(pow(p.x-c.x,2)+pow(p.y-c.y,2)+pow(p.z-c.z,2));
}

bool conv(__local float3* c1, __local float3*c2){
	float acc = 0;
	int i;
	for(i=0;i<5;i++){
		acc += fabs(c1[i].x-c2[i].x);
		acc += fabs(c1[i].y-c2[i].y);
		acc += fabs(c1[i].z-c2[i].z);
	}
	return acc < 0.0001;
}

//pixels x-r y-g z-b w-label
//centroid x-r y-g z-b
//centroidScratch index-centroid x-r y-b z-g w-count 
__kernel void Kmeans(__global float3* pixels, __global float4* centroidScratch, __global float3* output, __global float3* GLOBALcentroids)
{
	const int CENTROID_COUNT = 5;
	const int numThreads = get_global_size(0)*get_global_size(1);
    const int id = get_global_id(1) * get_global_size(0) + get_global_id(0);
	const int lid = get_local_id(1) * get_local_size(0) + get_local_id(0);
	float minVal=999;
	float currDist;
	int halfSize;
	float4 temp;
	int label = 0;
	int i = 0;
	float3 currPix = pixels[id]; 
	__local float3 centroids[5];
	__local float3 oldCentroids[5];
	if(lid<CENTROID_COUNT){
		centroids[lid] = GLOBALcentroids[lid];
	}
	do{
		barrier(CLK_GLOBAL_MEM_FENCE);
		if(lid<CENTROID_COUNT){
			oldCentroids[lid]=centroids[lid];	
		}
		barrier(CLK_GLOBAL_MEM_FENCE);
		//LABEL
		minVal = 9999;
		for(i=0; i < CENTROID_COUNT; i++){
			currDist = dist(currPix,centroids[i]);
			if(minVal > currDist){
				minVal = currDist;
				label = i;
			 }
		}
		barrier(CLK_GLOBAL_MEM_FENCE);
		//prepare scratch
		for(i=0;i<CENTROID_COUNT;i++){
			centroidScratch[id*CENTROID_COUNT+i] = (float4)(0.0f,0.0f,0.0f,0.0f);
		}
		centroidScratch[id*CENTROID_COUNT+label] = (float4)(currPix.x,currPix.y,currPix.z,1.0f);
		//reduce
		for(halfSize=numThreads/2;halfSize>0;halfSize>>=1){
			barrier(CLK_GLOBAL_MEM_FENCE);
			if(id<halfSize && (id+halfSize) < numThreads){
				for(i=0;i<CENTROID_COUNT;i++){
					centroidScratch[id*CENTROID_COUNT+i] += centroidScratch[(id+halfSize)*CENTROID_COUNT+i];
				}
			}
		}
		barrier(CLK_GLOBAL_MEM_FENCE);
		if(id<CENTROID_COUNT){
			temp = centroidScratch[id];
			temp.x /= temp.w;
			temp.y /= temp.w;
			temp.z /= temp.w;
			centroidScratch[id] = temp;
		}
		barrier(CLK_GLOBAL_MEM_FENCE);
		//load moved centroids to local
		if(lid<CENTROID_COUNT){
			temp = centroidScratch[lid];
			centroids[lid] = (float3)(temp.x,temp.y,temp.z);
		}
		barrier(CLK_GLOBAL_MEM_FENCE);
	}while(!conv(oldCentroids,centroids));
	output[id]=centroids[label];
}



