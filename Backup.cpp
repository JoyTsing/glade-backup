//
// Created by Administrator on 2021/9/25.
//

#include "Backup.h"

#include <algorithm>

using CompressKeyType = uint8_t;

const std::wstring BACKUP_PATH = L"test\\backup";
const std::wstring CONFIG_FILENAME = L"config";
const std::wstring BACKUP_FILENAME = L"content.bak";

std::vector<BackupRecord> recordCache;

bool BackupRecord::operator < (const BackupRecord &rhs) const
{
  return util::toQword(backupTime) > util::toQword(rhs.backupTime);
}

void createBackupDir()
{
  auto file = file::open(BACKUP_PATH);
  if (!file)
    file::createFile(BACKUP_PATH);
}

std::wstring getBackupDir(const std::wstring &name)
{
  return BACKUP_PATH + L'\\' + name;
}

std::wstring getBackupConfigPath(const std::wstring &name)
{
  return getBackupDir(name) + L'\\' + CONFIG_FILENAME;
}

std::wstring getBackupFilePath(const std::wstring &name)
{
  return getBackupDir(name) + L'\\' + BACKUP_FILENAME;
}

std::vector<BackupRecord> getBackupRecordList()
{
  std::vector<BackupRecord> recList;
  std::wstring exPath = BACKUP_PATH + L"\\*";
  auto [nextFile, data] = file::findWithData(exPath);
  while (FindNextFileW(nextFile.get(), &data))
  {
    if (!lstrcmpW(data.cFileName, L".."))
      continue;
    std::wstring path = getBackupConfigPath(std::wstring(data.cFileName));
    auto config = file::open(path);

    size_t size = file::getSize(config);
    Stream ss(size);
    ReadFile(config.get(), (LPVOID)ss.data(), size, nullptr, nullptr);
    BackupRecord rec;
    rec.backupTime = ss.to<FILETIME>();
    if (util::toQword(rec.backupTime) == 0)
      continue;
    rec.compressed = ss.to<bool>();
    rec.encrypted = ss.to<bool>();
    rec.isDirectory = ss.to<bool>();
    rec.source = std::wstring(reinterpret_cast<const wchar_t*>(ss.pointing()) + 4);
    rec.name = std::wstring(data.cFileName);
    recList.push_back(rec);
    config.release();
  }
  nextFile.release();
  std::sort(recList.begin(), recList.end());
  return recList;
}

void refreshRecordListCache()
{
  recordCache = getBackupRecordList();
}

Stream backup(const std::wstring &path, bool compress, const std::string *key)
{
  auto stream = packFile({ path });
  if (compress)
    stream = ::compress<CompressKeyType>(stream);
  if (key)
    stream = aes::encrypt(stream, *key);
  return stream;
}

bool backup(const std::wstring &path, const std::wstring &name, bool compress, bool isDirectory, const std::string *key)
{
  timer::init();
  CreateDirectoryW(getBackupDir(name).c_str(), nullptr);
  auto config = file::createFile(getBackupConfigPath(name));
  auto backupFile = file::createFile(getBackupFilePath(name));
  if (!config || !backupFile)
    return false;

  auto stream = backup(path, compress, key);
  Stream ss;
  SYSTEMTIME time;
  GetLocalTime(&time);
  ss.from(file::sysTimeToFileTime(time));
  ss.from(compress);
  ss.from(key != nullptr);
  ss.from(isDirectory);
  auto fullPath = file::getSymbLinkPath(path);
  ss.from(fullPath.data(), fullPath.length() * sizeof(wchar_t));
  ss.from(L'\0');

  WriteFile(config.get(), ss.data(), ss.size(), nullptr, nullptr);
  WriteFile(backupFile.get(), stream.data(), stream.size(), nullptr, nullptr);
  config.release();
  backupFile.release();
  refreshRecordListCache();
  std::cout << timer::getTimeString() << "\n";
  return true;
}

