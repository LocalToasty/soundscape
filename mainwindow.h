#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QJsonObject>
#include <QMainWindow>
#include <QVBoxLayout>

namespace Ui {
class MainWindow;
}

class SoundEffect;

class MainWindow : public QMainWindow {
  Q_OBJECT

  friend SoundEffect;

 public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  void read(QJsonObject const& json);
  void write(QJsonObject& json) const;

 private slots:
  void on_action_Add_Sound_triggered();
  void on_action_New_triggered();
  void on_action_Save_triggered();
  void on_action_Open_triggered();

  void on_pushButton_clicked();

 protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dropEvent(QDropEvent* event);

 private:
  Ui::MainWindow* ui;
  QVBoxLayout* soundLayout;
};

#endif  // MAINWINDOW_H
