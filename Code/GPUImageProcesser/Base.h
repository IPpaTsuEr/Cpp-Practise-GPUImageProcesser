#pragma once
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
//#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include"cl/cl.h"

#include"FileProcess.h"
#include<regex>



bool HasPropertie(cl_ulong src, cl_ulong tag) {
	return src & tag ? true : false;
}

struct PathInfo
{
	std::string srcPath;
	std::string codePath;
	std::string hashPath;
	std::string entryName;
	std::string filter;
	bool copySrcPath = true;
};

struct ImageInfoNode {
	int inx, iny;
	float sx, sy;
	int outx, outy;
	int inchannel, outchannel;
};


struct BufferInfo {
	cl_mem input, output, image;
	UCHAR * inputptr;
	UCHAR * outputptr;
	ImageInfoNode * imageptr;
	~BufferInfo() {
		if (input != NULL)clReleaseMemObject(input);
		if (output != NULL)clReleaseMemObject(output);
		if (image != NULL)clReleaseMemObject(image);
		if (inputptr != NULL)free(inputptr);
		if (outputptr != NULL)free(outputptr);
		if (imageptr != NULL)free (imageptr);
	}
};

static std::stringstream ErrorCheck(int error,bool * Hit=NULL) {
	std::stringstream inf;
	if(Hit!=NULL)*Hit = true;
	switch (error) {
	//case     0:inf.append(" Success    "); break;
	case	-1:inf<<" \n\t Error    CL_DEVICE_NOT_FOUND "; break;
	case	-2:inf<<" \n\t Error    CL_DEVICE_NOT_AVAILABLE "; break;
	case	-3:inf<<" \n\t Error    CL_COMPILER_NOT_AVAILABLE "; break;
	case	-4:inf<<" \n\t Error    CL_MEM_OBJECT_ALLOCATION_FAILURE "; break;
	case	-5:inf<<" \n\t Error    CL_OUT_OF_RESOURCES "; break;
	case	-6:inf<<" \n\t Error    CL_OUT_OF_HOST_MEMORY "; break;
	case	-7:inf<<" \n\t Error    CL_PROFILING_INFO_NOT_AVAILABLE "; break;
	case	-8:inf<<" \n\t Error    CL_MEM_COPY_OVERLAP "; break;
	case	-9:inf<<" \n\t Error    CL_IMAGE_FORMAT_MISMATCH "; break;
	case	-10:inf<<" \n\t Error    CL_IMAGE_FORMAT_NOT_SUPPORTED "; break;
	case	-11:inf<<" \n\t Error    CL_BUILD_PROGRAM_FAILURE "; break;
	case	-12:inf<<" \n\t Error    CL_MAP_FAILURE "; break;
	case	-13:inf<<" \n\t Error    CL_MISALIGNED_SUB_BUFFER_OFFSET "; break;
	case	-14:inf<<" \n\t Error    CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST "; break;
	case	-30:inf<<" \n\t Error    CL_INVALID_VALUE "; break;
	case	-31:inf<<" \n\t Error    CL_INVALID_DEVICE_TYPE "; break;
	case	-32:inf<<" \n\t Error    CL_INVALID_PLATFORM "; break;
	case	-33:inf<<" \n\t Error    CL_INVALID_DEVICE "; break;
	case	-34:inf<<" \n\t Error    CL_INVALID_CONTEXT "; break;
	case	-35:inf<<" \n\t Error    CL_INVALID_QUEUE_PROPERTIES "; break;
	case	-36:inf<<" \n\t Error    CL_INVALID_COMMAND_QUEUE "; break;
	case	-37:inf<<" \n\t Error    CL_INVALID_HOST_PTR "; break;
	case	-38:inf<<" \n\t Error    CL_INVALID_MEM_OBJECT "; break;
	case	-39:inf<<" \n\t Error    CL_INVALID_IMAGE_FORMAT_DESCRIPTOR "; break;
	case	-40:inf<<" \n\t Error    CL_INVALID_IMAGE_SIZE "; break;
	case	-41:inf<<" \n\t Error    CL_INVALID_SAMPLER "; break;
	case	-42:inf<<" \n\t Error    CL_INVALID_BINARY "; break;
	case	-43:inf<<" \n\t Error    CL_INVALID_BUILD_OPTIONS "; break;
	case	-44:inf<<" \n\t Error    CL_INVALID_PROGRAM "; break;
	case	-45:inf<<" \n\t Error    CL_INVALID_PROGRAM_EXECUTABLE "; break;
	case	-46:inf<<" \n\t Error    CL_INVALID_KERNEL_NAME "; break;
	case	-47:inf<<" \n\t Error    CL_INVALID_KERNEL_DEFINITION "; break;
	case	-48:inf<<" \n\t Error    CL_INVALID_KERNEL "; break;
	case	-49:inf<<" \n\t Error    CL_INVALID_ARG_INDEX "; break;
	case	-50:inf<<" \n\t Error    CL_INVALID_ARG_VALUE "; break;
	case	-51:inf<<" \n\t Error    CL_INVALID_ARG_SIZE "; break;
	case	-52:inf<<" \n\t Error    CL_INVALID_KERNEL_ARGS "; break;
	case	-53:inf<<" \n\t Error    CL_INVALID_WORK_DIMENSION "; break;
	case	-54:inf<<" \n\t Error    CL_INVALID_WORK_GROUP_SIZE "; break;
	case	-55:inf<<" \n\t Error    CL_INVALID_WORK_ITEM_SIZE "; break;
	case	-56:inf<<" \n\t Error    CL_INVALID_GLOBAL_OFFSET "; break;
	case	-57:inf<<" \n\t Error    CL_INVALID_EVENT_WAIT_LIST "; break;
	case	-58:inf<<" \n\t Error    CL_INVALID_EVENT "; break;
	case	-59:inf<<" \n\t Error    CL_INVALID_OPERATION "; break;
	case	-60:inf<<" \n\t Error    CL_INVALID_GL_OBJECT "; break;
	case	-61:inf<<" \n\t Error    CL_INVALID_BUFFER_SIZE "; break;
	case	-62:inf<<" \n\t Error    CL_INVALID_MIP_LEVEL "; break;
	case	-63:inf<<" \n\t Error    CL_INVALID_GLOBAL_WORK_SIZE "; break;
	case	-64:inf<<" \n\t Error    CL_INVALID_PROPERTY "; break;
	case   -233:inf<<" \n\t Error    Normal File Process Error  "; break;
	case   -234:inf<<" \n\t Waring    Image File Process Error  "; *Hit = false; break;
	case   -235:inf<<" \n\t Waring    Image File Size Error  "; *Hit = false; break;
	default:if(Hit!=NULL) *Hit = false;
	}
	return inf;
}

