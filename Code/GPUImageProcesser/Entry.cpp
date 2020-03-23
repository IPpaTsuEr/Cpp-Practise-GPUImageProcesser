#include"ThreadManager.h"

int main() {


	SafeList<std::string>  SourceList;
	FW = FileWriter();
	FW.Open((char*)".\\log");
	
	ThreadManager ih = ThreadManager();
	ih.LoadSetting((char*)".\\src\\Setting.set");

	if (!ih.start())return 0;
	ih.scan(& SourceList);
	try {
		ih.RunMult(&SourceList);
	}
	catch (int err) {
		std::cout << "\n 程序因出错而终止 ErrorCode：" << err << std::endl;
	}

	FW.Close();

	system("pause");
}
