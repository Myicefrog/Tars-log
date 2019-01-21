#include "TarsLogger.h"
#include <iostream>
#include <string>

using namespace std;
using namespace tars;

int main()
{
	TarsRollLogger::getInstance()->setLogInfo("Hello", "HelloServer", "/home/luguang/demo/tars-log/", 52428800, 10, "tars.hellolog.LogObj");

	TarsRollLogger::getInstance()->logger()->setLogLevel("DEBUG");

	TarsRollLogger::getInstance()->logger()->debug() << "[TARS]" << "Hello" << endl;

	return 0;
}
