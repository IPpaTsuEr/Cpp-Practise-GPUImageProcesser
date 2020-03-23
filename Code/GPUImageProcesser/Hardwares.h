#pragma once
#include<stdlib.h>
#include<vector>
#include"Base.h"


class Hardwares {
// 	cl_platform_id *AllPlatform;
// 	cl_device_id ** AllDevice;

	cl_uint Pcount;
	cl_uint Dcount;

	std::vector<PlatformBlock *> Plist= std::vector<PlatformBlock *>();

	void getPlatforms() {
		cl_uint pcount,ccount;
		clGetPlatformIDs(0, NULL, &pcount);
		cl_platform_id * platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id)*pcount);
		clGetPlatformIDs(pcount, platforms, NULL);
		
		char * t;
		PlatformBlock * info;

		for (cl_uint i = 0; i < pcount; i++) {
			Plist.push_back(new PlatformBlock());
			info =Plist[i];
			info->id = &platforms[i];
			t=getInfo<char>(*info->id, CL_PLATFORM_NAME);
			info->name = t;
			free(t);
			t= getInfo<char>(*info->id, CL_PLATFORM_PROFILE);
			info->profile = t;
			free(t);
			t=getInfo<char>(*info->id, CL_PLATFORM_VENDOR);
			info->vendor = t;
			free(t);
			t=getInfo<char>(*info->id, CL_PLATFORM_VERSION);
			info->version = t;
			free(t);
			t=getInfo<char>(*info->id, CL_PLATFORM_EXTENSIONS);
			info->extensions = t;
			free(t);
			clGetDeviceIDs(*info->id, CL_DEVICE_TYPE_ALL, 0, NULL, &ccount);
			info->deviceCount = ccount;

			info->devices = new DeviceBlock[ccount]();
				
			getDivices(*info->id,ccount,info->devices);
			
			info = NULL;
		}
		//std::cout << Plist[0].name << std::endl;
	}
	void getDivices(cl_platform_id id,cl_uint ccount,DeviceBlock * db) {
		

		cl_device_id * devices = (cl_device_id *)malloc(sizeof(cl_device_id)*ccount);

		clGetDeviceIDs(id, CL_DEVICE_TYPE_ALL, ccount, devices, NULL);

		char * t; cl_uint * i;
		for (cl_uint j = 0; j < ccount; j++) {

			db[j].id = &devices[j];
			t=getInfo<char>(devices[j], CL_DEVICE_NAME);
			db[j].name=(t);
			free(t);
			t = getInfo<char>(devices[j], CL_DEVICE_VENDOR);
			db[j].vendor = t;
			free(t);
			t = getInfo<char>(devices[j], CL_DEVICE_VERSION);
			db[j].version = t;
			free(t);
			t = getInfo<char>(devices[j], CL_DEVICE_PROFILE);
			db[j].profile = t;
			free(t);
			t = getInfo<char>(devices[j], CL_DRIVER_VERSION);
			db[j].driverVersion = t;
			free(t);
			t = getInfo<char>(devices[j], CL_DEVICE_OPENCL_C_VERSION);
			db[j].openclCVersion = t;
			free(t);
			t = getInfo<char>(devices[j], CL_DEVICE_EXTENSIONS);
			db[j].extensions = t;
			free(t);
			i = getInfo<cl_uint>(devices[j], CL_DEVICE_VENDOR_ID);
			db[j].vendorId = *i;
			free(i);
			i = getInfo<cl_uint>(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS);
			db[j].units = *i;
			free(i);
			size_t *s = getInfo<size_t>(devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE);
			db[j].groupSize = *s;
			free(s);
			i = getInfo<cl_uint>(devices[j], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
			db[j].dims = *i;
			free(i);
			s = getInfo<size_t>(devices[j],CL_DEVICE_MAX_WORK_ITEM_SIZES);
			db[j].itemSize = (size_t *)malloc(sizeof(size_t)*db[j].dims);
			memcpy(db[j].itemSize, s, sizeof(size_t)*db[j].dims);
			free(s);

			cl_command_queue_properties * pr =getInfo<cl_command_queue_properties>(devices[j], CL_DEVICE_QUEUE_PROPERTIES);
			db[j].properties = *pr;
			free(pr);

			cl_device_svm_capabilities * cp = getInfo<cl_device_svm_capabilities>(devices[j], CL_DEVICE_SVM_CAPABILITIES);
			if (cp != NULL) {
				db[j].svm = *cp;
				free(cp);
			}
			else db[j].svm =1<<10;

			db[j].supportVertion = (float)atof(db[j].openclCVersion.substr(db[j].openclCVersion.rfind("OpenCL C ") + 9).c_str());
			if (db[j].name.find("Intel(R) HD Graphics") != db[j].name.npos || db[j].name.find("CPU") != db[j].name.npos ) db[j].isOnHost = true;
			else db[j].isOnHost = false;
		}
		//free(devices);

	}

public:

	~Hardwares() {
		for (PlatformBlock* e : Plist) {
			delete e;
		}
	}

	void toString(bool toScreen) {
		for (PlatformBlock* e : Plist) {
			e->toString(toScreen);
		}
	}
	void Init() {
		getPlatforms();
	}

	template<typename T>
	T * getInfo(cl_device_id d, unsigned int InfoType) {
		size_t c;
		cl_int error =clGetDeviceInfo(d, InfoType, 0, NULL, &c);
		if (error != CL_SUCCESS)return NULL;
		T * result = (T*)malloc(sizeof(T)*c);
		error=clGetDeviceInfo(d, InfoType, c, result, NULL);
		if (error != CL_SUCCESS) {
			free(result);
			return NULL;
		}
		return result;
	}
	template<typename T>
	T * getInfo(cl_platform_id d, unsigned int InfoType) {
		size_t c;
		clGetPlatformInfo(d, InfoType, 0, NULL, &c);
		T * result = (T*)malloc(sizeof(T)*c);
		clGetPlatformInfo(d, InfoType, c, result, NULL);
		return result;
	}


	bool select(std::vector<PlatformBlock*> &selectList) {
SELECT:		std::cout << "\n\t目标平台及设备信息：" << std::endl;
		std::string str;
		for (int e = 0; e < Plist.size();e++) {
			std::cout << "  【" << e <<"】"<< Plist[e]->name <<"    -    "<<Plist[e]->version<< std::endl;
			DeviceBlock * t = Plist[e]->devices;
					
					for (unsigned int s = 0; s < Plist[e]->deviceCount; s++) {
			
						std::cout << "\t└── [" << s << "]" << trim(t[s].name) << "   -    " << t[s].version << std::endl;
					}
					std::cout << std::endl;
		}
		std::cout << "  【Q】  退出 \n"<< std::endl;
		std::cout << "请选择目标平台及硬件（例如1-0，选定多个平台使用空格隔开）：" << std::endl;
		std::getline(std::cin,str,'\n');
		if (str.find("Q") != str.npos || str.find("q") != str.npos) {
			str.clear();
			str.append(" 用户主动退出。");
			Log(str);
			return false;
		}

		
		std::vector<std::string> spl=split(str, " ");
		
		for (int y = 0; y < spl.size(); y++) {
			std::vector<std::string> sspl = split(spl[y], "-");
			for (int x = 0; x < sspl.size(); x+=2) {
				int pindex = stoi(sspl[x]);
				int sindex = stoi(sspl[x + 1]);
				if (pindex > Plist.size()) {
					std::cout<< "只有" << Plist.size() << "个平台可选\n但输入的序号超过了可索引范围，序号应从0开始。";
				
					goto SELECT;
				}
				if (sindex >= (int)Plist[pindex]->deviceCount) {
					std::cout << "平台 " << Plist[pindex]->name << " 只有" << Plist[pindex]->deviceCount << "个可选设备\n"<< " 但输入的序号超过了可索引范围，序号应从0开始。";
				
					goto SELECT;
				}
				Plist[pindex]->selectedDevice = sindex;
				selectList.push_back(Plist[pindex]);
				
			}
		}
		std::cout << "\n以下设备已被选择：\n\t";
		for (int z = 0; z < selectList.size(); z++) {
			std::cout <<"\t" <<trim(selectList[z]->name) << "── " << selectList[z]->devices[selectList[z]->selectedDevice].name <<"  Ver." << selectList[z]->devices[selectList[z]->selectedDevice].supportVertion<< std::endl;
			
		}
		return true;
	}

/*
	void getAllPlatformAndDevice() {

		clGetPlatformIDs(0, NULL, &Pcount);
		AllPlatform = (cl_platform_id*)malloc(sizeof(cl_platform_id)*Pcount);
		clGetPlatformIDs(Pcount, AllPlatform, NULL);

		for (cl_uint i = 0; i < Pcount; i++) {
			cl_uint temp = 0;
			clGetDeviceIDs(AllPlatform[i], CL_DEVICE_TYPE_ALL, 0, NULL, &temp);
			if (Dcount <= temp)Dcount = temp;
		}
		AllDevice = (cl_device_id **)malloc(sizeof(cl_device_id)*Pcount);
		for (cl_uint z = 0; z < Pcount; z++) {
			AllDevice[z] = (cl_device_id*)malloc(sizeof(cl_device_id)*Dcount);
			for (cl_uint v = 0; v < Dcount; v++)AllDevice[z][v] = NULL;
		}


		for (cl_uint i = 0; i < Pcount; i++) {
			clGetDeviceIDs(AllPlatform[i], CL_DEVICE_TYPE_ALL, 0, NULL, &Dcount);
			clGetDeviceIDs(AllPlatform[i], CL_DEVICE_TYPE_ALL, Dcount, AllDevice[i], NULL);
		}
	}

	void AllInfoToString() {
		for (cl_uint x = 0; x < Pcount; x++) {
			printOutInfo(AllPlatform[x]);
			for (cl_uint y = 0; y < Dcount; y++) {
				if (AllDevice[x][y] != NULL)
				{
					printOutInfo(AllDevice[x][y]);
				}
			}
		}
	}




	info::PlatformInfo getTargetPlatformInfo(cl_platform_id p) {
		info::PlatformInfo *i = new info::PlatformInfo();
		i->name = getInfo<char>(p, CL_PLATFORM_NAME);
		i->profile = getInfo<char>(p, CL_PLATFORM_PROFILE);
		i->vendor = getInfo<char>(p, CL_PLATFORM_VENDOR);
		i->version = getInfo<char>(p, CL_PLATFORM_VERSION);
		i->ext = getInfo<char>(p, CL_PLATFORM_EXTENSIONS);

		return *i;
	}

	info::DeviceInfo getTargetDeviceInfo(cl_device_id d) {
		info::DeviceInfo  *i = new info::DeviceInfo();
		i->units = getInfo<cl_uint>(d, CL_DEVICE_MAX_COMPUTE_UNITS);
		i->frequency = getInfo<cl_uint>(d, CL_DEVICE_MAX_CLOCK_FREQUENCY);
		i->cachesize = getInfo<cl_uint>(d, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE);
		i->dimensions = getInfo<cl_uint>(d, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
		i->workgroup = getInfo<size_t>(d, CL_DEVICE_MAX_WORK_GROUP_SIZE);
		i->name = getInfo<char>(d, CL_DEVICE_NAME);
		return *i;
	}
	void printOutInfo(cl_platform_id P) {
		auto e = getTargetPlatformInfo(P);
		std::wcout.imbue(std::locale("chs"));
		std::wcout << std::endl << L"平台:" << e.name << std::endl;
		std::wcout << L"    厂商:" << e.vendor << std::endl;
		std::wcout << L"    特性:" << e.profile << std::endl;
		std::wcout << L"    版本:" << e.version << std::endl;
		std::wcout << L"    扩展:" << e.ext << std::endl;
	}
	void printOutInfo(cl_device_id D) {
		info::DeviceInfo  dinfo = getTargetDeviceInfo(D);
		std::wcout << std::endl;
		std::wcout.imbue(std::locale("chs"));
		std::wcout << L"    设备:" << dinfo.name << std::endl;
		std::cout << "            Units:" << *dinfo.units << std::endl;
		std::cout << "            CacheSize:" << *dinfo.cachesize << std::endl;
		std::cout << "            Frequency:" << *dinfo.frequency << std::endl;
		std::cout << "            WorkGroup:" << *dinfo.workgroup << std::endl;
		std::cout << "            Dimension:" << *dinfo.dimensions << std::endl;
	}
	*/
};