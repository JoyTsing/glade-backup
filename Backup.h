//
// Created by Administrator on 2021/9/25.
//

#ifndef UNTITLED1__BACKUP_H_
#define UNTITLED1__BACKUP_H_

#include "aes/AES.h"
#include "compress/Compress.h"
#include "pack/Pack.h"

struct BackupRecord
{
  bool operator < (const BackupRecord &rhs) const;

  std::wstring name;
  bool isDirectory;
  bool compressed;
  bool encrypted;
  FILETIME backupTime;
  std::wstring source;
};

void createBackupDir();

std::vector<BackupRecord> getBackupRecordList();
void refreshRecordListCache();

Stream backup(const std::wstring &path, bool compress, const std::string *key);
bool backup(const std::wstring &path, const std::wstring &name, bool compress, bool isDirectory, const std::string *key);

bool verify(const std::wstring &path, const std::string *key);

std::optional<Stream> recover(Stream stream, bool decompress, const std::string *key);
bool recover(const std::wstring &name, const std::wstring &dest, const std::string *key);

bool remove(const std::wstring &name);

#endif //UNTITLED1__BACKUP_H_
