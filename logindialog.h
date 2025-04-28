#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "databasehandler.h"
#include "ui_logindialog.h"

namespace Ui {
class LoginDialog;
}


class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    bool isUserLoggedIn() const { return userLoggedIn; }  // Getter function
    QString getUsername() const { return loggedUsername; }      // Getter for username

    void clearFields() { // Clear the input fields
        ui->usernameLineEdit->clear();
        ui->passwordLineEdit->clear();
    }

private slots:
    void on_loginButton_clicked();
    void on_cancelButton_clicked();
    void on_guestButton_clicked();
    void on_registerMenuButton_clicked();

signals:
    void dialogAccepted();

private:
    Ui::LoginDialog *ui;
    bool userLoggedIn = false;
    QString loggedUsername = ""; // Store the logged-in username
};


#endif // LOGINDIALOG_H
