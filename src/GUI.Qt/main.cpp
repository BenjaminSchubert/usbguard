//
// Copyright (C) 2015 Red Hat, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Authors: Daniel Kopecek <dkopecek@redhat.com>
//
#ifdef HAVE_BUILD_CONFIG_H
#include <build-config.h>
#endif

#include "MainWindow.h"
#include "SessionBlocker.h"
#include "SingleInstanceManager.h"

#include "usbguard/Logger.hpp"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QString>


/*
  This structure contains the command line parameters for the application.
*/
struct CommandLineParameters {
  bool background = false;
};


/*
  Parse the given arguments into a CommandLineParameters.

  If some arguments are invalid, this will print an error message and exit.

  @param argc Number of arguments.
  @param argv List of arguments.
  @return the parsed arguments.
*/
const CommandLineParameters parse_arguments(int argc, char *argv[]);

const CommandLineParameters parse_arguments(int argc, char *argv[]) {
  CommandLineParameters params;

  // we skip the first argument which is the name of the program
  for (int i = 1; i < argc; i++) {
    const char * arg = argv[i];

    if ((strcmp("-b", arg) == 0) || strcmp("--background", arg) == 0) {
      params.background = true;
    }
    else if ((strcmp("-h", arg) == 0) || strcmp("--help", arg) == 0) {
      qWarning() << "Usage:" << argv[0] << "[OPTIONS]";
      qWarning() << "";
      qWarning() << "Options:";
      qWarning() << "\t-h, --help\t\tShow this help message";
      qWarning() << "\t-b, --background\tLaunch the application in background";
      exit(0);
    }
    else {
      qWarning() << QString("Unrecognized option %1").arg(arg);
      qWarning() << QString("run %1 --help to see more information on how to run.").arg(argv[0]);
      exit(1);
    }
  }

  return params;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CommandLineParameters params = parse_arguments(argc, argv);
    QTranslator translator;

    USBGUARD_LOG(Debug) << "Loading translations for locale: "
                        << QLocale::system().name().toStdString();

    if (translator.load(QLocale::system(),
                        /*filename=*/QString(),
                        /*prefix=*/QString(),
                        /*directory=*/":/translations",
                        /*suffix=*/".qm")) {
      a.installTranslator(&translator);
    }
    else {
      USBGUARD_LOG(Debug) << "Translations not available for the current locale.";
    }

    SingleInstanceManager instanceManager;
    SingleInstanceManager::Action action;

    if (params.background) {
      action = SingleInstanceManager::Action::NOOP;
    }
    else {
      action = SingleInstanceManager::Action::Show;
    }

    if (instanceManager.sendCommand(action)) {
      if (params.background) {
        qWarning() << "An instance of this program is already running. Exiting.";
        exit(1);
      }
      else {
        exit(0);
      }
    }

    if (!instanceManager.connect()) {
      qFatal("Could not launch the application, failed to setup SingleApplicationManager with error '%s'",
        instanceManager.getLastError().toStdString().c_str());
    }

    const SessionBlocker block(a);

    MainWindow w(instanceManager, params.background);
    a.setQuitOnLastWindowClosed(false);
    return a.exec();
}

/* vim: set ts=2 sw=2 et */
