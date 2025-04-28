#include <iostream>
#include <sstream>
#include <string>
#include "databasehandler.h"
#include <cmath>

// Emanuel Bengtsson - 03-mar
// Function for getting path to databasefile
std::string getDatabasePath(std::string argv){
    std::string databasePath = argv;
    // Since Mac and Windows uses different signs (forward- and backslash) in filepaths
    // We need to decide which sign to use in the filepath
    char cut;
    if(databasePath.find('\\')!=-1){
        cut = '\\';
    }
    else{
        cut = '/';
    }

    // cuts out what is not the path to the current directory
    // adds on the name of the database file
    for(int i=0; i<3; i++){
        int cutter = databasePath.find_last_of(cut);
        databasePath = databasePath.substr(0,cutter);    
    }
    std::stringstream ss;
    std::string filepath = "local_stars.db";
    ss << databasePath << cut << filepath;
    databasePath = ss.str();

    std::cout << databasePath << "  " << std::endl;

    return databasePath;
}

QSqlDatabase openDatabase(const std::string& filename) {
    QString connectionName = "starsConnection";

    // Återanvänd anslutningen om den redan finns
    if (QSqlDatabase::contains(connectionName)) {
        return QSqlDatabase::database(connectionName);
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    QString databasePath = QString::fromStdString(getDatabasePath(filename));
    db.setDatabaseName(databasePath);

    if (!db.open()) {
        qWarning() << "Error: Unable to open database:" << db.lastError().text();
        QMessageBox::critical(nullptr, "Database Error", "Unable to open the stars database.");
    }

    return db;
};

void SeparateStars(const std::string& filename){
    // opens databasefile
    QSqlDatabase db = openDatabase(filename);
    
    // selects every star from the stars-table
    QSqlQuery rows("SELECT * FROM stars",db);
    if(!rows.exec()){
        qDebug() << "Error executing query:" << rows.lastError();
        return;
    }
    int counter = 0;
    // for every star, checks if any other stars are clumped with it
    // if a clumped star is found, seperates the stars by moving one of them away from the other
    while(rows.next()){
        double x_koord_1 = rows.value(5).toDouble();
        double y_koord_1 = rows.value(6).toDouble();
        double z_koord_1 = rows.value(7).toDouble();

        QSqlQuery clumped(db);
        clumped.prepare("SELECT * FROM stars WHERE ABS(x_koord - :value_x) < 0.2 AND ABS(y_koord - :value_y) < 0.2 AND ABS(z_koord - :value_z) < 0.2");
        clumped.bindValue(":value_x",x_koord_1);
        clumped.bindValue(":value_y",y_koord_1);
        clumped.bindValue(":value_z",z_koord_1);
        //("SELECT * FROM stars WHERE ABS(x_koord-x_koord_1)<3 AND ABS(y_koord-y_koord_1)<3 AND ABS(z_koord-z_koord_1)<3",db);
        if(!clumped.exec()){
            qDebug() << "Error executing query:" << clumped.lastError();
            return;
        }
        while(clumped.next()){
            counter++;
            if(rows.value(0) != clumped.value(0)){
                double x_diff = clumped.value(5).toDouble() - x_koord_1;
                double y_diff = clumped.value(6).toDouble() - y_koord_1;
                double z_diff = clumped.value(7).toDouble() - z_koord_1;
                double dist = sqrt(pow(x_diff,2)+pow(y_diff,2)+pow(z_diff,2));
                double factor = 0.3/dist;
                qDebug() << dist;

                // updating the koordinates in the database
                QSqlQuery change_x(db);
                change_x.prepare("UPDATE stars SET x_koord = :newValue WHERE MAIN_ID = :id");
                change_x.bindValue(":newValue",clumped.value(5).toDouble()-x_diff*factor);
                change_x.bindValue(":id",rows.value(0).toString());
                if(!change_x.exec()){
                    qDebug() << "Error: failed to upload data -" << change_x.lastError();
                }
                QSqlQuery change_y(db);
                change_y.prepare("UPDATE stars SET y_koord = :newValue WHERE MAIN_ID = :id");
                change_y.bindValue(":newValue",clumped.value(6).toDouble()-y_diff*factor);
                change_y.bindValue(":id",rows.value(0).toString());
                if(!change_y.exec()){
                    qDebug() << "Error: failed to upload data -" << change_y.lastError();
                }
                QSqlQuery change_z(db);
                change_z.prepare("UPDATE stars SET z_koord = :newValue WHERE MAIN_ID = :id");
                change_z.bindValue(":newValue",clumped.value(7).toDouble()-z_diff*factor);
                change_z.bindValue(":id",rows.value(0).toString());
                if(!change_z.exec()){
                    qDebug() << "Error: failed to upload data -" << change_z.lastError();
                }
            }
        }
    }
    qDebug() << counter;
    db.close();
};
