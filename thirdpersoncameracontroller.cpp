#include "thirdpersoncameracontroller.h"

ThirdPersonCameraController::ThirdPersonCameraController(Qt3DRender::QCamera *camera, Qt3DCore::QEntity *rootEntity, BackgroundMusic *bgMusic, QObject *parent)
    : QObject(parent), m_camera(camera), m_rootEntity(rootEntity), m_bgMusic(bgMusic),
    m_isAnimating(false), m_elapsedTime(0.0f), m_focusPoint(0, 0, 0),
    m_duration(2000.0f), m_easingCurve(QEasingCurve::InOutQuad),
    m_thirdPersonOffset(0, 5, 15)
{
    m_cameraTimer = new QTimer(this);
    m_cameraTimer->setInterval(16);
    connect(m_cameraTimer, &QTimer::timeout, this, &ThirdPersonCameraController::updateCameraPosition);

    m_focusTimer = new QTimer(this);
    m_focusTimer->setInterval(16);
    connect(m_focusTimer, &QTimer::timeout, this, &ThirdPersonCameraController::updateFocus);

    // Initialize orbit controller for third-person mode
    m_orbitController = new Qt3DExtras::QOrbitCameraController(rootEntity);
    m_orbitController->setCamera(camera);
    m_orbitController->setLookSpeed(180.0f);
    m_orbitController->setLinearSpeed(50.0f);
    m_orbitController->setZoomInLimit(2.0f);
}

ThirdPersonCameraController::~ThirdPersonCameraController()
{
    // Clean up timers if active
    if (m_cameraTimer->isActive())
        m_cameraTimer->stop();

    if (m_focusTimer->isActive())
        m_focusTimer->stop();
}

void ThirdPersonCameraController::setEnabled(bool enabled)
{
    m_isEnabled = enabled;

    // Enable/disable the orbit controller
    m_orbitController->setEnabled(enabled);
}

void ThirdPersonCameraController::animateCameraToPosition(const QVector3D &targetPosition, const QVector3D &targetViewCenter)
{
    if (!m_camera || !m_isEnabled) return;

    m_startPosition = m_camera->position();
    m_targetPosition = targetPosition;
    m_startViewCenter = m_camera->viewCenter();
    m_targetViewCenter = targetViewCenter;
    m_elapsedTime = 0.0f;
    m_isAnimating = true;
    m_cameraTimer->start();
    m_focusPoint = targetViewCenter;
}

void ThirdPersonCameraController::teleportToStar(const QVector3D &coordinates, const QString &starId)
{
    if (!m_camera || !m_isEnabled) return;

    if (m_bgMusic) {
        m_bgMusic->playSoundEffect(nullptr, QString("qrc:/BackgroundMusic/star_click.wav"));
    }

    const float scaleFactor = 10.0f;
    QVector3D scaledCoords = coordinates * scaleFactor;

    // Calculate direction from current position to star
    QVector3D toStar = scaledCoords - m_camera->position();
    QVector3D direction = toStar.normalized();

    // Set target position at fixed distance from star
    QVector3D targetCameraPos = scaledCoords - direction * m_stoppingDistance;

    // Animate camera to this position
    animateCameraToPosition(targetCameraPos, scaledCoords);

    if (!starId.isEmpty()) {
        emit starTeleported(starId, scaledCoords);
    }
}

void ThirdPersonCameraController::handleStarClick(Qt3DCore::QTransform *starTransform)
{
    if (!m_camera || !starTransform || !m_isEnabled) return;

    if (m_bgMusic) {
        m_bgMusic->playSoundEffect(nullptr, QString("qrc:/BackgroundMusic/star_click.wav"));
    }

    QVector3D starPosition = starTransform->translation();

    // Calculate direction from current position to star
    QVector3D toStar = starPosition - m_camera->position();
    QVector3D direction = toStar.normalized();

    // Set target position at fixed distance from star
    QVector3D targetPosition = starPosition - direction * m_stoppingDistance;

    // Animate the camera to this position
    animateCameraToPosition(targetPosition, starPosition);
}

void ThirdPersonCameraController::handleSunClick(Qt3DCore::QTransform *sunTransform)
{
    if (!m_camera || !sunTransform || !m_isEnabled) return;

    if (m_bgMusic) {
        m_bgMusic->playSoundEffect(nullptr, QString("qrc:/BackgroundMusic/star_click.wav"));
    }

    QVector3D sunPosition = sunTransform->translation();

    // Position the camera at an offset from the sun for third-person view
    QVector3D targetPosition = sunPosition + m_thirdPersonOffset;

    // Animate the camera to this position
    animateCameraToPosition(targetPosition, sunPosition);

    emit starSelected("Sun", sunPosition);
}

void ThirdPersonCameraController::updateCameraPosition()
{
    m_elapsedTime += 16.0f;

    float t = m_easingCurve.valueForProgress(m_elapsedTime / m_duration);
    if (t >= 1.0f) {
        t = 1.0f;
        m_cameraTimer->stop();
        m_isAnimating = false;
        m_camera->setViewCenter(m_targetViewCenter);
        m_focusTimer->start();
    }

    QVector3D newPosition = m_startPosition * (1 - t) + m_targetPosition * t;
    QVector3D newViewCenter = m_startViewCenter * (1 - t) + m_targetViewCenter * t;

    m_camera->setPosition(newPosition);
    m_camera->setViewCenter(newViewCenter);
}

void ThirdPersonCameraController::updateFocus()
{
    if (!m_isAnimating && m_isEnabled) {
        m_camera->setViewCenter(m_focusPoint);
    }
}
