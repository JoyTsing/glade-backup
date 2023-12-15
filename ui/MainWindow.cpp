//
// Created by Administrator on 2021/9/27.
//

#include "MainWindow.h"

#include <iostream>
#include <codecvt>

bool checkPasswordValidity(const std::string &password)
{
  auto len = password.length();
  return (len > 0) && (len < aes::BLOCK_SIZE);
}

std::wstring utf8ToUtf16(const std::string& utf8Str)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
  return conv.from_bytes(utf8Str);
}

std::string utf16ToUtf8(const std::wstring& utf16Str)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
  return conv.to_bytes(utf16Str);
}

MainWindow::MainWindow(Glib::RefPtr<Gtk::Application> app) :
    app(std::move(app))
{
  auto builder = Gtk::Builder::create();
  builder->add_from_file("mainUI.glade");

  builder->get_widget("window", window);
  builder->get_widget("upload", backupFileButton);
  builder->get_widget("uploadDir", backupDirButton);
  builder->get_widget("check", verifyButton);
  builder->get_widget("recover", recoverButton);
  builder->get_widget("delete", deleteButton);
  builder->get_widget("encrypt", encryptChecker);
  builder->get_widget("compress", compressChecker);
  builder->get_widget("scrolledWindow", scrolledWindow);
  builder->get_widget("passwordEncrypt", passwordEnc);
  builder->get_widget("passwordDecrypt", passwordDec);
  builder->get_widget("file_tree", fileTree);

  backupFileButton->signal_clicked().connect(
      [this]() { onBackupFileButtonClicked(); }
  );
  backupDirButton->signal_clicked().connect(
      [this]() { onBackupDirButtonClicked(); }
  );
  verifyButton->signal_clicked().connect(
      [this]() { onVerifyButtonClicked(); }
  );
  recoverButton->signal_clicked().connect(
      [this]() { onRecoverButtonClicked(); }
  );
  deleteButton->signal_clicked().connect(
      [this]() { onDeleteButtonClicked(); }
  );

  pswEncBuffer = Gtk::TextBuffer::create();
  pswDecBuffer = Gtk::TextBuffer::create();

  passwordEnc->set_buffer(pswEncBuffer);
  passwordDec->set_buffer(pswDecBuffer);

  scrolledWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  treeModel = Gtk::ListStore::create(columns);
  fileTree->set_model(treeModel);
  refreshList();
  //scrolledWindow->add(*fileTree);
}

MainWindow::~MainWindow()
{
}

int MainWindow::run()
{
  window->set_resizable(false);
  window->show_all();
  return app->run(*window);
}

void MainWindow::onBackupFileButtonClicked()
{
  auto dialog = Gtk::FileChooserDialog(
      "Choose a file to backup", Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for(*window);
  dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("Select", Gtk::RESPONSE_OK);

  if (dialog.run() != Gtk::RESPONSE_OK)
  {
    dialog.hide();
    return;
  }

  auto path = file::getSymbLinkPath(utf8ToUtf16(dialog.get_filename()));
  std::wstring name = L"Backup_" + std::to_wstring(util::getTime());
  std::string password(pswEncBuffer->get_text());
  const std::string *key = encryptChecker->get_active() ?
      &password : nullptr;
  if (key != nullptr && !checkPasswordValidity(password))
  {
    auto dial = Gtk::MessageDialog("Password length should be 1 to 16");
    dial.run();
    return;
  }
  backup(path, name, compressChecker->get_active(), false, key);
  dialog.hide();
  pswEncBuffer->set_text("");
  refreshList();
}

void MainWindow::onBackupDirButtonClicked()
{
  auto dialog = Gtk::FileChooserDialog(
      "Choose a folder to backup", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  dialog.set_transient_for(*window);
  dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("Select", Gtk::RESPONSE_OK);

  if (dialog.run() != Gtk::RESPONSE_OK)
  {
    dialog.hide();
    return;
  }
  auto path = file::getSymbLinkPath(utf8ToUtf16(dialog.get_filename()));
  std::wstring name = L"Backup_" + std::to_wstring(util::getTime());
  std::string password(pswEncBuffer->get_text());
  const std::string *key = encryptChecker->get_active() ?
                           &password : nullptr;
  if (key != nullptr && !checkPasswordValidity(password))
  {
    auto dial = Gtk::MessageDialog("Password length should be 1 to 16");
    dial.run();
    return;
  }
  backup(path, name, compressChecker->get_active(), true, key);
  dialog.hide();
  pswEncBuffer->set_text("");
  refreshList();
}

void MainWindow::onVerifyButtonClicked()
{
  auto name = getSelectedItemName();
  if (!name)
  {
    auto dialog = Gtk::MessageDialog("No project to verify selected");
    dialog.run();
    return;
  }
  std::string password(pswDecBuffer->get_text());
  pswDecBuffer->set_text("");
  if (!verify(name.value(), &password))
  {
    auto dialog = Gtk::MessageDialog("Incorrect or wrong password");
    dialog.run();
  }
  else
  {
    auto dialog = Gtk::MessageDialog("Correct");
    dialog.run();
  }
}

void MainWindow::onRecoverButtonClicked()
{
  auto index = getSelectedItemName();
  if (!index)
    return;
  auto dialog = Gtk::FileChooserDialog(
      "Choose target directory to recover", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  dialog.set_transient_for(*window);
  dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("Select", Gtk::RESPONSE_OK);

  if (dialog.run() != Gtk::RESPONSE_OK)
  {
    dialog.hide();
    return;
  }
  auto path = file::getSymbLinkPath(utf8ToUtf16(dialog.get_filename()));
  std::string password(pswDecBuffer->get_text());
  pswDecBuffer->set_text("");
  if (!recover(index.value(), path, &password))
  {
    auto dFail = Gtk::MessageDialog("Recovery failed or incorrect password");
    dFail.run();
  }
  else
  {
    auto dSuccess = Gtk::MessageDialog("Recovery success");
    dSuccess.run();
  }
  dialog.hide();
}

void MainWindow::onDeleteButtonClicked()
{
  auto index = getSelectedItemName();
  if (!index)
    return;
  remove(index.value());
  refreshList();
}

std::optional<std::wstring> MainWindow::getSelectedItemName()
{
  auto selection = fileTree->get_selection();
  auto item = selection->get_selected();
  if (!item)
    return std::nullopt;

  Glib::ustring name;
  item->get_value(0, name);

  return utf8ToUtf16(std::string(name.c_str()));
}

void MainWindow::refreshList()
{
  auto recList = getBackupRecordList();
  treeModel->clear();
  for (const auto &rec : recList)
  {
    auto ref = *treeModel->append();
    ref[columns.filename] = utf16ToUtf8(rec.name);
    ref[columns.type] = rec.isDirectory ?
        "Directory" : "Single file";
    ref[columns.encrypted] = rec.encrypted ?
        "True" : "False";
    ref[columns.compressed] = rec.compressed ?
        "True" : "False";
    ref[columns.lastTime] = utf16ToUtf8(file::fileTimeToString(rec.backupTime));
    ref[columns.sourcePath] = utf16ToUtf8(rec.source);
    std::wcout << rec.name << " " << rec.source << "\n";
  }
}