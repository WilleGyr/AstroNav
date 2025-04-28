#ifndef FIRSTPERSONCAMERACONTROLLER_H
#define FIRSTPERSONCAMERACONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QVector3D>
#include <QEasingCurve>
#include <Qt3DRender/QCamera>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DInput/QMouseDevice>
#include <Qt3DInput/QMouseHandler>
#include <Qt3DLogic/QFrameAction>
#include "music.h"
#include <Qt3DInput/QKeyboardDevice>
#include <Qt3DInput/QKeyboardHandler>
#include <QKeyEvent>
#include <QHash>



class FirstPersonCameraController : public QObject
{
    Q_OBJECT

public:
    explicit FirstPersonCameraController(Qt3DRender::QCamera *camera,
                                         Qt3DCore::QEntity  *rootEntity,
                                         BackgroundMusic   *bgMusic,
                                         QObject *parent = nullptr);

    // Animate camera from current position to the target
    void animateCameraToPosition(const QVector3D &targetPosition,
                                 const QVector3D &targetViewCenter);

    // Remove the picker from the star so it no longer blocks picking
    void disablePickingOnStar(Qt3DCore::QEntity* starEntity);

    // Re-add the picker so the star is clickable again
    void enablePickingOnCurrentStar();

    // Enable/disable this controller
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_isEnabled; }

public slots:
    void handleStarClick(Qt3DCore::QTransform *starTransform);
    void handleSunClick(Qt3DCore::QTransform *sunTransform);
    void teleportToStar(const QVector3D &coordinates,
                        const QString &starId = QString());
    void onFrameUpdate(float dt);


signals:
    void starTeleported(const QString &starId, const QVector3D &position);
    void starSelected(const QString &starId, const QVector3D &position);

    void animationStarted();
    void animationFinished();

private slots:
    void updateCameraPosition();
    void updateFocus();

    // Mouse event handlers
    void mousePressed(Qt3DInput::QMouseEvent *event);
    void mouseReleased(Qt3DInput::QMouseEvent *event);
    void mouseMoved(Qt3DInput::QMouseEvent *event);



private:


    bool anyArrowKeyPressed() const;
    void adjustCameraForImmersion(bool inside);

    QVector3D calculateViewDirection(const QVector3D &currentStarPos);     // Calculate view direction based on previous star


    Qt3DRender::QCamera   *m_camera;
    Qt3DCore::QEntity     *m_rootEntity;
    BackgroundMusic       *m_bgMusic;

    QTimer               *m_cameraTimer;
    QTimer               *m_focusTimer;

    bool                  m_isAnimating;
    float                 m_elapsedTime;
    float                 m_duration;
    QEasingCurve          m_easingCurve;

    QVector3D             m_startPosition;
    QVector3D             m_targetPosition;
    QVector3D             m_startViewCenter;
    QVector3D             m_targetViewCenter;
    QVector3D             m_focusPoint;

    // Track previous star position for view direction
    QVector3D             m_previousStarPosition;
    bool                  m_hasPreviousStar;

    // Which star we are currently "inside"
    Qt3DCore::QEntity    *m_currentInsideStar;
    // The picker we removed from that star
    Qt3DRender::QObjectPicker *m_currentInsidePicker;

    // ---- Mouse-based rotation members ----
    Qt3DInput::QMouseDevice   *m_mouseDevice;
    Qt3DInput::QMouseHandler  *m_mouseHandler;


    // NEW FOR ARROW KEYS

    void onKeyPressed(Qt3DInput::QKeyEvent *event);
    void onKeyReleased(Qt3DInput::QKeyEvent *event);
    Qt3DInput::QKeyboardDevice  *m_keyboardDevice;
    Qt3DInput::QKeyboardHandler *m_keyboardHandler;
    QHash<int, bool> m_keysPressed;




    // For velocity-based rotation
    Qt3DLogic::QFrameAction* m_frameAction;

    QVector2D m_rotationVelocity;
    QVector2D m_filteredRotationVelocity; // Filtered velocity
    float m_velocityDamping;
    float m_filterStrength;               // Filter parameter
    QElapsedTimer m_deltaTimer;


    // If true, we only allow "look around" rotation
    bool  m_isInsideViewMode;

    // For in-place rotation
    bool  m_leftMouseButtonPressed;
    QPoint m_lastMousePosition;
    float  m_pitch;
    float  m_yaw;

    // The speed factor for rotation in degrees per second (based on mouse delta)
    float m_lookSpeed;

    // Whether this controller is active
    bool m_isEnabled;

};

#endif // FIRSTPERSONCAMERACONTROLLER_H
