/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */

#ifndef __TARS_LOGGER_H__
#define __TARS_LOGGER_H__

#include "util/tc_logger.h"
#include "util/tc_file.h"
#include "util/tc_singleton.h"

#define DYEING_DIR "tars_dyeing"
#define DYEING_FILE "dyeing"

namespace tars
{

/**
 * LOG的库说明:
 * 1 循环日志采用TLOGERROR(...),TLOGDEBUG(...)
 * 2 循环日志不上传到服务器
 * 3 按天日志采用DLOG, FDLOG来记录
 * 4 按天日志也可以不上传到远程服务器:DLOG("")->disableRemote();
 * 5 按天日志可以改变每天一个文件的方式:
 *   DLOG("abc3")->setFormat("%Y%m%d%H");
 *   每个小时一个文件
 */

/*****************************************************************************
实现方式说明(只介绍按时间的日志, 会写到tarslog):
    1 自定义时间日志的WriteT类:RemoteTimeWriteT
    2 在RemoteTimeWriteT类中, 写入到远程
    3 定义远程日志类:typedef TC_Logger<RemoteTimeWriteT, TC_RollByTime> RemoteTimeLogger;
    4 为了保证远程的写日志也是在单独线程处理,重新定义本地按天日志类
    5 自定义时间日志的WriteT类:TimeWriteT
    6 在TimeWriteT类中包含RemoteTimeLogger对象
    7 在TimeWriteT类的写入操作中, 写入本地文件后, 同时写入到RemoteTimeLogger对象中
    8 RemoteTimeLogger会在RemoteTimeWriteT对象中, 异步写入到远程
    9 从而本地文件写和远程写不在一个线程中.
*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////

class RollWriteT
{
public:
    RollWriteT();
    ~RollWriteT();

    void operator()(ostream &of, const deque<pair<int, string> > &ds);

    void setDyeingLogInfo(const string &sApp, const string &sServer, const string & sLogPath,
            int iMaxSize, int iMaxNum, const string & sLogObj);

protected:

    TC_RollLogger *_dyeingRollLogger;

    static int  _dyeingThread;

    string _app;
    string _server;
    string _logPath;
    int _maxSize;
    int _maxNum;

    /**
     * 染色远程滚动日志代理
     */
    //LogPrx                _logPrx;


};


/**
 * 本地日志帮助类, 单件
 * 循环日志单件是永生不死的, 保证任何地方都可以使用
 * 当该对象析够以后, 则直接cout出来
 */
class TarsRollLogger : public TC_Singleton<TarsRollLogger, CreateUsingNew, PhoneixLifetime>
{
public:
    enum
    {
        NONE_LOG    = 1,    /**所有的log都不写*/
        ERROR_LOG   = 2,    /**写错误log*/
        WARN_LOG    = 3,    /**写错误,警告log*/
        DEBUG_LOG   = 4,    /**写错误,警告,调试log*/
        INFO_LOG    = 5        /**写错误,警告,调试,Info log*/
    };
public:
    typedef TC_Logger<RollWriteT, TC_RollBySize> RollLogger;

    /**
     * 设置本地信息
     * @param app, 业务名称
     * @param server, 服务名称
     * @param logpath, 日志路径
     * @param iMaxSize, 文件最大大小,字节
     * @param iMaxNum, 文件最大数
     */
    void setLogInfo(const string &sApp, const string &sServer, const string &sLogpath, int iMaxSize = 1024*1024*50, int iMaxNum = 10, const string &sLogObj="");

    /**
     * 设置同步写日志
     *
     * @param bSync
     */
    void sync(bool bSync = true);

    /**
     * 获取循环日志
     *
     * @return RollLogger
     */
    RollLogger *logger()          { return &_logger; }

    /**
     * 染色日志是否启用
     * @param bEnable
     */
    void enableDyeing(bool bEnable, const string& sDyeingKey = "");

protected:

    /**
     * 应用
     */
    string                  _app;

    /**
     * 服务名称
     */
    string                  _server;

    /**
     * 日志路径
     */
    string                  _logpath;

    /**
     * 循环日志
     */
    RollLogger              _logger;

    /**
     * 本地线程组
     */
    TC_LoggerThreadGroup    _local;

};

