/**
 * Copyrights (C) 2015 - 2016
 * All rights reserved
 *
 * File: log.h
 *
 * 日志封装
 *
 * Change Logs:
 * Date         Author      Notes
 * 2016-08-04   chenfei     创建
 */
#ifndef __CF_LOG_H
#define __CF_LOG_H

#include "logcat.h"
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

namespace cpplog {
const char* StripFileName(const char *full_name);
}

#if 1
/**
					* 设置日志级别
					* \param type 级别
					*/
#define LOG_LEV(level) \
	cpplog::root.setPriority(level);
/**
					* 流方式输出日志
					* \param type 级别
					*/
#define LOG_CXX(level) \
	cpplog::root.log(level) << "[" << ::syscall(SYS_gettid) << "][" \
					<< cpplog::StripFileName(__FILE__) << " L" << __LINE__ << "] " \
					<< __FUNCTION__ << "(): "

/**
					* 格式化输出日志
					* \param type 级别
					* \param fmt  格式化参数
					*/
#define LOG_C(level, fmt, ...) \
	cpplog::root.log(level, "[%lu][%s L%d] %s(): " fmt, ::syscall(SYS_gettid), \
					cpplog::StripFileName(__FILE__), __LINE__, __FUNCTION__, \
					## __VA_ARGS__);

/**
* 调试标签
*/
#define LOG_TAG  LOG_CXX(LOG_DEBUG) << "===[ RUN HERE ]===";

#else

#include <stdio.h>
#include <iostream>

#define LOG_LEV(level)\
	cpplog::root.setPriority(level);

#define LOG_CXX(level) \
	std::cout << std::endl \
			  << "["<< ::syscall(SYS_gettid) << "][" \
			  << cpplog::StripFileName(__FILE__) << " L" << __LINE__ << "] " \
			  << __FUNCTION__ << "(): "

#define LOG_C(level,fmt,...) \
	::printf("\n[%lu][%s L%d] %s(): " fmt, \
			 ::syscall(SYS_gettid),\
			 cpplog::StripFileName(__FILE__), __LINE__, __FUNCTION__, \
			 ## __VA_ARGS__);\
			 fflush(stdout);

#define LOG_TAG \
	::printf("\n[%lu][%s L%d] %s(): ===[ RUN HERE ]===" , \
			 ::syscall(SYS_gettid),\
			 cpplog::StripFileName(__FILE__), __LINE__, __FUNCTION__);\
			 fflush(stdout);

#endif

#endif
