//
// Created by Administrator on 2021/9/24.
//

#include "Pack.h"

#include <set>
#include <map>

std::wstring PackedFileHeader::toString() const
{
  std::wstringstream ss;
  ss << "Name\t" << name << "\n";
  ss << "Type\t" << file::typeToString(type) << "\n";
  if (type == file::Type::Regular)
  {
    ss << "Size\t" << size << "\n";
    ss << "Offset\t" << dataOffset << "\n";
  }
  else if (type == file::Type::SymbLink)
  {
    ss << "Link\t" << linkTo << "\n";
    ss << "Target\t" << targetOffset << "\n";
  }
  else if (type == file::Type::HardLink)
    ss << "Target\t" << targetOffset << "\n";
  ss << "Attrib\t" << toBitString(attribute) << "\n";
  ss << "Volume\t" << volumeNumber << "\n";
  ss << "Create\t" << file::fileTimeToString(timeCreation) << "\n";
  ss << "Access\t" << file::fileTimeToString(timeLastAccess) << "\n";
  ss << "Write\t" << file::fileTimeToString(timeLastWrite) << "\n";
  return ss.str();
}

void getPackedHeadersAndHierarchy(
    const std::wstring &path,
    std::vector<PackedFileHeader> &headers,
    std::stringstream &hierarchy,
    std::map<uint64_t, size_t> &indexMap)
{
  std::wcout << "##" << path << "\n";
  auto file = file::openReadOnly(path);
  if (!file)
    return;
  hierarchy << '<';
  PackedFileHeader header{};
  auto truncatedName = file::getTruncatedPath(path);
  auto type = file::getType(path);
  auto info = file::getInfo(file);
  size_t size = util::toQword(info.nFileSizeHigh, info.nFileSizeLow);
  uint64_t index = util::toQword(info.nFileIndexHigh, info.nFileIndexLow);

  auto finalFile = file::openFinalReadOnly(path);
  auto finalInfo = file::getInfo(finalFile);
  uint64_t finalIndex = util::toQword(finalInfo.nFileIndexHigh, finalInfo.nFileIndexLow);

  lstrcpyW(header.name, truncatedName.c_str());
  header.attribute = info.dwFileAttributes;
  header.volumeNumber = info.dwVolumeSerialNumber;
  header.timeCreation = info.ftCreationTime;
  header.timeLastAccess = info.ftLastAccessTime;
  header.timeLastWrite = info.ftLastWriteTime;
  header.type = type;

  if (type == file::Type::Regular)
    header.size = size;
  else if (type == file::Type::HardLink)
  {
    if (indexMap.find(index) == indexMap.end())
    {
      header.type = file::Type::Regular;
      header.size = size;
    }
  }
  else if (type == file::Type::SymbLink)
  {
    auto finalPath = file::getSymbLinkPath(finalFile);
    lstrcpyW(header.linkTo, finalPath.c_str());
  }
  if (finalIndex != index)
    indexMap[index] = finalIndex;
  else
  {
    if (indexMap.find(index) == indexMap.end())
      indexMap[index] = headers.size();
  }
  headers.push_back(header);

  if (type != file::Type::Directory)
  {
    hierarchy << '>';
    return;
  }
  std::wstring exPath = (path.find(L'*') == std::string::npos) ?
                        path + L"\\*" : path;
  //std::wcout << "##" << exPath << "\n";
  auto [nextFile, data] = file::findWithData(exPath);
  while (FindNextFileW(nextFile.get(), &data))
  {
    if (!lstrcmpW(data.cFileName, L".."))
      continue;
    std::wstring subPath = path + L"\\" + data.cFileName;
    getPackedHeadersAndHierarchy(subPath, headers, hierarchy, indexMap);
  }
  hierarchy << '>';
}

