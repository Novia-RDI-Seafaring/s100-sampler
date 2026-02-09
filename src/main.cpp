#include <iostream>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <chrono>
#include "MainWindow.hpp"
// #include <chrono>
#ifdef _WIN32
#  include <windows.h>
#endif
#include <curl/curl.h>
#include <QStyleFactory>

static const double DPI = 92.0;

void setDarkPalette(QApplication &app)
{
  QPalette darkPalette;

  darkPalette.setColor(QPalette::Window, QColor(40, 40, 40));
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
  darkPalette.setColor(QPalette::AlternateBase, QColor(45, 45, 45));
  darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
  darkPalette.setColor(QPalette::ToolTipText, Qt::white);
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::Button, QColor(45, 45, 45));
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

  darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::HighlightedText, Qt::white);

  darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(100, 100, 100));
  darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(100, 100, 100));
  darkPalette.setColor(QPalette::Disabled, QPalette::Button, QColor(60, 60, 60));
  darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(60, 60, 60));
  darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
  darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(150, 150, 150));
  darkPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(60, 60, 60));
  // darkPalette.setColor(QPalette::Disabled, QPalette::Background, QColor(120, 120, 120));

  app.setPalette(darkPalette);
}

int main(int argc, char **argv)
{

  curl_version_info_data *info = curl_version_info(CURLVERSION_NOW);
  std::cout << "cURL Version: " << info->version << std::endl;
  std::cout << "SSL Backend: " << (info->ssl_version ? info->ssl_version : "None") << std::endl;
  std::cout << "Protocols: ";

  for (const char *const *proto = info->protocols; *proto; ++proto)
  {
    std::cout << *proto << " ";
  }
  std::cout << std::endl;
  ConfigManager::getInstance().LoadConfig("../config.toml");

  // Access the loaded configuration
  const Config &config = ConfigManager::getInstance().getConfig();

  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);

  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
  QCoreApplication::setOrganizationName("Novia");
  QCoreApplication::setApplicationName("NOVIA ECS");
  QApplication::setApplicationDisplayName("NOVIA ECS");
  QApplication app(argc, argv);

  app.setStyle(QStyleFactory::create("Fusion"));
  setDarkPalette(app);
  qDebug() << "Application is running";

  MainWindow mainWindow;

  mainWindow.show();

  app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

  return 1;
}
