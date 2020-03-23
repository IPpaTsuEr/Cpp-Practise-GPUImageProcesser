#pragma once
#include"Base.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<string>
//#include"InfoStruct.h"

class Programs {
	CInfoCallback * callback;
public:
	Programs(CInfoCallback * Muster) :callback(Muster) {};
	//const char * emptyProgram = "__kernel void test(){}";
 
#pragma warning(disable:4996)
	cl_program createProgram(cl_context context,cl_uint KernelCount ,int filecount, char * filename ... ) {
		 char ** codes=(char**)malloc(sizeof(char *)* filecount);
		 size_t * codeslength = (size_t *)malloc (sizeof(size_t)*filecount);
		
		 char ** ptr = &filename;
		 const char * modle ="r";
		 
		 for (int i=0;i<filecount;i++){
			 errno = 0;
			 FILE * f;// = fopen(*(ptr + i), modle);
			fopen_s(&f, *(ptr + i), modle);

			if (errno != 0) {
				callback->OnProgramError(-233,std::stringstream()<<"When Open CL Programe File Via String [" << *(ptr+i) << "] " <<strerror(errno));
			}

			fseek(f, 0, SEEK_END);
			codeslength[i] = ftell(f);
			codes[i] = (char *)malloc(sizeof(char *)* codeslength[i] + 1);
			rewind(f);
			fread(codes[i], sizeof(char), codeslength[i],f);
			codes[i][codeslength[i] + 1] = '\0';			
			fclose(f);
		 }
		 cl_int error;
		cl_program program = clCreateProgramWithSource(context, KernelCount, const_cast<const char **>(codes), codeslength, &error);

		callback->OnProgramError(error, std::stringstream()<<"Create program With Source");
		
		free(codeslength);
		for (int i = 0; i < filecount; i++)
		{
			free(codes[i]);
		}
		free(codes);

		return program;
	}



#pragma warning(disable:4996)
cl_program createProgram(cl_context *context, cl_device_id * devices, cl_uint devicecount, cl_int * results,int filecount, char * filename ...) {
		size_t * codeslength = (size_t *)malloc(sizeof(size_t)*filecount);
		unsigned char ** bincodes=(unsigned char **)malloc(sizeof(unsigned char *)*filecount);
		char ** ptr = &filename;
		const char * modle = "rb";

		for (int i = 0; i < filecount; i++)
		{
			errno = 0;
			FILE * f;// = fopen(*(ptr + i), modle);
			fopen_s(&f, *(ptr + i), modle);

			if (errno != 0) {
				callback->OnProgramError(-233, std::stringstream()<<"When Open CL Programe File Via Binary [" << *(ptr + i) << "] " << strerror(errno));
			}

			fseek(f, 0, SEEK_END);
			codeslength[i] = ftell(f);
			bincodes[i] = (unsigned char *)malloc(sizeof(unsigned char *)* codeslength[i] + 1);
			rewind(f);
			fread(bincodes[i], sizeof(unsigned char), codeslength[i], f);
			bincodes[i][codeslength[i] + 1] = '\0';
			fclose(f);
		}
		cl_int error;
		cl_program program = clCreateProgramWithBinary(*context, devicecount, devices, codeslength, const_cast<const unsigned char **>(bincodes), results, &error);
		callback->OnProgramError(error, std::stringstream()<<"Create program With Source");
		return program;
	 }
	

bool buildProgram(cl_program program,cl_device_id device, int count,const char * options="-Werror") {
		cl_int error = clBuildProgram(program, count, &device, options, NULL, NULL);
		
		callback->OnProgramError(error, std::stringstream()<<"Build Program");

		if (error != CL_SUCCESS) {
			char * buildlog=NULL;
			buildlog =programBuildInfo<char>(program, device, CL_PROGRAM_BUILD_LOG);
			callback->OnProgramError(error, std::stringstream()<<"Compile Program "<< buildlog);
			return false;
		}
		else return true;
	 }



#pragma warning(disable:4996)
bool saveToBinary(cl_program program, char * filename) {
	cl_int error;
	std::string path;
	path.append(filename);

	if(!endsWith(filename,".bin"))  path.append(".bin");

	cl_uint * ndevice=programInfo<cl_uint>(program, CL_PROGRAM_NUM_DEVICES);
	unsigned char ** bcode = (unsigned char**)malloc(sizeof(unsigned char*)*(*ndevice));
	size_t * bsize = (size_t *)malloc(sizeof(size_t)* (*ndevice));
	error=clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t)*(*ndevice), bsize,NULL);

	for (cl_uint i = 0; i < *ndevice; i++) {
		bcode[i] = (unsigned char*)malloc(sizeof(unsigned char)*bsize[i]);
	}
	error=clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(unsigned char *)*(*ndevice), bcode, NULL);
	callback->OnProgramError(error, std::stringstream()<<"Save Program To Binary File [" << filename<<"]");

		errno = 0;
		FILE * f = fopen(path.c_str(), "wb");
		 if (errno != 0) {
			 callback->OnProgramError(-233, std::stringstream()<<"Save Program To Binary When Open File [" << filename << "] "<< strerror(errno));
		 }
		 for (cl_uint i = 0; i < *ndevice; i++) {
			 errno = 0;
			 fwrite(bcode[i], sizeof(unsigned char), bsize[i], f);
			 if (errno != 0) {
				 callback->OnProgramError(-233, std::stringstream()<<"Save Program To Binary When Write File [" << filename << "] " << strerror(errno));
			 }
		 }
		 fclose(f);
	return error == CL_SUCCESS ? true : false;
	}



	template<typename T>
	static T * programInfo(cl_program program, int parame = CL_PROGRAM_KERNEL_NAMES) {
		size_t size;
		cl_int error;
		error = clGetProgramInfo(program, parame, NULL, NULL, &size);
		T * t = (T*)malloc(sizeof(T)* size);
		error |= clGetProgramInfo(program, parame, size, t, NULL);
	#ifdef DEBUG
		Log(ErrorCheck(error).str().append("When Get Program Information"));
	#endif // DEBUG
		return t;
	}

	template<typename T>
	static T *  programBuildInfo(cl_program program, cl_device_id device, int parame = CL_PROGRAM_BUILD_LOG) {
		size_t size;
		cl_int error;
		error = clGetProgramBuildInfo(program, device, parame, NULL, NULL, &size);
		T *	t = (T *)malloc(sizeof(T)*size);
		error |= clGetProgramBuildInfo(program, device, parame, size, t, NULL);
	#ifdef DEBUG
		Log(ErrorCheck(error).str().append("Get Program Build Information :"));
	#endif // DEBUG
		return t;
	}

};