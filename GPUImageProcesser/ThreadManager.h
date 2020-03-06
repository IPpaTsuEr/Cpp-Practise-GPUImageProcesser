#pragma once
#include "ImageProcess.h"
#include<iomanip>

class ThreadManager {
	Hardwares hw;
	FileProcess fp;

	std::vector<PlatformBlock*> selectList;
	std::vector<ImageProcess*> ruuntarget;
	
	PathInfo path;
	
	size_t totalcount,lastcount;
	LARGE_INTEGER starttime;
	float lasttime;

	size_t MaxRate=1, MinRate=100, AverageRate,CurrentRate;
	bool CanExite;

	bool IOBufferEnabel, InEnabel, OutEnabel;
	int resultChannel =1 ;
	unsigned int resultSize = 4096;
	bool copyDirStruct = true;

	int MaxIBQS, MaxOBQS;

public:

	enum PATH_TYPE {
		IMAGE_SOURCE,
		CODE_SOURCE,
		HASH_CODE,
		ENTRY_NAME,
		FILE_FILTER
	};

	bool start() {
		hw.Init();
		LOG(hw.toString(false));
		return hw.select(selectList);
	}
	void setPath(char * source, PATH_TYPE type) {
		switch (type)
		{
		case PATH_TYPE::IMAGE_SOURCE:
			path.srcPath = source;
			if (endsWith(path.srcPath, "\\"))path.srcPath.erase(path.srcPath.length()-1);
			break;
		case PATH_TYPE::CODE_SOURCE:
			path.codePath = source;
			break;
		case PATH_TYPE::HASH_CODE:
			path.hashPath = source;
			if (!endsWith(path.hashPath, "\\"))path.hashPath.append("\\");
			FileProcess::createFolders(path.hashPath, path.hashPath);
			break;
		case PATH_TYPE::ENTRY_NAME:
			path.entryName = source;
			break;
		case PATH_TYPE::FILE_FILTER:
			path.filter = source;
			break;
		default:
			break;
		}
		
	}
	void setPath(std::string  source, PATH_TYPE type) {
		setPath((char*)source.c_str(), type);
	}
	size_t getSelectCount() {
		return selectList.size();
	}
	void LoadSetting(char* settingpath =(char *)".\\Inif.inf") {
		FileReader fr = *new FileReader();
		if (fr.Open(settingpath)) {
			std::string line;
			std::stringstream convert;
			while (fr.ReadLine(&line)) {
				convert.str("");
				convert.clear();
				if(line.empty())continue;
				if (startsWith(line, "Source="))
				{
					setPath(line.substr(line.find("Source=") + 7), IMAGE_SOURCE);
					continue;
				}
				if (startsWith(line, "Filter="))
				{
					setPath(line.substr(line.find("Filter=") + 7), FILE_FILTER);
					continue;
				}
				if (startsWith(line, "Program="))
				{
					setPath(line.substr(line.find("Program=") + 8), CODE_SOURCE);
					continue;
				}
				if (startsWith(line, "Output="))
				{
					setPath(line.substr(line.find("Output=") + 7), HASH_CODE);
					continue;
				}
				if (startsWith(line, "Entry="))
				{
					setPath(line.substr(line.find("Entry=") + 6), ENTRY_NAME);
					continue;
				}
				if (startsWith(line, "IOQueue=")) {
					convert << line.substr(line.find("IOQueue=") + 8);
					convert >> std::boolalpha >> IOBufferEnabel;
					continue;
				}
				if (startsWith(line, "DeviceInBuffer=")) {
					convert << line.substr(line.find("DeviceInBuffer=") + 15);
					convert >> std::boolalpha >> InEnabel;
					continue;
				}
				if (startsWith(line, "DeviceOutBuffer=")) {
					convert << line.substr(line.find("DeviceOutBuffer=") + 16);
					convert >> std::boolalpha >> OutEnabel;
					continue;
				}
				if (startsWith(line, "OutputChannel=")) {
					convert << line.substr(line.find("OutputChannel=") + 14);
					convert >> resultChannel;
					continue;
				}
				if (startsWith(line, "OutputSize=")) {
					convert << line.substr(line.find("OutputSize=") + 11);
					convert >> resultSize;
					continue;
				}
				if (startsWith(line, "CopyDirStruct=")) {
					convert << line.substr(line.find("CopyDirStruct=") + 14);
					convert >> std::boolalpha >> copyDirStruct;
					continue;
				}
				if (startsWith(line, "InBufferQueue=")) {
					convert << line.substr(line.find("InBufferQueue=") + 14);
					convert >> MaxIBQS;
					continue;
				}
				if (startsWith(line, "OutBufferQueue=")) {
					convert << line.substr(line.find("OutBufferQueue=") + 15);
					convert >> MaxOBQS;
				}
			}
		}
		else {
			std::stringstream sstr;
			sstr << "\t 未找到文件 " << settingpath << " 配置文件不存在！！！\n";
			Log(sstr , true);
		}

		fr.Close();
		std::stringstream ss;
		ss << "\n 配置信息 :" <<
			" \n\t 处理对象路径: " << path.srcPath <<
			" \n\t 结果保存路径: " << path.hashPath <<
			" \n\t 对象筛选器: " << path.filter <<
			" \n\t CL程序路径: " << path.codePath <<
			" \n\t CL程序入口: " << path.entryName;

		IOBufferEnabel ? ss << "\n\t 启用IO缓冲队列 "<< "（ In队列大小：" << MaxIBQS << " Out队列大小：" << MaxOBQS << " ） " : ss << "\n\t 未启用IO缓冲队列";
		if (IOBufferEnabel) {
			ss << " 显存输入缓冲队列：" << std::boolalpha << InEnabel;
			ss << " 显存输出缓冲队列：" << std::boolalpha << OutEnabel;
		}
		ss << "\n";	
		Log(ss,true);
	}
	
