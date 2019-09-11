#ifndef AMBIENT_H
#define AMBIENT_H

#include <QJsonObject>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QTimer>
#include <QWidget>

namespace Ui {
class SoundEffect;
}

class MainWindow;

class SoundEffect : public QWidget {
  Q_OBJECT

 public:
  explicit SoundEffect(MainWindow& parent,
                       QString const& name = "",
                       QList<QUrl> const& soundFiles = QList<QUrl>(),
                       int loopInterval = 0,
                       bool loop = false);
  ~SoundEffect();

  static SoundEffect* fromJson(MainWindow& parent, QJsonObject const& json);
  void write(QJsonObject& json) const;

 protected:
  void contextMenuEvent(QContextMenuEvent* event);
  void mousePressEvent(QMouseEvent* event);

 private slots:
  void on_playButton_clicked();
  void set_playButton(QMediaPlayer::State state);
  void on_loop_stateChanged(int);
  void edit();
  void deleteFromParent();
  void fadeStep();
  void on_volumeSlider_valueChanged(int value);

 public slots:
  void play();
  void stop();

 protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dropEvent(QDropEvent* event);

 private:
  Ui::SoundEffect* ui;
  QMediaPlayer player;
  QMediaPlaylist playlist;

  QTimer loopTimer;
  int loopInterval;
  bool running{false};

  QTimer fadeTimer;

  MainWindow& parentWindow;
};

#endif  // AMBIENT_H
