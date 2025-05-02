#ifndef INFOBOX_H
#define INFOBOX_H

#include <QWidget>
#include <QKeyEvent>

namespace Ui {
class InfoBox;
}

class InfoBox : public QWidget
{
    Q_OBJECT

public:
    explicit InfoBox(QWidget *parent = nullptr);
    ~InfoBox();

    // Method to update the displayed star information
    void setStarInfo(const QString& id, const QString& x, const QString& y, const QString& z, const QString& spType);

    QString getStarId();
    void setEditButtonVisibleForAdmin(bool isAdmin);

signals:
    void requestReload();

private slots:
    void on_editSaveButton_clicked();

private:
    Ui::InfoBox *ui;
    void toggleEditMode(bool enabled);
    bool editMode = false;
};

#endif // INFOBOX_H
