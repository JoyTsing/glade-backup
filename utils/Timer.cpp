//
// Created by Administrator on 2021/10/12.
//

#include "Timer.h"

NAMESPACE_BEGIN(timer)

void init()
{
  lastClock = clock();
}

float getTime()
{
  auto curClock = clock();
  float res = static_cast<float>(curClock - lastClock) / CLOCKS_PER_SEC;
  lastClock = curClock;
  return res;
}

std::string getTimeString()
{
  return "Time: " + std::to_string(getTime());
}

NAMESPACE_END