//MIT License
//
//Copyright(c) 2018 Alex Kasitskyi
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <ostream>

#ifdef ANDROID
#include <android/log.h>
#else
#include <iostream>
#endif

namespace c4 {
	template<typename T1, typename T2>
	std::ostream& operator<<(std::ostream &out, const std::pair<T1, T2> &t) {
		out << "( " << t.first << " , " << t.second << " )";
		return out;
	}

	template<typename T>
	std::ostream& operator<<(std::ostream &out, const std::vector<T> &t) {
		out << "{ ";
		for(size_t i = 0; i < t.size(); i++) {
			if(i)
				out << ", ";
			out << t[i];
		}
		out << " }";
		return out;
	}

	enum LogLevel{
		LOG_ERROR,
		LOG_WARN,
		LOG_INFO,
		LOG_DEBUG,
		LOG_VERBOSE
	};

	class Logger {
	public:
		Logger(LogLevel level) : level(level) {}
		
		template<typename T>
		Logger& operator<<(const T& v) {
			ss << v;
			return *this;
		}

		~Logger(){
			ss << "\n";
			
			switch(level)
			{
#ifdef ANDROID
            case LOG_ERROR:
                __android_log_print(ANDROID_LOG_ERROR, "", "%s", ss.str().c_str());
                break;
            case LOG_WARN:
                __android_log_print(ANDROID_LOG_WARN, "", "%s", ss.str().c_str());
                break;
            case LOG_INFO:
                __android_log_print(ANDROID_LOG_INFO, "", "%s", ss.str().c_str());
                break;
#ifndef C4_DEBUG_OUTPUT_DISABLED
            case LOG_DEBUG:
                __android_log_print(ANDROID_LOG_DEBUG, "", "%s", ss.str().c_str());
                break;
            case LOG_VERBOSE:
                __android_log_print(ANDROID_LOG_VERBOSE, "", "%s", ss.str().c_str());
                break;
#endif
#else
            case LOG_ERROR :
                std::cerr << "E: " << (ss.str().c_str());
				break;
			case LOG_WARN :
                std::cerr << "W: " << (ss.str().c_str());
				break;
			case LOG_INFO :
                std::cout << "I: " << (ss.str().c_str());
				break;
#ifndef C4_DEBUG_OUTPUT_DISABLED
			case LOG_DEBUG :
                std::cout << "D: " << (ss.str().c_str());
				break;
			case LOG_VERBOSE :
                std::cout << "V: " << (ss.str().c_str());
				break;
#endif
#endif
            }
		}
	private:
		std::stringstream ss;
		LogLevel level;
	};

#define LOGE c4::Logger(c4::LOG_ERROR)
#define LOGW c4::Logger(c4::LOG_WARN)
#define LOGI c4::Logger(c4::LOG_INFO)
#define LOGD c4::Logger(c4::LOG_DEBUG)
#define LOGV c4::Logger(c4::LOG_VERBOSE)

#define PRINT_DEBUG(X) LOGD << #X << " = " << (X) << " "

}; // namespace c4
