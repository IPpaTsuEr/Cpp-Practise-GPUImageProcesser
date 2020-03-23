#pragma once
#include"Contexts.h"
#include"Queues.h"
#include"Programs.h"
#include"Kernels.h"

#include"Buffers.h"
#include"Hardwares.h"
#include"ImageLoader.h"

#include "FileProcess.h"


struct Cache {
	BYTE * Host_data;
	ImageInfoNode * info;
	std::string savePath;
	cl_mem D_data;
};
struct Saver {
	BYTE * Host_data;
	ImageInfoNode * info;
	std::string savePath;
	cl_mem D_data;

};


class ImageProcess : CInfoCallback{
	cl_kernel  kernel;
	cl_context context;
	cl_program  program;
	cl_command_queue  queue;

	cl_context_properties * contextProperties;
	cl_queue_properties * queueProperties;
	std::string dname;
	size_t * offset, *globalWorksize, *localWorkSize;


	bool isOnHost;
	bool abort = false,loaderFinish=false,saverFinish=false;

	SafeList<std::string> *list;
	SafeList<Cache> InBuffer;
	SafeList<Saver> OutBuffer;

	cl_event mapevent, copyevent;
	size_t finisheditems,erroritems;

	size_t MAX_SIZE = sizeof(UCHAR) * 4096 * 4096 * 4;//8k 4c * 4
	cl_uint hashcodesize = 4096;
	
	ImageLoader il = ImageLoader(this, MAX_SIZE);
	Contexts cs = Contexts(this);
	Kernels ks = Kernels(this);
	Programs ps = Programs(this);
	Buffers bs = Buffers(this);
	Queues qs = Queues(this);

	PathInfo path;
	LARGE_INTEGER substart, totalstart;
	std::stringstream logstream;
	
	
	//ImageInfoNode * ImageInfo;
	BufferInfo buffers;

	bool IOBuffer = false;
	bool BufferOnDevice_In=false, BufferOnDevice_Out=false;
	size_t globalWorkSize[2] = { 0,0 };
	int outchannel;
	int MaxIBQS, MaxOBQS;

public:


	~ImageProcess() {

		clReleaseProgram(program);
		clReleaseContext(context);
		clReleaseKernel(kernel);
		clReleaseCommandQueue(queue);

	
	}
	std::string getName() { return dname; }
	size_t getInBufferCount() { return InBuffer.size(); }
	size_t getOutBufferCount() { return OutBuffer.size(); }
	size_t getFinishedCount() {return finisheditems;}
	size_t getErrorCount() {return erroritems;}
	bool getStatus() {
		if (abort)return saverFinish;
		else return abort;
	}
	void stop() {
		abort = true;
	}
	void start() {
		std::thread task(&ImageProcess::run,this);
		task.detach();
	}
	void Prepare(SafeList<std::string> *sourceList, cl_platform_id pid, DeviceBlock * did, PathInfo setPath, int outchannel, cl_uint outputsize) {
		
		path = setPath;
		isOnHost = did->isOnHost;
		this->outchannel = outchannel;
		
		hashcodesize = outputsize;
		dname = trim(did->name);
		list = sourceList;

		//ImageInfo=bs.getAlign<ImageInfoNode>(1);
		//ImageInfo->outchannel = outchannel;

		context = cs.create(&pid);
		program =ps.createProgram(context, 1, 1,(char *) path.codePath.c_str());
		ps.buildProgram(program, *did->id, 1);
		kernel =ks.create(program, (char *)path.entryName.c_str());
		queue = qs.create(context, *did->id, did->supportVertion,&did->properties);

		if (isOnHost) {
			onHostBuffer(&buffers);
		}
		else {
			onDeviceBuffer(&buffers);
		}

	}
	void Enable(bool IOQueueEnable =true,bool BufferOnDevice_In =true,int MaxIBQS=200, int MaxOBQS=2000, bool BufferOnDevice_Out=false ) {
		IOBuffer = IOQueueEnable;
		if (IOQueueEnable) {
		BufferOnDevice_In = BufferOnDevice_In;
		BufferOnDevice_Out = BufferOnDevice_Out;
		this->MaxIBQS=MaxIBQS;
		this->MaxOBQS=MaxOBQS;
		}
		
	}

	void normalizeWorksize(size_t G[], size_t L[]) {
		if (G[0] == 0)G[0] = 2;
		if (G[1] == 0)G[1] = 2;
		if (G[0] % 2 != 0)G[0]++;
		if (G[1] % 2 != 0)G[1]++;

		if (L != NULL) {

		}
	}

