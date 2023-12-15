//
// Created by Administrator on 2021/9/24.
//

#include "File.h"

NAMESPACE_BEGIN(file)

std::pair<SearchHandle, WIN32_FIND_DATAW> findWithData(const wchar_t *filePath)
{
  WIN32_FIND_DATAW info;
  auto file = FindFirstFileW(filePath, &info);
  return std::pair<SearchHandle, WIN32_FIND_DATAW>(file, info);
}

std::pair<SearchHandle, WIN32_FIND_DATAW> findWithData(const std::wstring &filePath)
{
  return findWithData(filePath.c_str());
}

SearchHandle find(const wchar_t *filePath)
{
  WIN32_FIND_DATAW info;
  return SearchHandle(FindFirstFileW(filePath, &info));
}

SearchHandle find(const std::wstring &filePath)
{
  return find(filePath.c_str());
}

ContentHandle open(const wchar_t *filePath, bool toFinalPath)
{
  auto attrib = FILE_FLAG_BACKUP_SEMANTICS;
  if (!toFinalPath)
    attrib |= FILE_FLAG_OPEN_REPARSE_POINT;
  auto handle = CreateFileW(
      filePath, GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
      OPEN_EXISTING, attrib, nullptr);
  return ContentHandle(handle);
}

ContentHandle open(const std::wstring &filePath, bool toFinalPath)
{
  return open(filePath.c_str(), toFinalPath);
}

ContentHandle openReadOnly(const std::wstring &filePath)
{
  auto attrib = FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT;
  auto handle = CreateFileW(
      filePath.c_str(), GENERIC_READ,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
      OPEN_EXISTING, attrib, nullptr);
  return ContentHandle(handle);
}

ContentHandle openFinalReadOnly(const std::wstring &filePath)
{
  auto attrib = FILE_FLAG_BACKUP_SEMANTICS;
  auto handle = CreateFileW(
      filePath.c_str(), GENERIC_READ,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
      OPEN_EXISTING, attrib, nullptr);
  return ContentHandle(handle);
}

ContentHandle createFile(const wchar_t *filePath)
{
  auto handle = CreateFileW(
      filePath, GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
      OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
      );
  return ContentHandle(handle);
}

ContentHandle createFile(const std::wstring &filePath)
{
  return createFile(filePath.c_str());
}

BY_HANDLE_FILE_INFORMATION getInfo(const ContentHandle &file)
{
  BY_HANDLE_FILE_INFORMATION info;
  GetFileInformationByHandle(file.get(), &info);
  return info;
}

BY_HANDLE_FILE_INFORMATION getInfo(const wchar_t *filePath)
{
  return getInfo(open(filePath));
}

BY_HANDLE_FILE_INFORMATION getInfo(const std::wstring &filePath)
{
  return getInfo(open(filePath));
}

Type getType(const wchar_t *filePath)
{
  auto hLocal = openReadOnly(filePath);
  if (!hLocal)
    return Type::Other;

  auto attrib = GetFileAttributesW(filePath);
  if (util::testBit(attrib, FILE_ATTRIBUTE_DIRECTORY))
  {
    auto hFinal = openFinalReadOnly(filePath);
    if (!hFinal || !hLocal)
      return Type::Other;
    auto indexFinal = getIndex(hFinal);
    auto indexLocal = getIndex(hLocal);
    return (indexFinal == indexLocal) ? Type::Directory : Type::SymbLink;
  }
  return (getInfo(hLocal).nNumberOfLinks > 1) ? Type::HardLink : Type::Regular;
}

Type getType(const std::wstring &filePath)
{
  return getType(filePath.c_str());
}

uint64_t getIndex(const ContentHandle &file)
{
  auto info = getInfo(file);
  return util::toQword(info.nFileIndexHigh, info.nFileIndexLow);
}

uint64_t getIndex(const wchar_t *filePath)
{
  auto info = getInfo(filePath);
  return util::toQword(info.nFileIndexHigh, info.nFileIndexLow);
}

uint64_t getIndex(const std::wstring &filePath)
{
  return getIndex(filePath.c_str());
}

size_t getSize(const ContentHandle& file)
{
  DWORD high, low;
  low = GetFileSize(file.get(), &high);
  return util::toQword(high, low);
}

size_t getSize(const wchar_t *filePath)
{
  auto file = open(filePath);
  if (!file)
    return 0;
  return getSize(file);
}

size_t getSize(const std::wstring &filePath)
{
  return getSize(filePath.c_str());
}

