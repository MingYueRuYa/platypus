#include "helpdialog.h"

#include "ui_helpdialog.h"
#include "config_manager.h"

using ShortcutType = ConfigManager::ConfigType;

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::HelpDialog) {
  ui->setupUi(this);
  updateUI();
}

HelpDialog::~HelpDialog() {}

void HelpDialog::updateUI() {
  QString short_cut = QString::fromStdString(
      ConfigManagerInstance.GetConfig(ShortcutType::Create_New_Tab));
  ui->lbl_create_new_tab->setText(QString("create new tab: %1").arg(short_cut));
  short_cut = QString::fromStdString(
      ConfigManagerInstance.GetConfig(ShortcutType::Delete_Tab));
  ui->lbl_delete_tab->setText(QString("delete tab: %1").arg(short_cut));
  short_cut = QString::fromStdString(
      ConfigManagerInstance.GetConfig(ShortcutType::Switch_Forward_Tab));
  ui->switch_forward_tab->setText(
      QString("switch forward tab: %1").arg(short_cut));
  short_cut = QString::fromStdString(
      ConfigManagerInstance.GetConfig(ShortcutType::Switch_Back_Tab));
  ui->switch_back_tab->setText(QString("switch back tab: %1").arg(short_cut));
  short_cut = QString::fromStdString(
      ConfigManagerInstance.GetConfig(ShortcutType::Max_Min_Window));
  ui->max_min_window->setText(QString("max/min window: %1").arg(short_cut));
}