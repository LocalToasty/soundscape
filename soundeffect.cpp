#include "soundeffect.h"
#include <QContextMenuEvent>
#include <QDrag>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMediaPlayer>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include "mainwindow.h"
#include "soundeffectdialog.h"
#include "ui_soundeffect.h"
#include "ui_soundeffectdialog.h"

SoundEffect::SoundEffect(MainWindow& parent,
                         QString const& name,
                         QList<QUrl> const& soundUrls,
                         int loopInterval,
                         bool loop)
    : QWidget(&parent),
      ui(new Ui::SoundEffect),
      loopInterval(loopInterval),
      parentWindow(parent) {
  ui->setupUi(this);
  ui->label->setText(name);
  ui->loop->setChecked(loop);

  connect(&player, &QMediaPlayer::stateChanged, this,
          &SoundEffect::set_playButton);

  for (auto url : soundUrls)
    playlist.addMedia(url);

  playlist.setPlaybackMode(QMediaPlaylist::PlaybackMode::CurrentItemOnce);
  player.setPlaylist(&playlist);

  connect(ui->action_Edit, &QAction::triggered, this, &SoundEffect::edit);
  connect(ui->action_Delete, &QAction::triggered, this,
          &SoundEffect::deleteFromParent);

  loopTimer.setSingleShot(true);
  connect(&loopTimer, &QTimer::timeout, this, &SoundEffect::play);

  connect(&fadeTimer, &QTimer::timeout, this, &SoundEffect::fadeStep);
}

SoundEffect::~SoundEffect() {
  delete ui;
  qDebug() << "Foo\n";
}

SoundEffect* SoundEffect::fromJson(MainWindow& parent,
                                   const QJsonObject& json) {
  if (!json.contains("name") || !json["name"].isString())
    return nullptr;

  QList<QUrl> urls;
  for (auto val : json["sources"].toArray()) {
    if (!val.isString())
      return nullptr;
    urls.append(val.toString());
  }

  auto* sf = new SoundEffect(parent, json["name"].toString(), urls,
                             json["interval"].toInt(), json["loop"].toBool());
  sf->ui->volumeSlider->setValue(json["volume"].toInt(50));

  return sf;
}

void SoundEffect::write(QJsonObject& json) const {
  json["name"] = ui->label->text();
  json["volume"] = ui->volumeSlider->value();
  json["loop"] = ui->loop->isChecked();
  json["interval"] = loopInterval;

  QJsonArray sources;
  for (int i = 0; i < playlist.mediaCount(); i++)
    sources.push_back(playlist.media(i).canonicalUrl().toString());

  json["sources"] = sources;
}

void SoundEffect::contextMenuEvent(QContextMenuEvent* event) {
  QMenu menu(this);
  menu.addAction(ui->action_Edit);
  menu.addAction(ui->action_Delete);
  menu.exec(event->globalPos());
}

void SoundEffect::on_playButton_clicked() {
  if (running)
    stop();
  else
    play();
}

void SoundEffect::set_playButton(QMediaPlayer::State state) {
  if (running) {
    if (!ui->loop->isChecked())
      running = false;
    else if (state != QMediaPlayer::State::PlayingState)
      loopTimer.start(loopInterval);
  }

  if (running)
    ui->playButton->setText("■");
  else
    ui->playButton->setText("▶");
}

void SoundEffect::play() {
  fadeTimer.stop();
  player.stop();
  playlist.shuffle();
  player.setVolume(ui->volumeSlider->value());
  player.play();
  if (player.state() != QMediaPlayer::PlayingState)
    return;
  running = true;
  ui->playButton->setText("■");
  qInfo() << playlist.currentMedia().canonicalUrl().toString();
}

void SoundEffect::stop() {
  loopTimer.stop();
  running = false;
  ui->playButton->setText("▶");

  if (player.volume())
    fadeTimer.start(2000 / player.volume());
}

void SoundEffect::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasUrls() || event->mimeData()->hasText())
    event->acceptProposedAction();
}

void SoundEffect::dropEvent(QDropEvent* event) {
  if (event->mimeData()->hasUrls()) {
    for (auto url : event->mimeData()->urls())
      playlist.addMedia(url);

    event->acceptProposedAction();
  } else if (event->mimeData()->hasText()) {
    auto* sf = SoundEffect::fromJson(
        parentWindow,
        QJsonDocument::fromJson(event->mimeData()->data("text/plain"))
            .object());

    if (!sf)
      return;

    QBoxLayout* layout = parentWindow.soundLayout;

    // insert above
    for (int i = 0; i < layout->count() - 1; i++) {
      if (layout->itemAt(i)->widget() == this) {
        layout->insertWidget(i, sf);
        break;
      }
    }

    // delete old instance if dragged within window
    for (int i = 0; i < layout->count() - 1; i++) {
      if (event->source() == layout->itemAt(i)->widget()) {
        layout->removeWidget(static_cast<QWidget*>(event->source()));
        delete event->source();
        break;
      }
    }

    event->acceptProposedAction();
  }
}

void SoundEffect::edit() {
  stop();

  SoundEffectDialog dialog(this);

  dialog.ui->name->setText(ui->label->text());
  dialog.ui->interval->setValue(loopInterval / 1000.);
  for (int i = 0; i < playlist.mediaCount(); i++)
    dialog.ui->sound_list->addItem(playlist.media(i).canonicalUrl().toString());

  if (dialog.exec()) {
    ui->label->setText(dialog.name());
    loopInterval = dialog.loopInterval();
    playlist.clear();
    for (auto url : dialog.urls())
      playlist.addMedia(url);
  }
}

void SoundEffect::deleteFromParent() {
  if (QMessageBox::question(this, tr("Delete sound"),
                            tr("Do you really want to delete this sound?")) !=
      QMessageBox::Yes)
    return;
  stop();
  auto layout = parentWindow.soundLayout;
  for (int i = 0; i < layout->count() - 1; i++) {
    if (layout->itemAt(i)->widget() == this) {
      hide();
      delete layout->takeAt(i);
    }
  }
}

void SoundEffect::fadeStep() {
  player.setVolume(player.volume() - 1);

  if (!player.volume())
    fadeTimer.stop();
}

void SoundEffect::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    QJsonObject o;
    write(o);
    QJsonDocument doc(o);
    QMimeData* mimeData = new QMimeData();
    mimeData->setText(doc.toJson());

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
  }
}

void SoundEffect::on_loop_stateChanged(int state) {
  if (state == Qt::Unchecked)
    loopTimer.stop();
}

void SoundEffect::on_volumeSlider_valueChanged(int value) {
  if (running)
    player.setVolume(value);
}
