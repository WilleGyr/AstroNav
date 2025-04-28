#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include <Qt3DRender/QCamera>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include "firstpersoncameracontroller.h"
#include "thirdpersoncameracontroller.h"
#include "music.h"

class CameraManager : public QObject
{
    Q_OBJECT

public:
    enum CameraMode {
        FirstPersonMode,
        ThirdPersonMode
    };
    Q_ENUM(CameraMode)

    explicit CameraManager(Qt3DRender::QCamera *camera, Qt3DCore::QEntity *rootEntity,
                           BackgroundMusic *bgMusic, QObject *parent = nullptr);
    ~CameraManager();

    void setCameraMode(CameraMode mode);
    CameraMode cameraMode() const { return m_cameraMode; }

public slots:
    void toggleCameraMode();

    // Forward relevant signals to the active controller
    void teleportToStar(const QVector3D &coordinates, const QString &starId = QString());
    void handleStarClick(Qt3DCore::QTransform *starTransform);
    void handleSunClick(Qt3DCore::QTransform *sunTransform);

signals:
    void cameraModeChanged(CameraMode mode);
    void starTeleported(const QString &starId, const QVector3D &position);
    void starSelected(const QString &starId, const QVector3D &position);

private:
    Qt3DRender::QCamera *m_camera;
    FirstPersonCameraController *m_firstPersonController;
    ThirdPersonCameraController *m_thirdPersonController;
    CameraMode m_cameraMode;

    // Track the currently focused star/entity
    Qt3DCore::QTransform *m_currentStarTransform;
    QString m_currentStarId;
    bool m_isViewingSun;
};

#endif // CAMERAMANAGER_H