	void onHostBuffer(BufferInfo * info) {
		info->inputptr = Buffers::getAlign<UCHAR>(MAX_SIZE);
		info->input = bs.createBufferAndSet(0, kernel, context, MAX_SIZE, info->inputptr, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR);
		//clRetainMemObject(info->input);
		info->imageptr = Buffers::getAlign<ImageInfoNode>(1);
		info->image = bs.createBufferAndSet(1, kernel, context, sizeof(ImageInfoNode), info->imageptr, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR);
		//clRetainMemObject(info->image);
		info->outputptr = Buffers::getAlign<UCHAR>(sizeof(BYTE)*hashcodesize * 4);
		info->output = bs.createBufferAndSet(2, kernel, context, sizeof(BYTE)*hashcodesize * 4, info->outputptr, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR);
		//clRetainMemObject(info->output);
	}
	void onDeviceBuffer(BufferInfo * info) {
		info->input = bs.createBufferAndSet(0, kernel, context, MAX_SIZE, NULL, CL_MEM_READ_WRITE);
		//clRetainMemObject(info->input);
		info->imageptr = Buffers::getAlign<ImageInfoNode>(1);
		info->image = bs.createBufferAndSet(1, kernel, context, sizeof(ImageInfoNode), NULL, CL_MEM_READ_WRITE);
		//clRetainMemObject(info->image);
		info->output = bs.createBufferAndSet(2, kernel, context, sizeof(BYTE)*hashcodesize * 4, NULL, CL_MEM_WRITE_ONLY);
		//clRetainMemObject(info->output);
		info->outputptr = (BYTE*)malloc(sizeof(BYTE)*hashcodesize * 4);
	}
 

	void BufferLoader(ImageLoader  * il) {
		std::string filepath;

		while (true) {
				try {
					if (list->size() > 0) {
					
						filepath = list->getAndPop_front();
						
						BYTE * imageData = il->load(filepath.c_str());//注意数据使用完毕后释放
						
						if (imageData == NULL) {
							erroritems++;
							continue;
						}
				
						Cache newcache;
						newcache.info =  new ImageInfoNode();

						il->getImageInfo(newcache.info);
						il->getAdjustSize(newcache.info, hashcodesize);
						newcache.info->outchannel = outchannel;
						newcache.savePath = filepath;
						
						if (BufferOnDevice_In && !isOnHost) {
								newcache.D_data=bs.createBuffer(context, il->getWidth()*il->getHeight()*il->getChannel(),imageData,CL_MEM_READ_WRITE| CL_MEM_COPY_HOST_PTR);
						}
						else {//本地设备 或是 不启用设备缓存
							newcache.Host_data = (BYTE *)malloc(il->getWidth()*il->getHeight()*il->getChannel());
							memcpy(newcache.Host_data, imageData, il->getWidth()*il->getHeight()*il->getChannel());
						}
						

						InBuffer.pushToBack(newcache);

						il->clear();

					} else {
						logstream << "\n 载入缓冲队列载入完毕，线程已退出。 from："<<dname<<"\n";
						Log(logstream);
						loaderFinish = true;
						break;
					}
				}
				catch (int err) {
					if (err > -234) {
						logstream<< dname << " 读缓存遇到严重错误，已停止继续运行。错误代码：" << err << "\n";
						Log(logstream, true);
						loaderFinish = true;
						break;
					}
					else {
						//logstream <<"\n"<<dname<< " IO Buffer Queue Loader Error :" << err << std::endl;
						//Log(logstream);
						erroritems++;
					}
				}
		}
	}
	void BufferSaver() {
		std::string save;
		size_t RNCount = 0;
		while (true ) {
			try {
				if (OutBuffer.size() > 0) {
				
					while (OutBuffer.size() < MaxOBQS) {
						if(abort)break;
						else Sleep(500);
						
					}
					RNCount = OutBuffer.size();
					while (RNCount > 0) {
						RNCount--;                            //直到连续处理完 MaxOBQS 个任务后才开始下一次缓冲填充等待
						
					
						Saver s = OutBuffer.getAndPop_front();
						FIBITMAP * map;

						if (BufferOnDevice_Out && !isOnHost) {
							BYTE * sdata = (BYTE *)malloc(sizeof(BYTE)* s.info->outx * s.info->outy * outchannel);
							bs.getBuffer(queue, s.D_data, sizeof(BYTE)* s.info->outx * s.info->outy * outchannel, sdata);
							map = il.create(s.info->outx, s.info->outy, sdata, outchannel);
							free(sdata);
							clReleaseMemObject(s.D_data);
							clReleaseMemObject(s.D_data);
						}
						else {
							map = il.create(s.info->outx, s.info->outy, s.Host_data, outchannel);
							free(s.Host_data);
						}

						save.clear();
						save = path.hashPath;

						if (path.copySrcPath) {
							std::string rp = s.savePath.replace(s.savePath.find(path.srcPath), path.srcPath.length() + 1, "");
							save.append(rp);
							FileProcess::createFolders(save, save);
							if (outchannel >= 3)il.saveToJPG(map, save.c_str());
							else il.saveToGray(map, save.c_str());
						}
						else {
							if (outchannel >= 3)il.saveToJPG(map, save.append(FileProcess::getFileNameFromPath(s.savePath)).c_str());
							else il.saveToGray(map, save.append(FileProcess::getFileNameFromPath(s.savePath)).c_str());
						}

						delete s.info;
					
					
					
					}
					
				
				}
				else {
					if (abort) {
						logstream << "\n 输出缓冲队列载入完毕，线程已退出。 from：" << dname << "\n";
						Log(logstream);
						saverFinish = true;
						break;
					}
					
				}
			}
			catch (int err) {
				if (err > -234) {
					logstream << dname << "写缓存遇到严重错误，已停止继续运行。错误代码：" << err << "\n";
					Log(logstream, true);
					saverFinish = true;
					break;
				}
				else {
					//logstream <<"\n "<< dname << "  IO Buffer Queue Save Error :" << err << std::endl;
					//Log(logstream);
					erroritems++;
				}
				
			}

		}
	}

