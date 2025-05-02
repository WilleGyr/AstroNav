#include "infobox.h"
#include "ui_infobox.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>


InfoBox::InfoBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoBox)
{
    ui->setupUi(this);
    connect(ui->editButton, &QPushButton::clicked, this, &InfoBox::on_editSaveButton_clicked);
    ui->nameLineEdit->setVisible(false);
    ui->spTypeLineEdit->setVisible(false);
    ui->spTypeLabel_2->setVisible(false);


}

InfoBox::~InfoBox()
{
    delete ui;
}

void InfoBox::setEditButtonVisibleForAdmin(bool isAdmin) {
    ui->editButton->setVisible(isAdmin);
    ui->editButton->setEnabled(isAdmin);

}


void InfoBox::setStarInfo(const QString& id, const QString& x, const QString& y, const QString& z, const QString& spType) {
    ui->ID_label->setText(id);
    ui->coordinatesLabel->setText("<b>Coordinates:</b><br> X: " + x + "<br>Y: " + y + "<br>Z: "+ z);
    ui->spTypeLabel->setText("<b>Spectral Type:</b> " + spType);
}

QString InfoBox::getStarId(){
    return ui->ID_label->text();
}

void InfoBox::toggleEditMode(bool enabled) {
    ui->nameLineEdit->setVisible(true);
    ui->spTypeLineEdit->setVisible(true);
    ui->spTypeLabel_2->setVisible(true);
    editMode = enabled;

    ui->nameLineEdit->setVisible(enabled);
    ui->spTypeLineEdit->setVisible(enabled);
    ui->ID_label->setVisible(!enabled);
    ui->spTypeLabel->setVisible(!enabled);

    ui->editButton->setText(enabled ? "Save" : "Edit");

    if (enabled) {
        ui->nameLineEdit->setText(ui->ID_label->text());
        QString currentSp = ui->spTypeLabel->text().section(" ", -1);
        ui->spTypeLineEdit->setText(currentSp);
    }
}

void InfoBox::on_editSaveButton_clicked() {
    if (!editMode) {
        toggleEditMode(true);
    } else {
        QString newName = ui->nameLineEdit->text();
        QString newSpType = ui->spTypeLineEdit->text();
        QString oldName = ui->ID_label->text();

        QSqlDatabase db = QSqlDatabase::database("starsConnection");
        QSqlQuery query(db);
        query.prepare("UPDATE stars SET MAIN_ID = ?, SP_TYPE = ? WHERE MAIN_ID = ?");
        query.addBindValue(newName);
        query.addBindValue(newSpType);
        query.addBindValue(oldName);

        if (!query.exec()) {
            QMessageBox::warning(this, "Update Failed", "Failed to update star info: " + query.lastError().text());
        } else {
            emit requestReload();
            ui->ID_label->setText(newName);
            ui->spTypeLabel->setText("<b>Spectral Type:</b> " + newSpType);
            toggleEditMode(false);
            ui->nameLineEdit->setVisible(false);
            ui->spTypeLineEdit->setVisible(false);
            ui->spTypeLabel_2->setVisible(false);
        }
    }
}
