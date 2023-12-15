//
// Created by Administrator on 2021/10/12.
//

#ifndef UNTITLED1_UTILS_TIMER_H_
#define UNTITLED1_UTILS_TIMER_H_

#include "NamespaceDecl.h"
#include <iostream>
#include <ctime>

NAMESPACE_BEGIN(timer)

static clock_t lastClock = 0;

void init();
float getTime();
std::string getTimeString();

NAMESPACE_END

#endif //UNTITLED1_UTILS_TIMER_H_
