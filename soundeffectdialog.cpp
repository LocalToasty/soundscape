#include "soundeffectdialog.h"
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMimeData>
#include "ui_soundeffectdialog.h"

SoundEffectDialog::SoundEffectDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::SoundEffectDialog) {
  ui->setupUi(this);
  ui->sound_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->sound_list->setSortingEnabled(true);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
}

SoundEffectDialog::~SoundEffectDialog() {
  delete ui;
}

QString SoundEffectDialog::name() const {
  return ui->name->text();
}

QList<QUrl> SoundEffectDialog::urls() const {
  QList<QUrl> list;
  for (int i = 0; i < ui->sound_list->count(); i++)
    list.append(ui->sound_list->item(i)->text());

  return list;
}

int SoundEffectDialog::loopInterval() const {
  return static_cast<int>(ui->interval->value() * 1000);
}

void SoundEffectDialog::on_add_button_clicked() {
  auto filenames = QFileDialog::getOpenFileUrls(
      this, tr("Select Sounds"), QUrl(), tr("Audio Files (*.wav *.ogg *.mp3)"));
  if (filenames.empty())
    return;

  // add new elements to sound list
  for (int i = 0; i < ui->sound_list->count(); i++)
    filenames.removeAll(ui->sound_list->item(i)->text());

  for (auto url : filenames)
    ui->sound_list->addItem(url.toString());
}

void SoundEffectDialog::on_delete_button_clicked() {
  for (auto* item : ui->sound_list->selectedItems()) {
    ui->sound_list->takeItem(ui->sound_list->row(item));
  }
}

void SoundEffectDialog::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasUrls())
    event->acceptProposedAction();
}

void SoundEffectDialog::dropEvent(QDropEvent* event) {
  if (event->mimeData()->hasUrls()) {
    QStringList filenames;
    for (auto url : event->mimeData()->urls())
      if (url.isLocalFile())
        filenames.append(url.toLocalFile());

    // add new elements to sound list
    for (int i = 0; i < ui->sound_list->count(); i++)
      filenames.removeAll(ui->sound_list->item(i)->text());

    ui->sound_list->addItems(filenames);
    event->acceptProposedAction();
  }
}

void SoundEffectDialog::on_name_textChanged(const QString& arg1) {
  ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(arg1.isEmpty());
}
