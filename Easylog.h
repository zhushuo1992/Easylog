/*
 * Easylog.h
 *
 *  Created on: 2018年12月7日
 *      Author: Administrator
 *      支持非C++11,支持日志级别，支持留式日志,支持printf，不需要定义，只需要包含头文件
 *      感谢原作者sollyu
 *      by zhushuo
 */

#ifndef EASYLOG_H_
#define EASYLOG_H_

//
//  EasyLog.h
//
//  Created by sollyu on 14/11/20.
//  Copyright (c) 2014年 sollyu. All rights reserved.
//

/**
	#include <iostream>
	#include "Easylog.h"

	ELOGI("i'm %s", "sollyu");          // 输出 INFO (只有 LOGI 不会打印所在行)
	ELOGE("I'm " << "sollyu");          // 输出 ERROR (会打印所在行)
	ELOG_DEBUG("i'm %s", "sollyu");     // 输出 DEBUG (会打印所在行)
	EasyLog::GetInstance()->WriteLog(EasyLog::LOG_DEBUG, "i'm %s", "sollyu");

 // 上面代码的执行结果
[2018.12.09] [19:48:41] [LOG_INFO ] ------------------ LOG SYSTEM START ------------------

[2018.12.09] [19:48:41] [LOG_INFO ] i'm zhushuo
[2018.12.09] [19:48:41] [LOG_ERROR] I'm sollyu (..\src\Easylog.cpp : main : 16 )
[2018.12.09] [19:48:41] [LOG_DEBUG] i'm sollyu (..\src\Easylog.cpp : main : 17 )
[2018.12.09] [19:48:41] [LOG_DEBUG] i'm sollyu

 */
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <functional>
#include <iomanip>
#include <stdarg.h>
#include <time.h>

#ifndef EASY_LOG_FILE_NAME
#  define EASY_LOG_FILE_NAME			"EasyLog.log"   /** 日志的文件名 */
#endif

#ifndef EASY_LOG_LINE_BUFF_SIZE
#  define EASY_LOG_LINE_BUFF_SIZE		1024            /** 一行的最大缓冲 */
#endif

#ifndef EASY_LOG_DISABLE_LOG
#  define EASY_LOG_DISABLE_LOG          0               /** 非0表示禁用LOG */
#endif

#ifndef EASY_LOG_PRINT_LOG
#  define EASY_LOG_PRINT_LOG          0               /** 非0表示禁用打印LOG */
#endif

#ifndef EASY_LOG_COVER_LOG
#  define EASY_LOG_COVER_LOG          1               /** 非0表示追加写 */
#endif

#ifdef WIN32
#else
#   define  sprintf_s   sprintf
#   define  vsnprintf_s vsnprintf//此处因为我用的是mingw，直接用了vsnprintf，请自行判断
#endif

