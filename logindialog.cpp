#include "logindialog.h"
#include "ui_logindialog.h"
#include "registerdialog.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>
#include <QPixmap>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog) {
    ui->setupUi(this);

    // Create background label
    QLabel *label = new QLabel(this);
    QPixmap pixmap(":/images/login_background.jpg");
    label->setPixmap(pixmap);
    label->setScaledContents(true);
    label->lower();  // Make sure it stays behind all other widgets

    // Make the label resize with the dialog
    label->setGeometry(0, 0, this->width(), this->height());

    this->setWindowFlags(Qt::FramelessWindowHint);


    ui->InvalidLoginLabel->setVisible(false);
    ui->emptyFieldsLabel->setVisible(false);
}

LoginDialog::~LoginDialog() {
    delete ui;
}

void LoginDialog::on_loginButton_clicked() {
    ui->InvalidLoginLabel->setVisible(false);
    ui->emptyFieldsLabel->setVisible(false);

    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        ui->emptyFieldsLabel->setVisible(true);
        return;
    }

    QString appPath = QCoreApplication::applicationFilePath();
    QString databasePath = QString::fromStdString(getDatabasePath(appPath.toStdString()));
    // Open (or retrieve) a connection to the database using a named connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databasePath);
    // QString dbPath = "/Users/williamgyrulf/Uni/Programvaruutveckling/programvaruutveckling-projekt/SapceApp/local_stars.db";
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

    // Query the database for the given username and password
    QSqlQuery query(db);
    query.prepare("SELECT * FROM users WHERE USERNAME = :username AND PASSWORD = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    if(query.exec() && query.next()) {
        userLoggedIn = true;
        loggedUsername = username;
        emit dialogAccepted(); // Emit the signal before accepting the dialog
        accept();  // Login successful
    } else {
        ui->InvalidLoginLabel->setVisible(true);
    }
}

void LoginDialog::on_cancelButton_clicked() {
    reject(); // Close the dialog without success
}

void LoginDialog::on_registerMenuButton_clicked() {
    RegisterDialog regDialog(this); // Create the dialog
    regDialog.exec(); // Show it as a modal dialog
}

void LoginDialog::on_guestButton_clicked() {
    userLoggedIn = false;
    loggedUsername = "Guest";  // Set username as "Guest" for guest logins
    emit dialogAccepted(); // Emit the signal before accepting the dialog
    accept();
}
