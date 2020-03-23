#pragma once
#include<string>

class DllSocket {

public:
	virtual std::string getDllInfo()=0;
	virtual bool run() = 0;
};