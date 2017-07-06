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
#pragma once
#ifdef HAVE_BUILD_CONFIG_H
#include <build-config.h>
#endif

#include "DeviceModel.h"
#include "SingleInstanceManager.h"
#include "TargetDelegate.h"
#include "notifications/DBusNotifier.h"
#include "backend/DBusBackend.h"

#include <QSystemTrayIcon>
#include <QMainWindow>
#include <QTimer>
#include <QSettings>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(SingleInstanceManager &instanceManager, QWidget *parent = 0);
  ~MainWindow();

signals:
  void uiConnected();
  void uiDisconnected();

protected slots:
  void switchVisibilityState(QSystemTrayIcon::ActivationReason reason);
  void flashStep();
  void backendTryConnect();

  void showDeviceDialog(quint32 id, const usbguard::Rule& device_rule);
  void showMessage(const QString &message, bool alert = false, bool statusbar = false);

  void handleDevicePresenceChange(quint32 id,
                                  usbguard::DeviceManager::EventType event,
                                  usbguard::Rule::Target target,
                                  const std::string& device_rule);

  void handleDevicePolicyChange(quint32 id,
                                usbguard::Rule::Target target_old,
                                usbguard::Rule::Target target_new,
                                const std::string& device_rule,
                                quint32 rule_id);

  void notifyBackendConnected();
  void notifyBackendDisconnected();
  void notifyDevicePresenceChanged(usbguard::DeviceManager::EventType event, const usbguard::Rule& device_rule);
  void notifyDevicePolicyChanged(const usbguard::Rule& device_rule, quint32 rule_id);
  void notify(const QString& title, Notification::Urgency urgency, const usbguard::Rule& device_rule, bool show_notification);

  void allowDevice(quint32 id, bool permanent);
  void blockDevice(quint32 id, bool permanent);
  void rejectDevice(quint32 id, bool permanent);

  void handleBackendConnect();
  void handleBackendDisconnect();

  void handleDeviceInsert(quint32 id, const usbguard::Rule& device_rule);
  void handleDeviceRemove(quint32 id, const usbguard::Rule& device_rule);

  void loadSettings();
  void saveSettings();

  void loadDeviceList();
  void editDeviceListRow(const QModelIndex &index);
  void commitDeviceListChanges();
  void clearDeviceList();
  void resetDeviceList();

  void changeEvent(QEvent* e) override;
  void closeEvent(QCloseEvent* e) override;

  void setupSystemTray();
  void setupSettingsWatcher();
  void startFlashing();
  void stopFlashing();

  /*
    Handle the action given by the Single Instance Manager.

    This allows other instance of this program to send messages to the application
    in order, for instance, to show its foreground without launching a second instance.

    @param action Action requested by the manager.
    @param data Additional data that might be useful.
  */
  void handleManagerAction(SingleInstanceManager::Action action, QByteArray &data);

private:
  Ui::MainWindow *ui;
  QSystemTrayIcon *systray;
  QTimer _flash_timer;
  bool _flash_state;
  QTimer _backend_timer;
  QSettings _settings;
  DeviceModel _device_model;
  TargetDelegate _target_delegate;

  void notifyFailure(std::string function, std::string message);

  DBusNotifier _notifier;
  DBusBackend _backend;
};

/* vim: set ts=2 sw=2 et */