static void ClearStream(std::stringstream & strs) {
	strs.clear();
	strs.str("");
}

static void Log(std::string & inf,bool see=false) {
	if (see)std::cout << inf;
	FW.WriteLine(&inf.insert(0,getTime()));
}
static void Log(std::stringstream & inf, bool see = false) {
	if (see)std::cout << inf.str();
	FW.WriteLine(&inf.str().insert(0, getTime()));
	ClearStream(inf);
}










struct DeviceBlock {
	cl_device_id * id;
	std::string name;
	std::string version;
	std::string driverVersion;
	std::string openclCVersion;
	std::string profile;
	std::string vendor;
	cl_uint vendorId;
	std::string extensions;
	cl_uint units;
	size_t * itemSize;
	size_t groupSize;
	cl_uint dims;
	cl_ulong  properties;
	cl_ulong  svm;
	bool isOnHost;
	float supportVertion;

	void toString(bool toScreen) {
		std::stringstream ss;
		ss << "\n\t         名称 " << name
			<< "\n\t         版本 " << version
			<< "\n\t         驱动版本 " << driverVersion
			<< "\n\t         OpenCl版本 " << openclCVersion
			<< "\n\t         开发商 " << vendor
			<< "\n\t         开发商ID " << vendorId
			<< "\n\t         Profile " << profile
			<< "\n\t         计算单元 " << units
			<< "\n\t         最大维度 " << dims
			<< "\n\t         工作组最大工作项 " << groupSize
			<< "\n\t         每个维度最大工作项  ={ ";
		for (cl_uint i = 0; i < dims; i++) {
			ss << itemSize[i] << " ";
		}
		ss << "}  \n\t         支持特性 ";
		if (HasPropertie(properties, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE))ss << "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE \t";
		if (HasPropertie(properties, CL_QUEUE_ON_DEVICE_DEFAULT))ss << "CL_QUEUE_ON_DEVICE_DEFAULT \t";
		if (HasPropertie(properties, CL_QUEUE_ON_DEVICE))ss << "CL_QUEUE_ON_DEVICE \t";
		if (HasPropertie(properties, CL_QUEUE_PROFILING_ENABLE))ss << "CL_QUEUE_PROFILING_ENABLE \t";
		ss << "\n\t         SVM特性 ";
		if (svm & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM)ss << "CL_DEVICE_SVM_FINE_GRAIN_SYSTEM \t";
		if (svm & CL_DEVICE_SVM_FINE_GRAIN_BUFFER)ss << "CL_DEVICE_SVM_FINE_GRAIN_BUFFER \t";
		if (svm & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER)ss << "CL_DEVICE_SVM_COARSE_GRAIN_BUFFER \t";
		if (svm & CL_DEVICE_SVM_ATOMICS)ss << "CL_DEVICE_SVM_ATOMICS \t";
		if (svm == 1 << 10)ss << "不支持 \t";
		ss	<< "\n\t         扩展 " << extensions
			<< "\n";
		Log(ss, toScreen);
	};

