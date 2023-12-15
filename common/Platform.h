//
// Created by Administrator on 2021/9/24.
//

#ifndef UNTITLED1_COMMON_PLATFORM_H_
#define UNTITLED1_COMMON_PLATFORM_H_

#if defined(linux) || defined(__linux) || defined(__linux__)
#define PLATFORM_LINUX
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define PLATFORM_WIN
#endif

#endif //UNTITLED1_COMMON_PLATFORM_H_
