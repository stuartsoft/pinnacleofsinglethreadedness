# pinnacleofsinglethreadedness
##Documentation:

This repo contains multiple projects, each showcasing different implementations of the K-Means algorithm.

First, we have a traditional single threaded approach, which takes a while but has clearly the most readable code. 

We optimized that algorithm in "Pinnacle of Single" wherein we took advantage of loop unrolling, bit fiddling, and SSE instructions.

Next, we used traditional multithreaded approches such as OMP and the c++11 concurrency library to maximise performance on the CPU.

Lastly, we developed two approches on the GPU using Open-CL. "OCL-naive" performs labeling on the GPU, but has to migrate
data back to the CPU to move centroids and check for convergence. "OCL-complex" uses a more sophisticated method of reductions
to allow the entire k-means algorithm to run on the GPU. Although some portions of this algoirthm are inefficient on a GPU,
the lack of overhead moving data between the CPU and GPU is worth it.
