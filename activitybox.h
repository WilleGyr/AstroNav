#ifndef ACTIVITYBOX_H
#define ACTIVITYBOX_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QListWidget>
#include <QVector3D>
#include <QPushButton>

namespace Ui {
class ActivityBox;
}

class ActivityBox : public QWidget {
    Q_OBJECT

signals:
    void teleportToStar(const QVector3D &coordinates, const QString &starId);
    void backToLoginRequested();

    void toggleCameraMode(); // Signal to toggle camera mode


public:
    explicit ActivityBox(QWidget *parent = nullptr);
    ~ActivityBox();
    void userLogin(bool user, QString username); // Updated method to accept username
    bool isAdminUser(const QString& username);
    void setCurrentStarId(const QString& starId);

    void updateCameraModeButton(int mode);
    void setButtonImage(QPushButton* button, const QString& normalPath, const QString& pressedPath);
    void updateFavoriteButtonIcon();



private slots:
    void onSearchButtonClicked();
    void onFavoriteButtonClicked();
    void updateFavoriteList();
    void onSearchMenuButtonClicked();
    void onUsersMenuButtonClicked();
    void deleteSelectedUser();
    void onQuitButtonClicked();
    void onBackToLoginClicked();
    void onFavoriteMenuButtonClicked();
    void loadUserFavorites();
    void onFavoriteItemDoubleClicked(QListWidgetItem* item);
    void onPasswordButtonClicked();
    void onHelpMenuButtonClicked();
    void onSunButtonClicked();
    void onTypeBoxChanged(const QString &text);
    void onSearchResultsDoubleClicked(QListWidgetItem* item);

    void onToggleCameraModeClicked(); // Handle camera mode toggle button click
    void onFavoriteItemSingleClicked(QListWidgetItem* item);
    void showWidgets(const QList<QWidget*>& widgets);
    void hideWidgets(const QList<QWidget*>& widgets);
    void setMenuButtonPressed(QPushButton* pressedButton);
    void searchByType(const QString &typeLetter);

private:
    Ui::ActivityBox *ui;
    QString currentStarId; // Current star ID (now QString)
    QList<QString> favoritesList;

    static QSqlDatabase m_starsDb;

    void searchById(const QString &id);
    QString loggedInUsername;  // Store the username
    QListWidget *usersList;

    QList<QWidget*> SearchWidgets;
    QList<QWidget*> UsersWidgets;
    QList<QWidget*> FavoriteWidgets;
    QList<QWidget*> HelpWidgets;

    const QString ICON_PATH = ":/SpaceKnappar/";

    const QString NOT_PRESSED = "(NotPressed).png";
    const QString PRESSED = "(Pressed).png";
    const QString MID_PRESS = "(MidPress).png";

    const QString FAVO = ICON_PATH+"Favo";
    const QString USER_MENU = ICON_PATH+"UserMenu";
    const QString HELP_MENU = ICON_PATH+"HelpMenu";
    const QString MAKE_FAVO = ICON_PATH+"MakeFavo";
    const QString SEARCH_MENU = ICON_PATH+"SearchMenu";
};

#endif // ACTIVITYBOX_H
