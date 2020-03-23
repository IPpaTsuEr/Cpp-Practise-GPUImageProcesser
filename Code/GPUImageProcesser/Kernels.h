#pragma once
#include"Base.h"
class Kernels{
	CInfoCallback * callback;
public:
	Kernels(CInfoCallback* Muster) :callback(Muster) {};

	cl_kernel create(cl_program program, char * entryName) {
		cl_int error = 0;
		cl_kernel kernel = clCreateKernel(program, entryName, &error);
		callback->OnKernelError(error, std::stringstream() << "Create Kernel" );
		return kernel;
	}

	template<typename T>
	static T * kernelWorkInfo(cl_device_id device,cl_kernel kernel, int parama=CL_KERNEL_WORK_GROUP_SIZE) {
		//ÿ�����㵥Ԫ�����������ڵ����������������ˣ�
		cl_int error = 0;
		size_t size;
		error=clGetKernelWorkGroupInfo(kernel,device,parama,NULL,NULL,&size);
		T * t = (T *)malloc(sizeof(T)* size);
		error |= clGetKernelWorkGroupInfo(kernel, device, parama, size, &t, NULL);
#ifdef DEBUG
		Log(error, "Get KernelWorkGroup Information :");
#endif // DEBUG
		return t;
	}

	template<typename T>
	static T * kernelInfo(cl_kernel kernel , int parama= CL_KERNEL_FUNCTION_NAME) {
		size_t size;
		cl_int error = 0;
		error =clGetKernelInfo(kernel, parama, NULL, NULL, &size);
		T * t = (T *)malloc(sizeof(T)*size);
		error |= clGetKernelInfo(kernel, parama, size, t, NULL);
#ifdef DEBUG
		Log(error, "Get Kernel Information :");
#endif // DEBUG
		return t;
	}

	template<typename T>
	static T * kernelArgInfo(cl_kernel kernel, cl_uint index, int parama = CL_KERNEL_ARG_NAME) {
		size_t size;
		cl_int error = 0;
		error = clGetKernelArgInfo(kernel, index, parama, NULL, NULL, &size);
		T * t =(T *) malloc(sizeof(T)*size);
		error |= clGetKernelArgInfo(kernel, index, parama, size, t, NULL);
#ifdef DEBUG
		Log(error, "Get KernelArg Information :");
#endif // DEBUG
		return t;
	}

};