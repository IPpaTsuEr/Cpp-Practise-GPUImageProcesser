#pragma once
#include"Base.h"

class Contexts {
	CInfoCallback * callback;
	cl_context_properties properties[3] = { CL_CONTEXT_PLATFORM,NULL,0 };
public:
	Contexts(CInfoCallback * Muster) :callback(Muster) {};
	
	cl_context create(cl_platform_id * platform, int deviceType = CL_DEVICE_TYPE_GPU) {
		cl_int error;
		properties[1] = (cl_context_properties)*platform;
		cl_context context= clCreateContextFromType(properties, deviceType, NULL, NULL, &error);
		callback->OnContextError(error, std::stringstream() << "Create Context");
		return context;
	}


	template<typename T>
	static T * getContextInfo(cl_context & context,int infoType=CL_CONTEXT_PROPERTIES) {
		size_t size;
		cl_int error;
		clGetContextInfo(context, infoType, NULL, NULL, &size);
		T * t = (T *)malloc(sizeof(T)*size);
		error=clGetContextInfo(context, infoType, size, t, NULL);
#ifdef DEBUG
		Log(ErrorCheck(error).str().append("When Get Context Information"));
#endif // DEBUG
		return t;
	}
};