	void DR(BYTE* imageData, ImageInfoNode inputinfo,cl_mem * i =NULL) {

		bs.setBuffer(queue, buffers.image, sizeof(ImageInfoNode), &inputinfo);

		if (BufferOnDevice_In && !isOnHost) {
			bs.copy(queue,*i, buffers.input, sizeof(BYTE)* inputinfo.inx * inputinfo.iny * inputinfo.inchannel);
		}
		else {
			if(isOnHost)memcpy(buffers.inputptr, imageData, sizeof(BYTE)* inputinfo.inx * inputinfo.iny * inputinfo.inchannel);
			else bs.setBuffer(queue, buffers.input, sizeof(BYTE)* inputinfo.inx * inputinfo.iny * inputinfo.inchannel, imageData);
		}


		globalWorkSize[0] = inputinfo.outx;
		globalWorkSize[1] = inputinfo.outy;

		normalizeWorksize(globalWorkSize, NULL);
		qs.run(queue, kernel, globalWorkSize, NULL, 2);

		if (!isOnHost)
			bs.getBuffer(queue, buffers.output, sizeof(BYTE)* inputinfo.outx * inputinfo.outy * outchannel, buffers.outputptr);
		
	}

			
	void run() {
			ImageLoader lo1 = ImageLoader(this, MAX_SIZE);
			//ImageLoader lo2 = ImageLoader(this, MAX_SIZE);

		if (IOBuffer) {
			Performance::get(&totalstart);
			
			std::thread loader(&ImageProcess::BufferLoader,this,&lo1);
			//std::thread loader2(&ImageProcess::BufferLoader, this,&lo2);
			std::thread saver(&ImageProcess::BufferSaver,this);
			
			loader.detach();
			Sleep(100);
			//loader2.detach();
			saver.detach();

			
			while ( InBuffer.size() < list->size() ) { 
				if (InBuffer.size() >= MaxIBQS)break;
				Sleep(200);
			}
			
		}

		std::string filepath;

		int RNcount = 0;

		while (!abort) {
			/************************************************************************/
			/* 依照队列或前置进程状态 决定是否结束主线程以结束运行                  */
			/************************************************************************/
			if (IOBuffer) {
				if ( loaderFinish== true && saverFinish == true) {
					break;
				}
			}
			else {
				if (list->size() <= 0)break;
			}



			try {
				if (IOBuffer) { //启用了缓冲队列的 读缓冲-处理-写缓冲 处理方式
					
					while (InBuffer.size() < MaxIBQS) {     //等待缓冲写满
							if (loaderFinish)break;         //缓冲未满，但数据已全部载入
							else Sleep(200); 
					}
					
					while (InBuffer.size() > 0 ) {

						RNcount++;                            //直到连续处理完 MaxIBQS 个任务后才开始下一次缓冲填充等待
						if (!loaderFinish) {
							if (RNcount >= MaxIBQS) {
								RNcount = 0;
								break;
							}
						}

						Cache ca=InBuffer.getAndPop_front();

						if (BufferOnDevice_In && !isOnHost) {
							DR(ca.Host_data, *ca.info, &ca.D_data);
							clReleaseMemObject(ca.D_data);
							clReleaseMemObject(ca.D_data);
							}
						else {
							DR(ca.Host_data, *ca.info);
							free(ca.Host_data);
						}
						
						Saver sv;
						if (BufferOnDevice_Out && !isOnHost) {
							sv.D_data = bs.createBuffer(context, sizeof(BYTE)* ca.info->outx * ca.info->outy * outchannel,NULL,CL_MEM_READ_WRITE);
							bs.copy(queue, buffers.output, sv.D_data, sizeof(BYTE)* ca.info->outx * ca.info->outy * outchannel);
						}
						else {
							sv.Host_data = (BYTE *)malloc(sizeof(BYTE)* ca.info->outx * ca.info->outy * outchannel);
							memcpy(sv.Host_data, buffers.outputptr, sizeof(BYTE)* ca.info->outx * ca.info->outy * outchannel);
						}
						
						sv.savePath = ca.savePath;
						sv.info = ca.info;
						OutBuffer.pushToBack(sv);


						finisheditems++;
					}
					//有可能存在 主线程在连续处理时 输入缓冲载入完毕并退出，此时缓冲队列中还用数据，应再次检测队列是否为空才能退出
					if (InBuffer.size()<=0 && loaderFinish)break;      //输入缓冲为空，且输入缓冲载入线程已经结束
					
				}
				else { //无缓冲的 载入-处理-输出 处理方式
					
					filepath.clear();
					filepath = list->getAndPop_front();

					BYTE * imageData = il.load(filepath.c_str());//注意数据使用完毕后释放
					
					il.getImageInfo(buffers.imageptr);
					il.getAdjustSize(buffers.imageptr, hashcodesize);

					DR(imageData,*buffers.imageptr);
					saveResult(filepath);
					finisheditems++;
				}
				
			}
			catch (int err) {
				if (err > -233) {
					logstream<< dname << "  核心遇到严重错误，已停止继续运行。错误代码：" << err <<std::endl;
					Log(logstream,true);
					break;
				}
				else {
					//erroritems++;
					logstream << dname << " Run Error :" << err << std::endl;
					Log(logstream);
				}
			
			}
		}
		float td = Performance::getTakeTime(totalstart);
		abort = true;
		logstream <<"\n\t"<< dname << "\t 上的 "<< finisheditems + erroritems <<"\t 个任务已完成,有 " << erroritems << "\t 个任务出错 共耗时 ：\t" << td << "秒\n";
		Log(logstream);
		if (!IOBuffer) { loaderFinish = true; saverFinish = true; }
	}

