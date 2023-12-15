//
// Created by Administrator on 2021/9/24.
//

#ifndef UNTITLED1_PACK_PACK_H_
#define UNTITLED1_PACK_PACK_H_

#include <vector>
#include <memory>
#include <set>

#include "../common/File.h"
#include "../common/Stream.h"

const int MAX_PATH_LENGTH_W = MAX_PATH / 2;

struct PackedFileHeader
{
  std::wstring toString() const;

  wchar_t name[MAX_PATH_LENGTH_W];
  file::Type type;
  union
  {
    size_t size;
    size_t targetOffset;
  };
  union
  {
    size_t dataOffset;
    wchar_t linkTo[MAX_PATH_LENGTH_W];
  };
  DWORD attribute;
  DWORD volumeNumber;
  FILETIME timeCreation;
  FILETIME timeLastAccess;
  FILETIME timeLastWrite;
};

Stream packFile(const std::vector<std::wstring> &filePaths);

void traverseDir(const std::wstring &path);

#endif //UNTITLED1_PACK_PACK_H_
