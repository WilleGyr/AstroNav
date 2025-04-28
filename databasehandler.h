#include <string>
//#include <QCoreApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

std::string getDatabasePath(std::string argv);

QSqlDatabase openDatabase(const std::string& filename);

void SeparateStars(const std::string& filename);