std::string getPackedHeadersAndHierarchy(
    const std::vector<std::wstring> &paths,
    std::vector<PackedFileHeader> &headers,
    std::map<uint64_t, size_t> &indexMap)
{
  std::stringstream hierarchy;
  hierarchy << '<';
  for (const auto &path : paths)
    getPackedHeadersAndHierarchy(path, headers, hierarchy, indexMap);
  hierarchy << '>';
  return hierarchy.str();
}

void getIndicesAndFileData(
    const std::wstring &path,
    const std::map<uint64_t, size_t> &indexMap,
    std::vector<PackedFileHeader> &headers,
    Stream &fileData,
    size_t &headerPtr)
{
  auto file = file::openFinalReadOnly(path);
  if (!file)
    return;
  auto type = file::getType(path);
  auto info = file::getInfo(file);
  uint64_t index = util::toQword(info.nFileIndexHigh, info.nFileIndexLow);
  auto finalIndex = indexMap.find(index)->second;

  auto &header = headers[headerPtr];
  if (header.type == file::Type::HardLink || header.type == file::Type::SymbLink)
    header.targetOffset = finalIndex;
  else if (header.type == file::Type::Regular)
  {
    header.dataOffset = fileData.size();
    auto tmpData = new uint8_t[header.size];
    ReadFile(file.get(), tmpData, header.size, nullptr, nullptr);
    fileData.from(tmpData, header.size);
    delete[] tmpData;
  }
  headerPtr++;

  if (type != file::Type::Directory)
    return;
  std::wstring exPath = (path.find(L'*') == std::string::npos) ?
                        path + L"\\*" : path;
  auto [nextFile, data] = file::findWithData(exPath);
  while (FindNextFileW(nextFile.get(), &data))
  {
    if (!lstrcmpW(data.cFileName, L".."))
      continue;
    std::wstring subPath = path + L"\\" + data.cFileName;
    getIndicesAndFileData(subPath, indexMap, headers, fileData, headerPtr);
  }
}

Stream packFile(const std::vector<std::wstring> &filePaths)
{
  Stream stream;
  std::vector<PackedFileHeader> headers;
  Stream fileData;
  std::map<uint64_t, size_t> indexMap;

  auto hierarchy = getPackedHeadersAndHierarchy(filePaths, headers, indexMap);
  size_t headerPtr = 0;
  for (const auto &path : filePaths)
    getIndicesAndFileData(path, indexMap, headers, fileData, headerPtr);
  for (const auto &h : headers)
    std::wcout << h.toString() << "\n";
  std::cout << hierarchy << "\n";

  stream.from(headers.size());
  stream.from(headers.data(), headers.size() * sizeof(PackedFileHeader));
  stream.from(hierarchy.length());
  stream.from(hierarchy.data(), hierarchy.length());
  stream.fromStream(fileData);
  return stream;
}

void traverseDir(const std::wstring &path, std::set<uint64_t> &indexRec)
{
  std::wcout << "<<" << path << ">>\n";
  auto file = file::openReadOnly(path.c_str());
  std::wcout << file::infoToString(file::getInfo(file)) << "\n";

  auto type = file::getType(path.c_str());
  if (type == file::Type::Regular || type == file::Type::HardLink)
    return;
  if (type == file::Type::SymbLink)
  {
    auto finalPath = file::getSymbLinkPath(file::openFinalReadOnly(path.c_str()));
    std::wcout << "\t--> " << finalPath << "\n";
  }
  std::wstring exPath = (path.find(L'*') == std::string::npos) ?
                        path + L"\\" : path;
  auto [nextFile, data] = file::findWithData(exPath);
  while (FindNextFileW(nextFile.get(), &data))
  {
    if (!lstrcmpW(data.cFileName, L".."))
      continue;
    std::wstring subPath = path + L"\\" + data.cFileName;
    traverseDir(subPath);
  }
}

void traverseDir(const std::wstring &path)
{
  std::set<uint64_t> indexRec;
  traverseDir(path, indexRec);
}
