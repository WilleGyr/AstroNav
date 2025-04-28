#include "registerdialog.h"
#include "ui_registerdialog.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QCoreApplication>

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog) {
    ui->setupUi(this);
    ui->emptyFieldsLabel->setVisible(false);
    ui->passwordNotMatch->setVisible(false);
    ui->usernameExists->setVisible(false);

    QLabel *label = new QLabel(this);
    QPixmap pixmap(":/images/register_background.jpg");

    // Scale the pixmap to the size of the window while keeping the aspect ratio
    QPixmap scaled = pixmap.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    label->setPixmap(scaled);

    // Resize the label to fill the window
    label->resize(this->size());
    label->lower();

}

RegisterDialog::~RegisterDialog() {
    delete ui;
}


void RegisterDialog::on_registerButton_clicked() {
    ui->emptyFieldsLabel->setVisible(false);
    ui->passwordNotMatch->setVisible(false);
    ui->usernameExists->setVisible(false);

    QString username = ui->usernameRegister->text();
    QString password = ui->passwordRegister->text();
    QString confirmPassword = ui->passwordConfirmRegister->text();

    if (username.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()) {
        ui->emptyFieldsLabel->setVisible(true);
        return;
    }
    else if (password != confirmPassword) {
        ui->passwordNotMatch->setVisible(true);
        return;
    }

    QString appPath = QCoreApplication::applicationFilePath();
    QString databasePath = QString::fromStdString(getDatabasePath(appPath.toStdString()));
    // Open (or retrieve) a connection to the database using a named connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databasePath);
    // QString dbPath = "C:/Users/ceasa/OneDrive/Dokument/Shapecheck-git-push-pull-test/programvaruutveckling-projekt/SapceApp/local_stars.db";
    if (QSqlDatabase::contains("usersConnection"))
        db = QSqlDatabase::database("usersConnection");
    else {
        db = QSqlDatabase::addDatabase("QSQLITE", "usersConnection");
        db.setDatabaseName(databasePath);
        if (!db.open()) {
            QMessageBox::critical(this, "Database Error", "Unable to open the database.");
            return;
        }
    }

    // Check if the username already exists
    QSqlQuery query(db);
    query.prepare("SELECT * FROM users WHERE USERNAME = :username");
    query.bindValue(":username", username);
    if(query.exec() && query.next()) {
        ui->usernameExists->setVisible(true);
        return;
    }

    // Insert the new user into the database
    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO users (USERNAME, PASSWORD) VALUES (:username, :password)");
    insertQuery.bindValue(":username", username);
    insertQuery.bindValue(":password", password);
    if(!insertQuery.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to register new user: " + insertQuery.lastError().text());
        return;
    }

    // Registration successful, close the dialog
    accept();
}

void RegisterDialog::on_cancelButtonRegister_clicked() {
    reject();
}
