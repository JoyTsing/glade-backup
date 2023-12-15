#include <gtkmm/application.h>

#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
  auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");
  MainWindow window(app);
  return window.run();
}
