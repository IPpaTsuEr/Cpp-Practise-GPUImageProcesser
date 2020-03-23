#pragma once
#include<cl/cl.h>
#include"Base.h"



class Buffers {
	CInfoCallback *callback;
public:
	Buffers(CInfoCallback * Muster):callback(Muster) {};

	cl_mem createBufferAndSet(cl_uint parameterIndex, cl_kernel kernel, cl_context context, size_t size, void * data,cl_mem_flags types=CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR) {
		cl_int error;
		cl_mem one=clCreateBuffer(context, types, size, data, &error);
		if(error==CL_SUCCESS)error=clSetKernelArg(kernel, parameterIndex, sizeof(cl_mem), &one);
		callback->OnBufferError(error, std::stringstream() << "Create & Set KernelArg With Index:" << parameterIndex << " Size:" << size);
		return one;
	}

	void getBuffer(cl_command_queue queue, cl_mem mem,size_t length, void * data, size_t offset = 0, cl_bool block =CL_FALSE) {
		cl_event endevent;
		cl_int error = clEnqueueReadBuffer(queue, mem, block, offset, length, data, 0, NULL, &endevent);
		if (error == CL_SUCCESS) {
			error = clWaitForEvents(1, &endevent);
			if (error == CL_SUCCESS)error=clReleaseEvent(endevent);
		}
		callback->OnBufferError(error, std::stringstream() << "Get Buffer Data With Size:" << length);
 
	}

	void setBuffer(cl_command_queue queue,cl_mem buffer,size_t size,void * data,cl_bool block=CL_FALSE) {
		cl_event endevent;
		cl_int error;
		error= clEnqueueWriteBuffer(queue, buffer, block, 0, size, data, 0, NULL, &endevent);
		if (error == CL_SUCCESS) {
			error = clWaitForEvents(1, &endevent);
			if (error == CL_SUCCESS)error = clReleaseEvent(endevent);
		}
		callback->OnBufferError(error, std::stringstream() << "Set Buffer Data With Size:" << size);
 
	}

	void setImage(cl_command_queue queue,cl_mem image,void * data,size_t endX,size_t endY) {
		size_t origin[3] = { 0,0,0 };
		size_t region[3] = { endX,endY,1 };
		cl_event endevent;
		cl_int error;
		error=clEnqueueWriteImage(queue, image, CL_FALSE, origin, region, 0, 0, data, 0, NULL, &endevent);
		if (error == CL_SUCCESS) {
			error = clWaitForEvents(1, &endevent);
			if (error == CL_SUCCESS)error = clReleaseEvent(endevent);
		}
		callback->OnBufferError(error, std::stringstream() << "Set Image Buffer Data ");
	}

	static void mapBuffer(void ** outPtr,cl_command_queue queue,cl_mem mem,size_t size,size_t offset=0,cl_map_flags flags=CL_MAP_WRITE,cl_bool block=CL_TRUE, cl_event * finishevent=NULL) {
		cl_int error;
		*outPtr = clEnqueueMapBuffer(queue, mem, block,flags,offset,size,0, NULL, finishevent,&error);
#ifdef DEBUG
		Log(ErrorCheck(error).str().append("When Mapping Buffer"));
#endif // DEBUG
	}

	static void unmapBuffer(cl_command_queue queue,cl_mem mem,void * mapptr,cl_event* finishevent=NULL) {
		cl_int error=clEnqueueUnmapMemObject(queue, mem, mapptr, 0, NULL, finishevent);
#ifdef DEBUG
		Log(ErrorCheck(error).str().append("When Umapping Buffer"));
#endif // DEBUG
	}
	cl_mem createBuffer(cl_context context,size_t size,void * data,cl_mem_flags types) {
		cl_int error;
		cl_mem m=clCreateBuffer(context, types, size, data, &error);
		callback->OnBufferError(error, std::stringstream() << "Create Buffer With Size:" << size);
		return m;
	}

	void copy(cl_command_queue queue,cl_mem src ,cl_mem des,size_t dataLength,size_t srcOffset=0,size_t desOffset=0) {
		cl_event finishevent;
		cl_int error=clEnqueueCopyBuffer(queue, src, des, srcOffset, desOffset, dataLength, 0, NULL, &finishevent);
		if (error == CL_SUCCESS) {
			error = clWaitForEvents(1, &finishevent);
			if (error == CL_SUCCESS)error = clReleaseEvent(finishevent);
		}
		callback->OnBufferError(error, std::stringstream() <<"When Copy Buffer Data With Size:"<< dataLength );

	}


static	cl_mem createImageAndSet(cl_uint parameterIndex,cl_kernel kernel,cl_context context,void * data,cl_image_format *format,cl_image_desc *desc, cl_mem_flags types= CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR) {
	    cl_int error;

		cl_mem one = clCreateImage(context, types, format, desc, data, &error);
#ifdef DEBUG
		Log(ErrorCheck(error).str().append("When Create Image Buffer"));
#endif // DEBUG

		error = clSetKernelArg(kernel, parameterIndex, sizeof(cl_mem), &one);
#ifdef DEBUG
		Log(ErrorCheck(error).str().append("When Set Image Paramaters"));
#endif // DEBUG

		return one;
	}

static bool getImageData(cl_command_queue &queue,cl_mem &image,size_t endX,size_t endY, void * data,size_t startX=0,size_t startY=0) {
		size_t start[] = { startX,startY,0 },
			     end[] = { endX,endY ,1 };
		cl_int error;
		cl_event endevent;
		error = clEnqueueReadImage(queue, image, CL_TRUE, start, end, 0, 0, data, 0, NULL, &endevent);
		if (error == CL_SUCCESS) {
			error = clWaitForEvents(1, &endevent);
			if (error == CL_SUCCESS)error = clReleaseEvent(endevent);
		}
#ifdef DEBUG
		Log(ErrorCheck(error).str().append("When Get Image Data"));
#endif // DEBUG		
		
		if (error == CL_SUCCESS)return true;
		return false;
	}

	template<typename T>
	static T * getAlign(size_t size,size_t aliginsize=4096) {
		size_t datasize = ((sizeof(T) * size - 1) / 64 + 1) * 64;
		return (T *)_aligned_malloc(datasize, aliginsize);
	}
	
	static void releaseAlign(void * ptr) {
		if(ptr!=NULL)_aligned_free(ptr);
	}


};