/** 写日志方法 */
#define EWRITE_LOG(LEVEL, FMT, ...) \
{ \
    std::stringstream ss; \
    ss << FMT; \
    if (LEVEL != EasyLog::LOG_INFO) \
    { \
        ss << " (" << __FILE__ << " : " << __FUNCTION__ << " : " << __LINE__ << " )"; \
    } \
    EasyLog::GetInstance()->WriteLog(LEVEL, ss.str().c_str(), ##__VA_ARGS__); \
}

//! 快速宏
#define ELOG_TRACE(FMT , ...) EWRITE_LOG(EasyLog::LOG_TRACE, FMT, ##__VA_ARGS__)
#define ELOG_DEBUG(FMT , ...) EWRITE_LOG(EasyLog::LOG_DEBUG, FMT, ##__VA_ARGS__)
#define ELOG_INFO(FMT  , ...) EWRITE_LOG(EasyLog::LOG_INFO , FMT, ##__VA_ARGS__)
#define ELOG_WARN(FMT  , ...) EWRITE_LOG(EasyLog::LOG_WARN , FMT, ##__VA_ARGS__)
#define ELOG_ERROR(FMT , ...) EWRITE_LOG(EasyLog::LOG_ERROR, FMT, ##__VA_ARGS__)
#define ELOG_ALARM(FMT , ...) EWRITE_LOG(EasyLog::LOG_ALARM, FMT, ##__VA_ARGS__)
#define ELOG_FATAL(FMT , ...) EWRITE_LOG(EasyLog::LOG_FATAL, FMT, ##__VA_ARGS__)

#define ELOGT( FMT , ... ) ELOG_TRACE(FMT, ##__VA_ARGS__)
#define ELOGD( FMT , ... ) ELOG_DEBUG(FMT, ##__VA_ARGS__)
#define ELOGI( FMT , ... ) ELOG_INFO (FMT, ##__VA_ARGS__)
#define ELOGW( FMT , ... ) ELOG_WARN (FMT, ##__VA_ARGS__)
#define ELOGE( FMT , ... ) ELOG_ERROR(FMT, ##__VA_ARGS__)
#define ELOGA( FMT , ... ) ELOG_ALARM(FMT, ##__VA_ARGS__)
#define ELOGF( FMT , ... ) ELOG_FATAL(FMT, ##__VA_ARGS__)

// ============================================================
// time in 24 hours hh:mm:ss format
// ============================================================
static std::string TimeStamp()
{
	char str[9];

	// get the time, and convert it to struct tm format
	time_t a = time(0);
	struct tm* b = localtime(&a);

	// print the time to the string
	strftime(str, 9, "%H:%M:%S", b);

	return str;
}

// ============================================================
// date YYYY:MM:DD format
// ============================================================
static std::string DateStamp()
{
	char str[11];

	// get the time, and convert it to struct tm format
	time_t a = time(0);
	struct tm* b = localtime(&a);//VS中请自行修改

	// print the time to the string
	strftime(str, 11, "%Y.%m.%d", b);

	return str;
}


class EasyLog
{
public:
    /** 日志级别*/
	enum LOG_LEVEL { LOG_TRACE = 0, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_ALARM,  LOG_FATAL };

public:
    /** 单例模式 */
    static EasyLog * GetInstance() { static EasyLog* m_pInstance = new EasyLog(); return m_pInstance; }
    //void EasyLogDestroy(){delete this;}//调用：EasyLog::GetInstance()->EasyLogDestroy();不建议调用，destroy后再次调用会崩溃，每次写日志已经flush了，所以不太需要，如果要调用，请保证在最后用

public:


    /** 写日志操作 */
	void WriteLog(LOG_LEVEL level, const char *pLogText, ...)
	{
		va_list args;
		char logText[EASY_LOG_LINE_BUFF_SIZE] = { 0 };
		va_start(args, pLogText);
		vsnprintf(logText, EASY_LOG_LINE_BUFF_SIZE - 1, pLogText, args);
		WriteLog(logText, level);
	}

	void WriteLog(std::string logText, LOG_LEVEL level = LOG_ERROR)
	{
		static const char *const LOG_STRING[] =
		{
			"LOG_TRACE",
			"LOG_DEBUG",
			"LOG_INFO ",
			"LOG_WARN ",
			"LOG_ERROR",
			"LOG_ALARM",
			"LOG_FATAL",
		};


		// 生成一行LOG字符串
        std::stringstream szLogLine;
        szLogLine << "[" << DateStamp() <<"] [" << TimeStamp() << "] [" << LOG_STRING[level] << "] " << logText<<"\n";//如果有需要请改成\r\n


#if defined EASY_LOG_DISABLE_LOG && EASY_LOG_DISABLE_LOG == 0
		/* 输出LOG字符串 - 文件打开不成功的情况下按照标准输出 */
		if (m_fileOut.is_open())
		{
		    m_fileOut.write(szLogLine.str().c_str(), szLogLine.str().size());
		    m_fileOut.flush();
		}
		else
		{
		    std::cout << szLogLine.str();
		}
#endif

#if defined EASY_LOG_PRINT_LOG && EASY_LOG_PRINT_LOG == 0
		std::cout << szLogLine.str();
#endif

	}

private:
    EasyLog(void)
	{
#if defined EASY_LOG_COVER_LOG && EASY_LOG_COVER_LOG == 0
		m_fileOut.open(EASY_LOG_FILE_NAME, std::ofstream::out);
#else
		m_fileOut.open(EASY_LOG_FILE_NAME, std::ofstream::out | std::ofstream::app);
#endif
		WriteLog("------------------ LOG SYSTEM START ------------------\n", EasyLog::LOG_INFO);
	}
    virtual ~EasyLog(void)
	{
		WriteLog("------------------ LOG SYSTEM END ------------------ ", EasyLog::LOG_INFO);
		if (m_fileOut.is_open()) m_fileOut.close();
	}

private:
    /** 写文件 */
    std::ofstream m_fileOut;
};


#endif /* EASYLOG_H_ */