bool verify(Stream &stream, const std::wstring &origin)
{
  auto nHeaders = stream.to<size_t>();
  std::vector<PackedFileHeader> headers(nHeaders);
  stream.to(headers.data(), nHeaders * sizeof(PackedFileHeader));
  auto lenHierarchy = stream.to<size_t>();
  std::vector<uint8_t> hierarchy(lenHierarchy);
  stream.to(hierarchy.data(), lenHierarchy);
  Stream fileData(stream.size() - stream.getPtr());
  stream.to((void*)fileData.data(), stream.size() - stream.getPtr());

  for (const auto &header : headers)
    std::wcout << header.toString() << "\n";

  std::vector<std::wstring> fullPaths;
  std::stack<std::wstring> pathStack;
  pathStack.push(origin);

  size_t headerIndex = 0;
  for (size_t i = 1; i < lenHierarchy - 1; i++)
  {
    if (hierarchy[i] == '>')
    {
      pathStack.pop();
      continue;
    }
    const auto &h = headers[headerIndex];
    std::wstring curPath = pathStack.top();
    std::wstring filename = curPath + L"\\" + h.name;
    fullPaths.push_back(filename);
    std::wcout << filename << "\n";

    if (h.type == file::Type::Directory)
      pathStack.push(filename);
    else if (h.type == file::Type::Other)
      return false;
    else
      i++;

    auto file = file::openReadOnly(filename);
    if (!file)
      return false;
    auto info = file::getInfo(file);

    if (h.type == file::Type::Regular ||
        h.type == file::Type::HardLink)
    {
      auto size = (h.type == file::Type::HardLink) ?
          headers[h.targetOffset].size : h.size;
      if (util::toQword(info.nFileSizeHigh, info.nFileSizeLow) != size)
        return false;
    }
    headerIndex++;
  }
  return true;
}

bool verify(const std::wstring &path, const std::string *key)
{
  auto conf = file::open(getBackupConfigPath(path));
  auto file = file::open(getBackupFilePath(path));

  if (!conf || !file)
    return false;

  size_t fileSize = file::getSize(file);
  Stream ssFile(fileSize);
  ReadFile(file.get(), (LPVOID)ssFile.data(), fileSize, nullptr, nullptr);

  size_t confSize = file::getSize(conf);
  Stream ssConf(confSize);
  ReadFile(conf.get(), (LPVOID)ssConf.data(), confSize, nullptr, nullptr);
  ssConf.to<FILETIME>();
  auto compressed = ssConf.to<bool>();
  auto encrypted = ssConf.to<bool>();
  ssConf.to<bool>();
  std::wstring origin(reinterpret_cast<const wchar_t*>(ssConf.pointing()));
  for (size_t i = origin.size() - 1; i >= 0; i--)
  {
    if (origin[i] == L'\\')
    {
      origin.resize(i);
      break;
    }
  }
  auto pKey = encrypted ? key : nullptr;
  if (encrypted && !pKey)
    return false;
  auto res = recover(ssFile, compressed, pKey);
  if (!res)
    return false;
  auto stream = res.value();
  return verify(stream, origin);
}

std::optional<Stream> recover(Stream stream, bool decompress, const std::string *key)
{
  if (key)
  {
    auto dec = aes::decrypt(stream, *key);
    if (!dec)
      return std::nullopt;
    stream = dec.value();
  }
  if (decompress)
    stream = ::decompress<CompressKeyType>(stream);
  return stream;
}

