#pragma once
#include<direct.h>
#include<io.h>
#include<time.h>
#include"SafeList.h"
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<mutex>
#include<windows.h>
#include <thread>

#define DEBUG
#define LOG

static std::string getTime() {
	time_t tp;
	tm  p;
	time(&tp);
	localtime_s(&p, &tp);
	std::stringstream tstr;
	tstr << p.tm_year + 1900 << "年" << p.tm_mon + 1 << "月" << p.tm_mday << "日" << p.tm_hour << "点" << p.tm_min << "分" << p.tm_sec<<"秒";
	return tstr.str();
}

static int startsWith(std::string s, std::string sub) {
	return s.find(sub) == 0 ? 1 : 0;
}

static int endsWith(std::string s, std::string sub) {
	return s.rfind(sub) == (s.length() - sub.length()) ? 1 : 0;
}

class CInfoCallback
{
public:
	virtual void OnProgramError(int code, std::stringstream  e) = 0;
	virtual void OnKernelError (int code, std::stringstream  e) = 0;
	virtual void OnBufferError (int code, std::stringstream  e) = 0;
	virtual void OnQueueError  (int code, std::stringstream  e) = 0;
	virtual void OnDataError   (int code, std::stringstream  e) = 0;
	virtual void OnContextError(int code, std::stringstream  e) = 0;
	virtual void OnFolderScaned() = 0;
	virtual void OnFileScaned(std::string path) = 0;
};



class FileProcess {
public:
	static  void LoadFile(LPCWSTR FileName,char * buffer,size_t *size) {
		HANDLE File=CreateFileW(FileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		LARGE_INTEGER FileSize;
		DWORD HP;
		FileSize.LowPart = GetFileSize(File, &HP);
		FileSize.HighPart = HP;
		buffer = new char[FileSize.QuadPart];
		*size = FileSize.QuadPart;
		bool result= ReadFile(File, buffer, (DWORD)(FileSize.QuadPart), NULL, NULL);
	}
	static void SaveFile(LPCWSTR FileName,char * buffer,LARGE_INTEGER size) {
		HANDLE File = CreateFileW(FileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		bool result=WriteFile(File, buffer, (DWORD)(size.QuadPart), NULL, NULL);
	}

	static std::string getFileNameFromPath(std::string fullPath) {

		return fullPath.substr(fullPath.find_last_of("\\")+1);
	}
	static std::string getFilePathFromPath(std::string fullPath) {
		return fullPath.substr(0,fullPath.find_last_of("\\"));
	}


	static void createFolders(std::string fullpath,std::string& original) {
		size_t location = fullpath.find_last_of("\\");
		if (location != fullpath.npos) {
			std::string sortpath = fullpath.substr(0,location);
			//还有子路径，递归
			createFolders(sortpath, original);
			
			if (0 != _access(fullpath.c_str(), 0)) {
				//目标不存在，则创建 如果路径中包含后缀名作文件夹创建。
				if(! endsWith(original,fullpath))_mkdir(fullpath.c_str());

			}
		}
		
	}


	/************************************************************************/
	/* find the File under Path(like H:/a  witch is not a file) 
		the result save into list
		filter like ".jpg|.png" to scan target file,you can use empty("") or "*" to scan every type files
		if FULLPATH is true,the path of file will be saved with file name*/
	/************************************************************************/
	static void getFilesInDict(std::string  path,SafeList<std::string> *list,std::string & filter,bool FULLPATH=false, CInfoCallback * callback=NULL) {
		_finddata_t data;

		std::string search(path.c_str());
		if(!endsWith(search,"\\"))search.append("\\");
		if (!endsWith(search, "\\*"))search.append("*");

		intptr_t handle = _findfirst(search.c_str(), &data);
		
		if ( handle != -1  ) {
			std::cout  << ".";
			size_t i;
			std::string finalname;
			do {
					if (strcmp(data.name,".")!=0 && 0!=strcmp(data.name, "..")) {
								if (data.attrib & _A_SUBDIR ) {

									getFilesInDict(std::string(path.c_str()).append("\\").append(data.name),list, filter,FULLPATH);
								}
								else {

									std::string name = data.name;
									if (filter.find("*")!= filter.npos || filter.empty()) {
										if (FULLPATH)list->pushToBack(std::string(path.c_str()).append("\\").append(data.name));
										else list->pushToBack(name);
									}
									else {
										i= name.find_last_of(".");
										if (i != filter.npos) {//有后缀名的文件
											if (filter.find(name.substr(i)) != filter.npos) {//是筛选目标
												

												FULLPATH ? finalname = std::string(path.c_str()).append("\\").append(data.name) : finalname = name;
												
												list->pushToBack(finalname);

												if (callback != NULL)callback->OnFileScaned(finalname);
											}
										}
										
									}

								}
					}
			} while (0 == _findnext(handle, &data));
			_findclose(handle);	
			if (callback != NULL)callback->OnFolderScaned();
		}
			

	}
};

class FileReader
{
private:
	std::ifstream * file;
	std::string line;
public:
	~FileReader() {
		delete file;
	}
	bool Open(char* filepath,std::ios_base::openmode mode= std::ios_base::in | std::ios_base::binary) {
		file=new std::ifstream(filepath,mode);
		if (!file->fail())return true;
		return false;
	}
	bool ReadLine(std::string * src) {
		if (std::getline(*file, line)) {
			src->swap(line);
			return true;
		}

		else return false;
	}

	template<typename T>
	T *  ReadData(size_t  length) {
		T * buffer= (T *)malloc(length*sizeof(T));
		if(file->read((char *)buffer, length*sizeof(T)))return buffer;
		return NULL;
	}

	template<typename T>
	T * ReadData(T ** buffer ,size_t  length) {
		*buffer= (T*)malloc(length*sizeof(T));
		if (file->read((char *)*buffer, length * sizeof(T)))return *buffer;
		return NULL;
	}

	void Close() {
		file->close();
	}

};

std::mutex FileWriteLocker;

class FileWriter {
	std::ofstream * file;

	bool _WriteEnter() {
		if (file->write("\n", 1))return true;
		return false;
	}
public:

	~FileWriter()
	{
		delete file;
		
	};
	bool Open(char * filepath, std::ios_base::openmode mode = std::ios_base::out | std::ios_base::binary | std::ios_base::trunc) {
		std::string path = filepath;
		path.append("\\").append(getTime()).append(".txt");
		FileProcess::createFolders(path, path);

		//setlocale(LC_ALL, "Chinese-simplified");
		file = new std::ofstream((char*)path.c_str(),mode);
		bool result = file->is_open();
		//setlocale(LC_ALL, "C");
		
		if (result)return true;
		
		std::cout << " Open File Error " << std::endl;
		return false;
	}
	void WriteLine(std::string * line) {
		//FileWriteLocker.lock();
		file->write(line->c_str(), line->size());
		_WriteEnter();
		//FileWriteLocker.unlock();
		file->flush();
	}

	template<typename T>
	bool WriteData(T * data, size_t length) {
			if (!file->write((char *)(data), sizeof(T)*length)){
				return false;
			}
		
		return true;
	}

	void Close() {
		file->close();
	}
};


FileWriter FW; /*= FileWriter((char*)".\\log");*/