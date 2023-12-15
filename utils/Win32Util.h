//
// Created by Administrator on 2021/9/24.
//

#ifndef UNTITLED1_UTILS_WIN32UTIL_H_
#define UNTITLED1_UTILS_WIN32UTIL_H_

#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <windows.h>

#include "NamespaceDecl.h"
#include "Timer.h"

NAMESPACE_BEGIN(util)

static std::set<HANDLE> handleHistory;

template<int Id>
struct HandleKiller
{
  explicit HandleKiller(HANDLE handle) :
      handle(handle)
  {
    handleHistory.insert(handle);
  }
  void release()
  {
    handleHistory.erase(handle);
    CloseHandle(handle);
  }
  HANDLE get() const { return handle; }
  bool valid() { return handle != INVALID_HANDLE_VALUE; }
  void operator = (HANDLE rhs) { handle = rhs; }
  bool operator ! () { return !valid(); }
  HANDLE handle = INVALID_HANDLE_VALUE;
};

template<typename T1, typename T2>
bool testBit(T1 x, T2 bit)
{
  auto _x = static_cast<uint64_t>(x);
  auto _bit = static_cast<uint64_t>(bit);
  return (_x & _bit) == _bit;
}

uint64_t toQword(DWORD high, DWORD low);
uint64_t toQword(FILETIME time);

uint64_t hash(uint64_t seed);
uint64_t hash(const void *data, size_t bytes);

uint64_t getTime();

void forceCloseAllHandles();

// 区分同类型但意义不一定相同的各种HANDLE，以免都不知道怎么用错了
using FileSearchHandle = HandleKiller<0>;
using FileContentHandle = HandleKiller<1>;

NAMESPACE_END

#endif //UNTITLED1_UTILS_WIN32UTIL_H_