	~DeviceBlock() {
		if (itemSize != NULL) {
			free(itemSize); itemSize = NULL;
		}
		id = NULL;//id 源自hardware数组，由hardware释放；
	}
};

struct PlatformBlock {
	cl_platform_id * id;
	DeviceBlock * devices;
	cl_uint deviceCount;
	std::string name;
	std::string version;
	std::string profile;
	std::string vendor;
	std::string extensions;
	int selectedDevice;

	void toString(bool toScreen) {
		std::stringstream ss;
		ss << "\n 平台 " << name
			<< "\n 开发商 " << vendor
			<< "\n OpenCL版本 " << version
			<< "\n Profile " << profile
			<< "\n 扩展 " << extensions
			<< "\n";
		Log(ss, toScreen);
		if (toScreen)std::cout << ss.str();
		for (cl_uint i = 0; i < deviceCount; i++) {
			devices[i].toString(toScreen);
		}

	};

	~PlatformBlock() {
		if (devices != NULL)delete[] devices;
		id = NULL;//id 源自hardware数组，由hardware释放；

	}
};










static void repalceAll(char * str, char a, char b) {
	int l = 0;
	while (*(str + l) != '\0') {
		if (*(str + l) == a)*(str + l) = b;
		l++;
	}
}

static std::vector<std::string> split(std::string & str, const char * sp) {
	std::string delim = sp;
	std::regex re{ delim };
	// 调用 std::vector::vector (InputIterator first, InputIterator last,const allocator_type& alloc = allocator_type())
	// 构造函数,完成字符串分割
	return std::vector<std::string> {
		std::sregex_token_iterator(str.begin(), str.end(), re, -1),
			std::sregex_token_iterator()
	};
}

static std::string & trim(std::string & str) {
	str.erase(0, str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ") + 1);
	return str;
}

static bool EndWith(char * src, char * cmp) {
	size_t l = strlen(src);
	size_t c = strlen(cmp);
	for (size_t j = 0; j < c; j++) {
		if (src[l - c + j] != cmp[j])return false;
	}
	return true;
}
static bool EndWith(char * src, const char * cmp) {
	size_t l = strlen(src);
	size_t c = strlen(cmp);
	for (size_t j = 0; j < c; j++) {
		if (src[l - c + j] != cmp[j])return false;
	}
	return true;
}

class Performance {

public:
	static void get(LARGE_INTEGER * count) {
		QueryPerformanceCounter(count);
	}
	static float getTakeTime(LARGE_INTEGER & start,bool UseSecond=true) {
		LARGE_INTEGER end,fq;
		QueryPerformanceCounter(&end);
		QueryPerformanceFrequency(&fq);
		if(UseSecond) return (float)(end.QuadPart - start.QuadPart) / (float)fq.QuadPart;
		else return 1000.0f*(float)(end.QuadPart - start.QuadPart) / (float)fq.QuadPart;
	}
	
};