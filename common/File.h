//
// Created by Administrator on 2021/9/24.
//

#ifndef UNTITLED1_COMMON_FILE_H_
#define UNTITLED1_COMMON_FILE_H_

#include <iostream>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <fileapi.h>

#include "DataTypes.h"
#include "Platform.h"
#include "../utils/NamespaceDecl.h"
#include "../utils/Win32Util.h"

NAMESPACE_BEGIN(file)

// 硬链接的寻找是件麻烦的事情，因此只考虑选定文件所属文件夹下的硬链接关系

using SearchHandle = util::FileSearchHandle;
using ContentHandle = util::FileContentHandle;

struct FileHeader
{
  wchar_t name[MAX_PATH];
  size_t size;
  uint64_t code;
};

enum class Type : uint8_t
{
  Other = 0b0000,
  Regular = 0b0001,
  Directory = 0b0010,
  SymbLink = 0b0100,
  HardLink = 0b1000
};

std::pair<SearchHandle, WIN32_FIND_DATAW> findWithData(const wchar_t *filePath);
std::pair<SearchHandle, WIN32_FIND_DATAW> findWithData(const std::wstring &filePath);

SearchHandle find(const wchar_t *filePath);
SearchHandle find(const std::wstring &filePath);

ContentHandle open(const wchar_t *filePath, bool toFinalPath = true);
ContentHandle open(const std::wstring &filePath, bool toFinalPath = true);

ContentHandle openReadOnly(const std::wstring &filePath);
ContentHandle openFinalReadOnly(const std::wstring &filePath);

ContentHandle createFile(const wchar_t *filePath);
ContentHandle createFile(const std::wstring &filePath);

BY_HANDLE_FILE_INFORMATION getInfo(const ContentHandle &file);
BY_HANDLE_FILE_INFORMATION getInfo(const wchar_t *filePath);
BY_HANDLE_FILE_INFORMATION getInfo(const std::wstring &filePath);

//Type getType(const ContentHandle &file);
Type getType(const wchar_t *filePath);
Type getType(const std::wstring &filePath);

uint64_t getIndex(const ContentHandle &file);
uint64_t getIndex(const wchar_t *filePath);
uint64_t getIndex(const std::wstring &filePath);

size_t getSize(const ContentHandle &file);
size_t getSize(const wchar_t *filePath);
size_t getSize(const std::wstring &filePath);

std::wstring getSymbLinkPath(const ContentHandle &file);
std::wstring getSymbLinkPath(const wchar_t *filePath);
std::wstring getSymbLinkPath(const std::wstring &filePath);

std::wstring getTruncatedPath(const wchar_t *filePath);
std::wstring getTruncatedPath(const std::wstring &filePath);

std::wstring infoToString(const BY_HANDLE_FILE_INFORMATION &info);
std::wstring infoToString(const WIN32_FIND_DATAW &info);

std::wstring typeToString(const Type &type);

FILETIME sysTimeToFileTime(const SYSTEMTIME &time);
SYSTEMTIME fileTimeToSysTime(const FILETIME &time);
std::wstring sysTimeToString(const SYSTEMTIME &time);
std::wstring fileTimeToString(const FILETIME &time);

uint64_t hash(const ContentHandle &file);

NAMESPACE_END

#endif //UNTITLED1_COMMON_FILE_H_
