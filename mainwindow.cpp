#include "mainwindow.h"
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLayout>
#include <QMessageBox>
#include <QMimeData>
#include "soundeffect.h"
#include "soundeffectdialog.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  setWindowTitle(tr("Soundscape"));

  auto* central = new QWidget(this);
  soundLayout = new QVBoxLayout(central);
  soundLayout->addStretch();
  ui->scrollArea->setWidget(central);
  ui->scrollArea->setWidgetResizable(true);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::read(const QJsonObject& json) {
  for (auto val : json["sounds"].toArray()) {
    if (auto* sf = SoundEffect::fromJson(*this, val.toObject()))
      soundLayout->insertWidget(soundLayout->count() - 1, sf);
  }
}

void MainWindow::write(QJsonObject& json) const {
  QJsonArray sounds;
  for (int i = 0; i < soundLayout->count() - 1; i++) {
    QJsonObject o;
    auto* sf = static_cast<SoundEffect*>(soundLayout->itemAt(i)->widget());

    sf->write(o);
    sounds.push_back(o);
  }
  json["sounds"] = sounds;
}

void MainWindow::on_action_Add_Sound_triggered() {
  SoundEffectDialog dialog(this);
  if (dialog.exec()) {
    auto* sf = new SoundEffect(*this, dialog.name(), dialog.urls(),
                               dialog.loopInterval(), false);
    soundLayout->insertWidget(soundLayout->count() - 1, sf);
  }
}

void MainWindow::on_action_New_triggered() {
  if (soundLayout->count() > 1 &&
      QMessageBox::question(this, tr("New Project"),
                            tr("Discard current and create new project?")) !=
          QMessageBox::Yes)
    return;

  // clear old sounds
  while (soundLayout->count() > 1)
    delete soundLayout->takeAt(0);
}

void MainWindow::on_action_Save_triggered() {
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save as"), "",
                                                  tr("JSON file (*.json)"));

  QFile saveFile(fileName);
  if (!saveFile.open(QIODevice::WriteOnly)) {
    qWarning("Couldn't open save file.");
    return;
  }

  QJsonObject o;
  write(o);
  QJsonDocument doc(o);
  saveFile.write(doc.toJson());
}

void MainWindow::on_action_Open_triggered() {
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                                                  tr("JSON file (*.json)"));
  if (fileName.isEmpty())
    return;

  if (soundLayout->count() > 1 &&
      QMessageBox::question(
          this, tr("Open Project"),
          tr("Really discard current and load other project?")) !=
          QMessageBox::Yes)
    return;

  // clear old sounds
  while (soundLayout->count() > 1)
    delete soundLayout->takeAt(0);

  QFile loadFile(fileName);
  if (!loadFile.open(QIODevice::ReadOnly)) {
    qWarning("Couldn't open save file.");
    return;
  }

  QByteArray saveData = loadFile.readAll();
  QJsonParseError error;
  QJsonDocument loadDoc(QJsonDocument::fromJson(saveData, &error));
  read(loadDoc.object());
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
  qDebug() << event->mimeData()->formats();
  if (event->mimeData()->hasFormat("text/plain"))
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event) {
  auto* sf = SoundEffect::fromJson(
      *this,
      QJsonDocument::fromJson(event->mimeData()->data("text/plain")).object());

  if (!sf)
    return;

  // delete old instance if dragged within window
  for (int i = 0; i < soundLayout->count() - 1; i++) {
    if (event->source() == soundLayout->itemAt(i)->widget()) {
      soundLayout->removeWidget(static_cast<QWidget*>(event->source()));
      delete event->source();
    }
  }

  soundLayout->insertWidget(soundLayout->count() - 1, sf);

  event->acceptProposedAction();
}

void MainWindow::on_pushButton_clicked() {
  for (int i = 0; i < soundLayout->count() - 1; i++)
    static_cast<SoundEffect*>(soundLayout->itemAt(i)->widget())->stop();
}
