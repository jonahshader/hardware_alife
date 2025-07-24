#include "cuda_test.cuh"
#include <cuda_runtime.h>
#include <stdio.h>

__global__ void cuda_test_kernel() {
  printf("Hello from block %d, thread %d\n", blockIdx.x, threadIdx.x);
}

void runCudaTest() {
  cuda_test_kernel<<<1, 4>>>();
  cudaDeviceSynchronize();
}