	void scan(SafeList<std::string> * list) {
		std::cout<<"\n扫描目标文件中\n";
		fp.getFilesInDict(path.srcPath, list, path.filter, true);

		totalcount = list->size();
		std::stringstream ss;

		ss << "\n扫描完成,共" << totalcount << "个待处理对象。\n" << std::endl;
		Log(ss,true);

	}

	void RunMult(SafeList<std::string> * srcList) {
		Performance::get(&starttime);
		path.copySrcPath = copyDirStruct;

		std::cout << " 准备中...";

		for (int i = 0; i < selectList.size(); i++) {
			ruuntarget.push_back(new ImageProcess());
			DeviceBlock * db = &selectList[i]->devices[selectList[i]->selectedDevice];
			ruuntarget.back()->Prepare(srcList,*selectList[i]->id, db, path, resultChannel, resultSize);
			if (IOBufferEnabel)ruuntarget.back()->Enable(IOBufferEnabel, InEnabel, MaxIBQS, MaxOBQS, OutEnabel);
			ruuntarget.back()->start();
			Sleep(500);
		}

		size_t sum = 0,ersum=0;  
		std::cout << std::endl;
		std::stringstream outlog;

		do {
			
			Sleep(900);

			CanExite = true;
			sum = 0;
			ersum = 0;

			for (int i = 0; i < ruuntarget.size(); i++) {
				
				sum += ruuntarget[i]->getFinishedCount();
				
				ersum += ruuntarget[i]->getErrorCount();
				
				outlog << i << " I:O " << ruuntarget[i]->getInBufferCount() << ":" << ruuntarget[i]->getOutBufferCount()<<"  ";
				
				CanExite = CanExite && ruuntarget[i]->getStatus();
			}
			
			CurrentRate = sum - lastcount;

			if (CanExite) {
				std::cout << std::endl;
				break;
			}
			else {
				std::cout << "\r";
				std::cout<< " 已完成 " << std::setprecision(5) << 100 * (float)sum / totalcount << "%   已耗时" << Performance::getTakeTime(starttime, true)  << "秒  剩余:" << totalcount - sum - ersum << "个任务("<<ersum<<"个任务出错)  速率:"<<  CurrentRate <<"/s  "<< outlog.str()<<"          ";
				ClearStream(outlog);
				if (CurrentRate > MaxRate)MaxRate = CurrentRate;
				if (CurrentRate < MinRate && CurrentRate != 0) MinRate = CurrentRate;
				AverageRate += CurrentRate;
				lastcount =  sum;
			}
			//Sleep(100);

		} while (true);

		lasttime = Performance::getTakeTime(starttime, true);
		AverageRate =(size_t)(AverageRate / lasttime);

		std::stringstream end;
		
		end << "\n 完成！！！\n\t 共计" << sum + ersum << "个任务(其中失败" << ersum << "个)  共耗时" << lasttime << "秒 平均速率："<<AverageRate<<"/s 最高速率："<<MaxRate<<"/s 最低速率:"<<MinRate<<"/s \n ";
		Log(end, true);
	}
};