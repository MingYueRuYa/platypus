#ifndef help_dialog_h
#define help_dialog_h

#include <QDialog>

namespace Ui {
class HelpDialog;
}

class HelpDialog : public QDialog {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(HelpDialog)

 public:
  HelpDialog(QWidget *parent = nullptr);
  ~HelpDialog();

 private:
  Ui::HelpDialog *ui;
};

#endif  // help_dialog_h