	void saveResult(std::string & filepath) {
		std::string save = path.hashPath;
		FIBITMAP * map = il.create(buffers.imageptr->outx,	buffers.imageptr->outy, buffers.outputptr, outchannel);
		if (path.copySrcPath) {
			save.append(filepath.replace(filepath.find(path.srcPath), path.srcPath.length() + 1, ""));
			FileProcess::createFolders(save, save);
			if (outchannel >= 3)il.saveToJPG(map, save.c_str());
			else il.saveToGray(map, save.c_str());
		}
		else {
			if (outchannel >= 3)il.saveToJPG(map, save.append(FileProcess::getFileNameFromPath(filepath)).c_str());
			else il.saveToGray(map, save.append(FileProcess::getFileNameFromPath(filepath)).c_str());
		}
		il.clear();
		
	}





	/************************************************************************/
	/* ERROR             HANDLE                                             */
	/************************************************************************/

	virtual void OnProgramError(int code, std::stringstream e) override
	{
		bool isHit = false;
		std::stringstream i = ErrorCheck(code, &isHit);
		i << e.str() << " On " << dname << "\n";
		if (isHit) {
			Log(i);
			throw(code);
		}
		else {
			//Log(i);
		}
		
	}

	virtual void OnKernelError(int code, std::stringstream e) override
	{
		bool isHit = false;
		std::stringstream i = ErrorCheck(code, &isHit);
		i << e.str() << " On " << dname << "\n";
		if (isHit) {
			Log(i);
			throw(code);
		}
		else {
			//Log(i);
		}
	}


	virtual void OnBufferError(int code, std::stringstream e) override
	{
		bool isHit = false;
		std::stringstream i = ErrorCheck(code, &isHit);
		i << e.str() << " On " << dname << "\n";
		if (isHit) {
			Log(i,true);
			throw(code);
		}
		else {
			//Log(i);
		}
	}


	virtual void OnQueueError(int code, std::stringstream e) override
	{
		bool isHit = false;
		std::stringstream i = ErrorCheck(code, &isHit);
		i << e.str() << " On " << dname << "\n";
		if (isHit) {
			Log(i);
			throw(code);
		}
		else {
			//Log(i);
		}
	}


	virtual void OnDataError(int code, std::stringstream e) override
	{
		bool isHit = false;
		std::stringstream i = ErrorCheck(code, &isHit);
		i << e.str() << " On " << dname << "\n";
		Log(i, true);
		throw(code);
		
	}


	virtual void OnContextError(int code, std::stringstream e) override
	{
		bool isHit = false;
		std::stringstream i = ErrorCheck(code, &isHit);
		i << e.str() << " On " << dname << "\n";
		if (isHit) {
			Log(i);
			throw(code);
		}
		else {
			//Log(i);
		}
	}


	virtual void OnFolderScaned() override
	{
		
	}


	virtual void OnFileScaned(std::string path) override
	{
		
	}

};