bool recover(Stream& stream, const std::wstring &path)
{
  std::cout << "<<Recovery directory>>\n";
  std::wcout << path << "\n";
  auto nHeaders = stream.to<size_t>();
  std::vector<PackedFileHeader> headers(nHeaders);
  stream.to(headers.data(), nHeaders * sizeof(PackedFileHeader));
  auto lenHierarchy = stream.to<size_t>();
  std::vector<uint8_t> hierarchy(lenHierarchy);
  stream.to(hierarchy.data(), lenHierarchy);
  Stream fileData(stream.size() - stream.getPtr());
  stream.to((void*)fileData.data(), stream.size() - stream.getPtr());

  for (const auto &header : headers)
    std::wcout << header.toString() << "\n";
  for (auto c : hierarchy)
    std::cout << c;
  std::cout << "\n";
//  std::wcout << fileData.toHexString() << "\n";

  std::vector<std::wstring> fullPaths;
  std::stack<std::wstring> pathStack;
  pathStack.push(path);

  size_t headerIndex = 0;
  for (size_t i = 1; i < lenHierarchy - 1; i++)
  {
    if (hierarchy[i] == '>')
    {
      pathStack.pop();
      continue;
    }
    const auto &h = headers[headerIndex];
    std::wstring curPath = pathStack.top();
    std::wstring filename = curPath + L"\\" + h.name;
    fullPaths.push_back(filename);
    std::wcout << filename << "\n";

    if (h.type == file::Type::Directory)
    {
      CreateDirectoryW(filename.c_str(), nullptr);
      pathStack.push(filename);
    }
    else if (h.type == file::Type::Regular)
    {
      auto file = file::createFile(filename);
      WriteFile(file.get(), fileData.data() + h.dataOffset, h.size, nullptr, nullptr);
      i++;
    }
    else if (h.type == file::Type::HardLink)
    {
      CreateHardLinkW(filename.c_str(), fullPaths[h.targetOffset].c_str(), nullptr);
      i++;
    }
    else if (h.type == file::Type::SymbLink)
    {
      //CreateSymbolicLinkW(filename.c_str(), fullPaths[h.targetOffset].c_str(), SYMBOLIC_LINK_FLAG_DIRECTORY);
      i++;
    }
    else
      return false;
    auto file = file::open(filename, false);
    SetFileTime(file.get(), &h.timeCreation, &h.timeLastAccess, &h.timeLastWrite);
    SetFileAttributesW(filename.c_str(), h.attribute);
    headerIndex++;
  }

  for (int i = 0; i < nHeaders; i++)
  {
    const auto &h = headers[i];
    if (h.type != file::Type::SymbLink)
      continue;
    if (fullPaths[i].find(path) != std::wstring::npos)
      CreateSymbolicLinkW(fullPaths[i].c_str(), fullPaths[h.targetOffset].c_str(), SYMBOLIC_LINK_FLAG_DIRECTORY);
    else
      CreateSymbolicLinkW(fullPaths[i].c_str(), h.linkTo, SYMBOLIC_LINK_FLAG_DIRECTORY);
  }
  // 不知道为什么文件夹需要额外设置一次时间才行
  for (int i = 0; i < nHeaders; i++)
  {
    const auto &h = headers[i];
    if (h.type != file::Type::Directory)
      continue;
    SetFileTime(file::open(fullPaths[i]).get(), &h.timeCreation, &h.timeLastAccess, &h.timeLastWrite);
  }
  std::cout << timer::getTimeString() << "\n";
  return true;
}

bool recover(const std::wstring &name, const std::wstring &dest, const std::string *key)
{
  timer::init();
  auto conf = file::open(getBackupConfigPath(name));
  auto file = file::open(getBackupFilePath(name));

  size_t fileSize = file::getSize(file);
  Stream ssFile(fileSize);
  ReadFile(file.get(), (LPVOID)ssFile.data(), fileSize, nullptr, nullptr);

  size_t confSize = sizeof(FILETIME) + sizeof(bool) * 2;
  Stream ssConf(confSize);
  ReadFile(conf.get(), (LPVOID)ssConf.data(), confSize, nullptr, nullptr);
  ssConf.to<FILETIME>();
  auto compressed = ssConf.to<bool>();
  auto encrypted = ssConf.to<bool>();

  auto pKey = encrypted ? key : nullptr;
  if (encrypted && !pKey)
    return false;
  auto res = recover(ssFile, compressed, pKey);
  if (!res)
    return false;
  auto stream = res.value();
//  std::wcout << stream.toHexString() << "\n";
  return recover(stream, file::getSymbLinkPath(dest));
}

bool remove(const std::wstring &name)
{
  auto file = file::open(getBackupConfigPath(name));
  if (!file)
    return false;
  SetFilePointer(file.get(), 0, nullptr, FILE_BEGIN);
  uint64_t time = 0;
  WriteFile(file.get(), &time, sizeof(uint64_t), nullptr, nullptr);
  return true;
}