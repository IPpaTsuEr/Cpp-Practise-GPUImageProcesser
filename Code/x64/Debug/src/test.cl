__kernel void Entry(__global float *A, __global float *B)
{
	int idx = get_global_id(0);
	B[idx] = A[idx]*idx;
}