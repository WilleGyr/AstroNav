#ifndef THIRDPERSONCAMERACONTROLLER_H
#define THIRDPERSONCAMERACONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QVector3D>
#include <QEasingCurve>
#include <Qt3DRender/QCamera>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QOrbitCameraController>
#include "music.h"

class ThirdPersonCameraController : public QObject
{
    Q_OBJECT

public:
    explicit ThirdPersonCameraController(Qt3DRender::QCamera *camera, Qt3DCore::QEntity *rootEntity, BackgroundMusic *bgMusic, QObject *parent = nullptr);
    ~ThirdPersonCameraController();

    void animateCameraToPosition(const QVector3D &targetPosition, const QVector3D &targetViewCenter);

    // Get/set camera and enable/disable control
    Qt3DRender::QCamera* camera() const { return m_camera; }
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_isEnabled; }

    // Access to the orbit controller for third-person view
    Qt3DExtras::QOrbitCameraController* orbitController() const { return m_orbitController; }

public slots:
    void teleportToStar(const QVector3D &coordinates, const QString &starId = QString());
    void handleStarClick(Qt3DCore::QTransform *starTransform);
    void handleSunClick(Qt3DCore::QTransform *sunTransform);

signals:
    void starTeleported(const QString &starId, const QVector3D &position);
    void starSelected(const QString &starId, const QVector3D &position);

private slots:
    void updateCameraPosition();
    void updateFocus();

private:
    Qt3DRender::QCamera *m_camera;
    Qt3DCore::QEntity *m_rootEntity;
    QTimer *m_cameraTimer;
    QTimer *m_focusTimer;
    bool m_isAnimating;
    QVector3D m_startPosition;
    QVector3D m_targetPosition;
    QVector3D m_startViewCenter;
    QVector3D m_targetViewCenter;
    float m_elapsedTime;
    float m_duration;
    QEasingCurve m_easingCurve;
    QVector3D m_focusPoint;
    BackgroundMusic *m_bgMusic;

    // Third-person view controls
    Qt3DExtras::QOrbitCameraController *m_orbitController;
    QVector3D m_thirdPersonOffset; // Offset for third-person camera
    float m_stoppingDistance = 10.0f;
    bool m_isEnabled = true;      // Whether this controller is active
};

#endif // THIRDPERSONCAMERACONTROLLER_H
