//
// Created by Administrator on 2021/9/27.
//

#ifndef UNTITLED1_UI_MAINWINDOW_H_
#define UNTITLED1_UI_MAINWINDOW_H_

#include <gtkmm.h>
#include <fstream>

#include "../Backup.h"

class MainWindow
{
 public:
  explicit MainWindow(Glib::RefPtr<Gtk::Application> app);
  virtual ~MainWindow();

  int run();

 protected:
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
   public:
    ModelColumns()
    {
      add(filename);
      add(type);
      add(encrypted);
      add(compressed);
      add(lastTime);
      add(sourcePath);
    }

    Gtk::TreeModelColumn<Glib::ustring> filename;
    Gtk::TreeModelColumn<Glib::ustring> type;
    Gtk::TreeModelColumn<Glib::ustring> encrypted;
    Gtk::TreeModelColumn<Glib::ustring> compressed;
    Gtk::TreeModelColumn<Glib::ustring> lastTime;
    Gtk::TreeModelColumn<Glib::ustring> sourcePath;
  };

 protected:
  void onBackupFileButtonClicked();
  void onBackupDirButtonClicked();
  void onVerifyButtonClicked();
  void onRecoverButtonClicked();
  void onDeleteButtonClicked();

 private:
  std::optional<std::wstring> getSelectedItemName();
  void refreshList();

 private:
  Glib::RefPtr<Gtk::Application> app;
  Gtk::Window *window = nullptr;

  Gtk::Button *backupFileButton = nullptr;
  Gtk::Button *backupDirButton = nullptr;
  Gtk::Button *verifyButton = nullptr;
  Gtk::Button *recoverButton = nullptr;
  Gtk::Button *deleteButton = nullptr;
  Gtk::CheckButton *encryptChecker = nullptr;
  Gtk::CheckButton *compressChecker = nullptr;
  Gtk::ScrolledWindow *scrolledWindow = nullptr;
  Gtk::TreeView *fileTree = nullptr;

  Gtk::TextView *passwordEnc;
  Gtk::TextView *passwordDec;

  Glib::RefPtr<Gtk::TextBuffer> pswEncBuffer;
  Glib::RefPtr<Gtk::TextBuffer> pswDecBuffer;

  Glib::RefPtr<Gtk::ListStore> treeModel;
  ModelColumns columns;
};

#endif //UNTITLED1_UI_MAINWINDOW_H_
