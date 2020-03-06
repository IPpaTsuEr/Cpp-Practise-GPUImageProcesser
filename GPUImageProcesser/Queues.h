#pragma once
#include"Base.h"
class Queues {
	CInfoCallback * callback;
public:
	Queues(CInfoCallback *Muster):callback(Muster) {};
	
	//
	cl_command_queue create(cl_context context, cl_device_id divece, float version,cl_queue_properties * support_properties=NULL) {
		cl_int error = 0;
		cl_command_queue queue = NULL;
		if (version >= 2.0f) {
			cl_queue_properties properties[3]={// | CL_QUEUE_PROFILING_ENABLE
				CL_QUEUE_PROPERTIES,
				CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE| CL_QUEUE_ON_DEVICE_DEFAULT |CL_QUEUE_ON_DEVICE,
				0
			};
			if (support_properties != NULL)properties[1] = *support_properties;
			properties[1] ^= CL_QUEUE_PROFILING_ENABLE;
			queue = clCreateCommandQueueWithProperties(context, divece, properties, &error);
		}
		else if (version < 2.0f) {
			cl_queue_properties properties = 0;//CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;// CL_QUEUE_PROFILING_ENABLE;
			if (support_properties != NULL) properties=*support_properties;
			properties ^= CL_QUEUE_PROFILING_ENABLE;
			queue = clCreateCommandQueue(context, divece, properties, &error);
		}
		callback->OnQueueError(error, std::stringstream()<<"Create Queue With Version:"<< version);
		return queue;
	}


	void run(cl_command_queue queue,cl_kernel kernel,size_t globalWorkSize[],size_t localWorkSize[],cl_uint Dims=1,size_t offset[]=NULL) {
		cl_event endevent;
		cl_int error;
		error = clEnqueueNDRangeKernel(queue, kernel, Dims,offset, globalWorkSize, localWorkSize, 0, NULL, &endevent);
		if (error == CL_SUCCESS) {
			error = clWaitForEvents(1, &endevent);
			if (error == CL_SUCCESS)error = clReleaseEvent(endevent);
		}
		callback->OnQueueError(error, std::stringstream()<<"Run Queue");
	}


	template<typename T>
	static T * getInfo(cl_command_queue commandqueue,int name) {
		size_t size;
		cl_int error =0 ;
		error =clGetCommandQueueInfo(commandqueue, name, NULL, NULL, &size);
		T * result = (T*)malloc(sizeof(T)*size);
		error = clGetCommandQueueInfo(commandqueue, name, size,result, NULL);
#ifdef DEBUG
		Log(error, "Get Queue Info :");
#endif // 

		return result;
	}
};