#include "TarsLogger.h"
#include <iostream>

using namespace std;

namespace tars
{

int RollWriteT::_dyeingThread = 0;

/////////////////////////////////////////////////////////////////////////////////////

RollWriteT::RollWriteT():_dyeingRollLogger(NULL), _maxSize(10000), _maxNum(1)
{
}

RollWriteT::~RollWriteT()
{
    if(_dyeingRollLogger)
    {
        delete _dyeingRollLogger;
    }
}

void RollWriteT::operator()(ostream &of, const deque<pair<int, string> > &ds)
{
    vector<string> vRemoteDyeing;

    deque<pair<int, string> >::const_iterator it = ds.begin();
    while(it != ds.end())
    {
        of << it->second;

        //染色线程id不存在
        if(it->first != 0)
        {
            if(!_dyeingRollLogger)
            {
                string sDyeingDir = _logPath;
                sDyeingDir += "/";
                sDyeingDir += DYEING_DIR;
                sDyeingDir += "/";

                string sDyeingFile = sDyeingDir;
                sDyeingFile += DYEING_FILE;

                TC_File::makeDirRecursive(sDyeingDir);

                //初始化染色循环日志
                _dyeingRollLogger = new TC_RollLogger();

                _dyeingRollLogger->init(sDyeingFile, _maxSize, _maxNum);
                _dyeingRollLogger->modFlag(TC_DayLogger::HAS_TIME, false);
                _dyeingRollLogger->modFlag(TC_DayLogger::HAS_TIME|TC_DayLogger::HAS_LEVEL|TC_DayLogger::HAS_PID, true);
                _dyeingRollLogger->setLogLevel("DEBUG");
            }

            _dyeingRollLogger->roll(make_pair(it->first, _app + "." + _server + "|" + it->second ));

            vRemoteDyeing.push_back(_app + "." + _server + "|" + it->second);
        }

        ++it;
    }
    of.flush();

}

void RollWriteT::setDyeingLogInfo(const string &sApp, const string &sServer, const string & sLogPath, int iMaxSize, int iMaxNum, const string &sLogObj)
{
    _app     = sApp;
    _server  = sServer;
    _logPath = sLogPath;
    _maxSize = iMaxSize;
    _maxNum  = iMaxNum;

}

/////////////////////////////////////////////////////////////////////////////////////

void TarsRollLogger::setLogInfo(const string &sApp, const string &sServer, const string &sLogpath, int iMaxSize, int iMaxNum, const string &sLogObj)
{
    _app       = sApp;
    _server    = sServer;
    _logpath   = sLogpath;

    //生成目录
    TC_File::makeDirRecursive(_logpath + "/" + _app + "/" + _server);

	cout<<"start threadpool"<<endl;
    _local.start(1);

    //初始化本地循环日志
	cout<<"_logger.init"<<endl;
    _logger.init(_logpath + "/" + _app + "/" + _server + "/" + _app + "." + _server, iMaxSize, iMaxNum);
    _logger.modFlag(TC_DayLogger::HAS_TIME, false);
    _logger.modFlag(TC_DayLogger::HAS_TIME|TC_DayLogger::HAS_LEVEL|TC_DayLogger::HAS_PID, true);

    //设置为异步
    sync(false);
}


void TarsRollLogger::sync(bool bSync)
{
    if(bSync)
    {
        _logger.unSetupThread();
    }
    else
    {
        _logger.setupThread(&_local);
    }
}

void TarsRollLogger::enableDyeing(bool bEnable, const string& sDyeingKey/* = ""*/)
{
    _logger.getRoll()->enableDyeing(bEnable, sDyeingKey);
}

/////////////////////////////////////////////////////////////////////////////////////

TarsLoggerThread::TarsLoggerThread()
{
    _local.start(1);
    _remote.start(1);
}

TarsLoggerThread::~TarsLoggerThread()
{
    //先刷新本地日志
    _local.flush();

    //再刷新远程日志, 保证不会丢日志
    _remote.flush();
}

TC_LoggerThreadGroup* TarsLoggerThread::local()
{
    return &_local;
}

TC_LoggerThreadGroup* TarsLoggerThread::remote()
{
    return &_remote;
}

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////


}
