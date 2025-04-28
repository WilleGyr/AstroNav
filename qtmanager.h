#ifndef QTMANAGER_H
#define QTMANAGER_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include "infobox.h"
#include "activitybox.h"


class qtmanager
{
public:
    qtmanager();
    static void createMainWindow(QMainWindow &mainWindow, QWidget *centralWidget);
    static void setMainLayout(QHBoxLayout *mainLayout, QWidget *rightBox, QWidget *container);
    static void setRightLayout(QVBoxLayout *rightLayout, InfoBox *topPanel, ActivityBox *bottomPanel);
};

#endif // QTMANAGER_H
