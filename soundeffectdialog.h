#ifndef SOUNDEFFECTDIALOG_H
#define SOUNDEFFECTDIALOG_H

#include <QDialog>

namespace Ui {
class SoundEffectDialog;
}

class QListWidgetItem;
class SoundEffect;

class SoundEffectDialog : public QDialog {
  friend SoundEffect;
  Q_OBJECT

 public:
  explicit SoundEffectDialog(QWidget* parent = nullptr);
  ~SoundEffectDialog();

  QString name() const;
  QList<QUrl> urls() const;
  int loopInterval() const;

 private slots:
  void on_add_button_clicked();
  void on_delete_button_clicked();

  void on_name_textChanged(const QString& arg1);

 protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dropEvent(QDropEvent* event);

 private:
  Ui::SoundEffectDialog* ui;
};

#endif  // SOUNDEFFECTDIALOG_H