///////////////////////////////////////////////////////////////////////////////////////
//
/**
 * 写日志线程
 * 将写本地日志和远程分开到不同的线程
 * 作为单件存在, 且是永生不死的单件
 */
class TarsLoggerThread : public TC_Singleton<TarsLoggerThread, CreateUsingNew, PhoneixLifetime>
{
public:
    /**
     * 构造函数
     */
    TarsLoggerThread();

    /**
     * 析够函数
     */
    ~TarsLoggerThread();

    /**
     * 本地写日志线程
     */
    TC_LoggerThreadGroup* local();

    /**
     * 远程写日志线程
     *
     * @return TC_LoggerThreadGroup*
     */
    TC_LoggerThreadGroup* remote();

protected:

    /**
     * 本地线程组
     */
    TC_LoggerThreadGroup    _local;

    /**
     * 远程写线程组
     */
    TC_LoggerThreadGroup    _remote;
};

///////////////////////////////////////////////////////////////////////////////////////
class TimeWriteT;

/**
 * 远程的Log写操作类
 */

////////////////////////////////////////////////////////////////////////////
/**
 * 写Logger
 */

////////////////////////////////////////////////////////////////////////////
/**
 * 远程日志帮助类, 单件
 */

/**
 * 染色开关类，析构时关闭
 */

/**
 * 循环日志
 */
#define LOG             (TarsRollLogger::getInstance()->logger())

/**
 * @brief 按级别循环日志宏
 *
 * @param level 日志等级,TarsRollLogger::INFO_LOG,TarsRollLogger::DEBUG_LOG,TarsRollLogger::WARN_LOG,TarsRollLogger::ERROR_LOG
 * @msg 日志内容语句,包括<<重定向符连接的语句,如 "Demo begin" << " testing !" <<endl;
 *
 * @用法:
 *       标准输出流方式: cout << "I have " << vApple.size() << " apples!"<<endl;
 *       框架宏方式:     LOGMSG(TarsRollLogger::INFO_LOG,"I have " << vApple.size() << " apples!"<<endl);
 */
#define LOGMSG(level,msg...) do{ if(LOG->IsNeedLog(level)) LOG->log(level)<<msg;}while(0)

/**
 * @brief 按级别循环日志
 *
 * @msg 日志内容语句,包括<<重定向符连接的语句,如 "Demo begin" << " testing !" <<endl;
 *
 * @用法:
 *       标准输出流方式: cout << "I have " << vApple.size() << " apples!"<<endl;
 *       框架宏方式:     TLOGINFO("I have " << vApple.size() << " apples!"<<endl);
 */
#define TLOGINFO(msg...)    LOGMSG(TarsRollLogger::INFO_LOG,msg)
#define TLOGDEBUG(msg...)   LOGMSG(TarsRollLogger::DEBUG_LOG,msg)
#define TLOGWARN(msg...)    LOGMSG(TarsRollLogger::WARN_LOG,msg)
#define TLOGERROR(msg...)   LOGMSG(TarsRollLogger::ERROR_LOG,msg)

/**
 * 按天日志
 */
#define DLOG            (TarsTimeLogger::getInstance()->logger()->any())
#define FDLOG(x)        (TarsTimeLogger::getInstance()->logger(x)->any())
#define FFDLOG(x,y,z)   (TarsTimeLogger::getInstance()->logger(x,y,z)->any())

/**
 *  按天日志局部使能开关，针对单个日志文件进行使能，请在所有按天日志输出前调用
 */
#define TENREMOTE_FDLOG(swith,sApp,sServer,sFile) (TarsTimeLogger::getInstance()->enableRemoteEx(sApp,sServer,sFile,swith))
#define TENLOCAL_FDLOG(swith,sApp,sServer,sFile)  (TarsTimeLogger::getInstance()->enableLocalEx(sApp,sServer,sFile,swith))

/**
 * 按天日志全局使能开关，请在所有按天日志输出前调用
 */
#define TENREMOTE(swith) (TarsTimeLogger::getInstance()->enableRemoteLog(swith))
#define TENLOCAL(swith)  (TarsTimeLogger::getInstance()->enableLocalLog(swith))
}

#endif


