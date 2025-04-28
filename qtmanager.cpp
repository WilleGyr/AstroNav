#include "qtmanager.h"

qtmanager::qtmanager() {}

void qtmanager::createMainWindow(QMainWindow &mainWindow, QWidget *centralWidget){
    mainWindow.setCentralWidget(centralWidget);
    mainWindow.setContentsMargins(0, 0, 0, 0);
    mainWindow.resize(1280, 720);
    mainWindow.show();
}

void qtmanager::setMainLayout(QHBoxLayout *mainLayout, QWidget *rightBox, QWidget *container){
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(container, 4);  // 3D view takes 4/5 of the width
    mainLayout->addWidget(rightBox, 1);  // Right box takes 1/5 of the width
}

void qtmanager::setRightLayout(QVBoxLayout *rightLayout, InfoBox *topPanel, ActivityBox *bottomPanel){
    rightLayout->setContentsMargins(0, 0, 0, 0);  // No margins
    rightLayout->setSpacing(0);  // No spacing
    rightLayout->addWidget(topPanel, 1);  // Top takes 1/3 of the height
    rightLayout->addWidget(bottomPanel, 2);  // Bottom takes 2/3 of the height
};