std::wstring getSymbLinkPath(const ContentHandle &file)
{
  size_t requiredSize = GetFinalPathNameByHandleW(file.get(), nullptr, 0, FILE_NAME_NORMALIZED);
  std::wstring str(requiredSize - 1, L'\000');
  GetFinalPathNameByHandleW(file.get(), str.data(), requiredSize, FILE_NAME_NORMALIZED);
  return str;
}

std::wstring getSymbLinkPath(const wchar_t *filePath)
{
  auto file = open(filePath);
  if (!file)
    return L"";
  return getSymbLinkPath(file);
}

std::wstring getSymbLinkPath(const std::wstring &filePath)
{
  return getSymbLinkPath(filePath.c_str());
}

std::wstring getTruncatedPath(const wchar_t *filePath)
{
  return { findWithData(filePath).second.cFileName };
}

std::wstring getTruncatedPath(const std::wstring &filePath)
{
  return { findWithData(filePath).second.cFileName };
}

std::wstring infoToString(const BY_HANDLE_FILE_INFORMATION &info)
{
  std::wstringstream ss;
  ss << "Attrib\t" << toBitString(info.dwFileAttributes) << "\n";
  ss << "Volume\t" << info.dwVolumeSerialNumber << "\n";
  ss << "Index\t" << util::toQword(info.nFileIndexHigh, info.nFileIndexLow) << "\n";
  ss << "Size\t" << util::toQword(info.nFileSizeHigh, info.nFileSizeLow) << "\n";
  ss << "Create\t" << util::toQword(info.ftCreationTime) << "\n";
  ss << "\t" << fileTimeToString(info.ftCreationTime) << "\n";
  ss << "Access\t" << util::toQword(info.ftLastAccessTime) << "\n";
  ss << "\t" << fileTimeToString(info.ftLastAccessTime) << "\n";
  ss << "Write\t" << util::toQword(info.ftLastWriteTime) << "\n";
  ss << "\t" << fileTimeToString(info.ftLastWriteTime) << "\n";
  ss << "Links\t" << info.nNumberOfLinks << "\n";
  return ss.str();
}

std::wstring infoToString(const WIN32_FIND_DATAW &info)
{
  std::wstringstream ss;
  ss << "Name\t" << info.cFileName << "\n";
  ss << "AltName\t" << info.cAlternateFileName << "\n";
  ss << "Size\t" << util::toQword(info.nFileSizeHigh, info.nFileSizeLow) << "\n";
  ss << "Create\t" << util::toQword(info.ftCreationTime) << "\n";
  ss << "\t" << fileTimeToString(info.ftCreationTime) << "\n";
  ss << "Access\t" << util::toQword(info.ftLastAccessTime) << "\n";
  ss << "\t" << fileTimeToString(info.ftLastAccessTime) << "\n";
  ss << "Write\t" << util::toQword(info.ftLastWriteTime) << "\n";
  ss << "Access\t" << util::toQword(info.ftLastWriteTime) << "\n";
  ss << "Attrib\t" << toBitString(info.dwFileAttributes) << "\n";
  return ss.str();
}

std::wstring typeToString(const Type &type)
{
  switch (type)
  {
    case Type::Regular:
      return L"Single regular file";
    case Type::Directory:
      return L"Directory";
    case Type::HardLink:
      return L"Multi-linked regular file";
    case Type::SymbLink:
      return L"Symbolic link";
    default:
      break;
  }
  return L"Unknown type";
}

FILETIME sysTimeToFileTime(const SYSTEMTIME &time)
{
  FILETIME f;
  SystemTimeToFileTime(&time, &f);
  return f;
}

SYSTEMTIME fileTimeToSysTime(const FILETIME &time)
{
  SYSTEMTIME s;
  FileTimeToSystemTime(&time, &s);
  return s;
}

std::wstring sysTimeToString(const SYSTEMTIME &time)
{
  std::wstringstream ss;
  ss << time.wYear << "-" << time.wMonth << "-" << time.wDay << " ";
  ss << time.wHour << ":" << time.wMinute << ":" << time.wSecond;
  //ss << "." << time.wMilliseconds;
  return ss.str();
}

std::wstring fileTimeToString(const FILETIME &time)
{
  return sysTimeToString(fileTimeToSysTime(time));
}

uint64_t hash(const ContentHandle &file)
{
  auto size = getSize(file);
  std::cout << size << "\n";
  auto data = new uint8_t[size];
  ReadFile(file.get(), data, size, nullptr, nullptr);
  auto res = util::hash(data, size);
  delete[] data;
  return res;
}

NAMESPACE_END