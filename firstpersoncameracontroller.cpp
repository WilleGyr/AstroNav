#include "firstpersoncameracontroller.h"

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DCore/QEntity>
#include <QtMath>
#include <QTimer>
#include <Qt3DInput/QKeyEvent>


FirstPersonCameraController::FirstPersonCameraController(Qt3DRender::QCamera *camera,
                                                         Qt3DCore::QEntity  *rootEntity,
                                                         BackgroundMusic    *bgMusic,
                                                         QObject *parent)
    : QObject(parent)
    , m_camera(camera)
    , m_rootEntity(rootEntity)
    , m_bgMusic(bgMusic)
    , m_cameraTimer(new QTimer(this))
    , m_focusTimer(new QTimer(this))
    , m_isAnimating(false)
    , m_elapsedTime(0.0f)
    , m_duration(2000.0f)
    , m_easingCurve(QEasingCurve::OutCubic )
    , m_previousStarPosition(0, 0, 0)
    , m_hasPreviousStar(false)
    , m_currentInsideStar(nullptr)
    , m_currentInsidePicker(nullptr)
    , m_mouseDevice(nullptr)
    , m_mouseHandler(nullptr)
    , m_frameAction(nullptr)
    , m_isInsideViewMode(false)
    , m_leftMouseButtonPressed(false)
    , m_pitch(0.0f)
    , m_yaw(0.0f)
    , m_lookSpeed(180.0f) //  mouse movement
    , m_isEnabled(false)
{


    // Connect camera animation updates
    m_cameraTimer->setInterval(16);
    connect(m_cameraTimer, &QTimer::timeout, this, &FirstPersonCameraController::updateCameraPosition);

    m_focusTimer->setInterval(16);
    connect(m_focusTimer, &QTimer::timeout,  this, &FirstPersonCameraController::updateFocus);



    // ----- Set up our input for rotation -----
    if (rootEntity) {
        m_mouseDevice = new Qt3DInput::QMouseDevice(rootEntity);
        m_mouseHandler = new Qt3DInput::QMouseHandler(rootEntity);
        m_mouseHandler->setSourceDevice(m_mouseDevice);

        connect(m_mouseHandler, &Qt3DInput::QMouseHandler::pressed,this, &FirstPersonCameraController::mousePressed);
        connect(m_mouseHandler, &Qt3DInput::QMouseHandler::released,this, &FirstPersonCameraController::mouseReleased);
        connect(m_mouseHandler, &Qt3DInput::QMouseHandler::positionChanged,this, &FirstPersonCameraController::mouseMoved);
    }


    // Set up our keyboard device/handler for arrow keys
    if (rootEntity) {
        m_keyboardDevice = new Qt3DInput::QKeyboardDevice(rootEntity);
        m_keyboardHandler = new Qt3DInput::QKeyboardHandler(rootEntity);
        m_keyboardHandler->setSourceDevice(m_keyboardDevice);

        // Set focus policy to ensure it gets keyboard events
        m_keyboardHandler->setFocus(true);


        connect(m_keyboardHandler, &Qt3DInput::QKeyboardHandler::pressed,this, [this](Qt3DInput::QKeyEvent *event)
                {
                    this->onKeyPressed(event);
                });

        connect(m_keyboardHandler, &Qt3DInput::QKeyboardHandler::released,this, [this](Qt3DInput::QKeyEvent *event)
                {
                    this->onKeyReleased(event);
                });
    }


    // Initialize arrow keys to "not pressed"
    m_keysPressed[Qt::Key_Left]  = false;
    m_keysPressed[Qt::Key_Right] = false;
    m_keysPressed[Qt::Key_Up]    = false;
    m_keysPressed[Qt::Key_Down]  = false;

    // Initialize pitch/yaw from the camera's existing orientation
    if (m_camera) {
        QVector3D dir = m_camera->viewVector().normalized();
        // how far we look up/down
        m_pitch = qRadiansToDegrees(-asinf(dir.y()));
        //  compass angle
        m_yaw = qRadiansToDegrees(atan2f(-dir.x(), -dir.z()));
    }

    // to fix the camera lag
    m_rotationVelocity = QVector2D(0.0f, 0.0f);
    m_filteredRotationVelocity = QVector2D(0.0f, 0.0f);
    m_velocityDamping = 10.0f; // control how quickly rotation slows down
    m_filterStrength = 0.7f;  //  0.0 (no filtering) and 0.95 (heavy filtering)
    m_deltaTimer.start();


    // Create a frame action for smooth velocity-based updates
    m_frameAction = new Qt3DLogic::QFrameAction(m_rootEntity);
    connect(m_frameAction, &Qt3DLogic::QFrameAction::triggered,
            this, &FirstPersonCameraController::onFrameUpdate);

    m_rootEntity->addComponent(m_frameAction);

}




//  the frame update method:

void FirstPersonCameraController::onFrameUpdate(float dt)
{
    if (!m_camera || !m_isEnabled || !m_isInsideViewMode)
        return;

    if (dt <= 0.0f) {
        dt = m_deltaTimer.elapsed() / 1000.0f;
        m_deltaTimer.restart();
    }

    // delta time to prevent large jumps
    dt = qMin(dt, 0.02f); // Cap at ~50fps equivalent

    // Check if any arrow keys are pressed
    bool arrowKeysActive = m_keysPressed[Qt::Key_Left] || m_keysPressed[Qt::Key_Right] ||
                           m_keysPressed[Qt::Key_Up] || m_keysPressed[Qt::Key_Down];

    // Determine rotation speed based on keys or mouse
    float arrowRotationSpeed = 75.0f; // Degrees per second - adjust as needed

    if (arrowKeysActive) {
        // Calculate keyboard-based rotation changes
        if (m_keysPressed[Qt::Key_Right]) {
            m_yaw -= arrowRotationSpeed * dt;
        }
        if (m_keysPressed[Qt::Key_Left]) {
            m_yaw += arrowRotationSpeed * dt;
        }
        if (m_keysPressed[Qt::Key_Down]) {
            m_pitch += arrowRotationSpeed * dt;
        }
        if (m_keysPressed[Qt::Key_Up]) {
            m_pitch -= arrowRotationSpeed * dt;
        }

        // Reset mouse velocity to prevent interference when using keyboard
        m_rotationVelocity = QVector2D(0.0f, 0.0f);
        m_filteredRotationVelocity = QVector2D(0.0f, 0.0f);
    }
    else {
        // No keyboard input, process mouse velocity
        if (!m_leftMouseButtonPressed) {
            // Apply velocity damping when not actively moving the mouse
            m_rotationVelocity *= qMax(0.0f, 1.0f - m_velocityDamping * dt);

            // If velocity is very small, stop completely
            if (m_rotationVelocity.lengthSquared() < 0.0001f) {
                m_rotationVelocity = QVector2D(0.0f, 0.0f);
            }
        }

        // Apply low-pass filter to velocity for smooth mouse movement
        m_filteredRotationVelocity = m_filteredRotationVelocity * m_filterStrength +
                                     m_rotationVelocity * (1.0f - m_filterStrength);

        // Apply rotation based on filtered velocity
        float sensitivity = m_lookSpeed * 0.0005f;
        m_yaw += m_filteredRotationVelocity.x() * sensitivity;
        m_pitch -= m_filteredRotationVelocity.y() * sensitivity;
    }

    // Bound pitch to prevent flipping
    m_pitch = qBound(-85.0f, m_pitch, 85.0f);

    // Convert degrees to radians
    float yawRad = qDegreesToRadians(m_yaw);
    float pitchRad = qDegreesToRadians(m_pitch);

    // Compute new direction vector
    QVector3D direction(
        -sinf(yawRad) * cosf(pitchRad),
        -sinf(pitchRad),
        -cosf(yawRad) * cosf(pitchRad)
        );

    // Set new view direction
    m_camera->setViewCenter(m_camera->position() + direction);
    m_focusPoint = m_camera->viewCenter();
}







void FirstPersonCameraController::setEnabled(bool enabled)
{
    if (m_isEnabled == enabled)
        return;

    m_isEnabled = enabled;

    if (enabled) {

        m_isInsideViewMode = true;

        // Initialize pitch/yaw from current camera orientation
        if (m_camera) {
            QVector3D dir = m_camera->viewVector().normalized();
            m_pitch = qRadiansToDegrees(-asinf(dir.y()));
            m_yaw = qRadiansToDegrees(atan2f(-dir.x(), -dir.z()));
        }
    } else {
        //
        enablePickingOnCurrentStar();

        // Reset view mode
        m_isInsideViewMode = false;

        // Stop any active timers
        if (m_cameraTimer->isActive())
            m_cameraTimer->stop();

        if (m_focusTimer->isActive())
            m_focusTimer->stop();
    }
}








// Calculate a view direction based on previous star position
QVector3D FirstPersonCameraController::calculateViewDirection(const QVector3D &currentStarPos)
{
    QVector3D viewDirection;

    if (m_hasPreviousStar) {
        // Calculate direction away from previous star
        viewDirection = (currentStarPos - m_previousStarPosition).normalized();

        // If direction is nearly zero (same position), use a default direction
        if (viewDirection.length() < 0.01f) {
            viewDirection = QVector3D(0, 0, -1); // Default to looking along -Z
        }
    } else {
        // No previous star, use a default direction
        viewDirection = QVector3D(0, 0, -1);
    }

    return viewDirection;
}

// Animate from current to target position/viewCenter
void FirstPersonCameraController::animateCameraToPosition(const QVector3D &targetPosition,
                                                          const QVector3D &targetViewCenter)
{
    if (!m_camera || !m_isEnabled) return;

    m_startPosition   = m_camera->position();
    m_targetPosition  = targetPosition;
    m_startViewCenter = m_camera->viewCenter();
    m_targetViewCenter = targetViewCenter;

    // Calculate and update pitch/yaw based on the target view direction
    QVector3D viewDir = (targetViewCenter - targetPosition).normalized();

    // Calculate pitch (up/down angle) from the view direction
    m_pitch = qRadiansToDegrees(-asinf(viewDir.y()));

    // Calculate yaw (left/right angle) from the view direction
    m_yaw = qRadiansToDegrees(atan2f(-viewDir.x(), -viewDir.z()));

    // Reset velocity for smooth transition
    m_rotationVelocity = QVector2D(0.0f, 0.0f);
    m_filteredRotationVelocity = QVector2D(0.0f, 0.0f);

    m_elapsedTime = 0.0f;
    m_isAnimating = true;

    emit animationStarted();
    m_cameraTimer->start();

    m_focusPoint = targetViewCenter;
}



void FirstPersonCameraController::updateCameraPosition()
{
    m_elapsedTime += 16.0f; // each tick ~16ms
    float t = m_easingCurve.valueForProgress(m_elapsedTime / m_duration);

    if (t >= 1.0f) {
        t = 1.0f;
        m_cameraTimer->stop();
        m_isAnimating = false;
        m_camera->setViewCenter(m_targetViewCenter);

        m_focusTimer->start();
        emit animationFinished();
    }

    QVector3D newPos = m_startPosition   * (1.0f - t) + m_targetPosition   * t;
    QVector3D newCtr = m_startViewCenter * (1.0f - t) + m_targetViewCenter * t;
    m_camera->setPosition(newPos);
    m_camera->setViewCenter(newCtr);
}

void FirstPersonCameraController::updateFocus()
{
    if (!m_isAnimating && m_isEnabled) {
        // Force camera to keep looking at the final focus point
        m_camera->setViewCenter(m_focusPoint);
    }
}

// If "inside star" is enabled, these mouse events just rotate the camera in place
void FirstPersonCameraController::mousePressed(Qt3DInput::QMouseEvent *event)
{
    if (!m_isEnabled) return;

    if (event->button() == Qt3DInput::QMouseEvent::LeftButton) {
        m_leftMouseButtonPressed = true;

        // Re-sync pitch & yaw to the camera's current orientation
        // in order to prevent jump camera jump in the begining of mouse rotaion
        if (m_camera) {
            QVector3D dir = m_camera->viewVector().normalized();
            m_pitch = qRadiansToDegrees(-asinf(dir.y()));
            m_yaw   = qRadiansToDegrees(atan2f(-dir.x(), -dir.z()));
        }

        // Reset velocities so we don’t get a sudden jump
        m_rotationVelocity         = QVector2D(0.0f, 0.0f);
        m_filteredRotationVelocity = QVector2D(0.0f, 0.0f);

        // Update last mouse position
        m_lastMousePosition = QPoint(event->x(), event->y());
    }
}


void FirstPersonCameraController::mouseReleased(Qt3DInput::QMouseEvent *event)
{
    if (!m_isEnabled) return;

    if (event->button() == Qt3DInput::QMouseEvent::LeftButton) {
        m_leftMouseButtonPressed = false;
    }
}

void FirstPersonCameraController::mouseMoved(Qt3DInput::QMouseEvent *event)
{
    if (!m_camera || !m_isEnabled || !m_isInsideViewMode)
        return;

    if (!m_leftMouseButtonPressed)
        return;

    QPoint currentPos(event->x(), event->y());
    QPoint delta = currentPos - m_lastMousePosition;
    m_lastMousePosition = currentPos;

    // Scale factor can be adjusted to change sensitivity
    float scaleFactor = 0.8f;

    // Update the raw velocity based on mouse movement
    m_rotationVelocity = QVector2D(delta.x(), delta.y()) * scaleFactor;
}



void FirstPersonCameraController::adjustCameraForImmersion(bool inside)
{
    if (!m_camera) return;

    if (inside) {
        m_camera->lens()->setPerspectiveProjection(
            m_camera->lens()->fieldOfView(),
            m_camera->lens()->aspectRatio(),
            0.01f,  // extremely close near plane
            1000.0f
            );
    } else {
        m_camera->lens()->setPerspectiveProjection(
            m_camera->lens()->fieldOfView(),
            m_camera->lens()->aspectRatio(),
            0.1f,   // normal near plane
            1000.0f
            );
    }
}

void FirstPersonCameraController::handleStarClick(Qt3DCore::QTransform *starTransform)
{
    if (!m_camera || !starTransform || !m_isEnabled)
        return;

    // Re-enable picking on the old star
    enablePickingOnCurrentStar();


    if (m_bgMusic) {
        m_bgMusic->playSoundEffect(nullptr,
                                   QStringLiteral("qrc:/BackgroundMusic/star_click.wav"));
    }


    QVector3D starPos = starTransform->translation();

    // Update previous star position if we have one
    if (m_isInsideViewMode) {
        // Set previous position to current star's position
        m_previousStarPosition = m_camera->position();
        m_hasPreviousStar = true;
    }

    // Identify the star entity
    Qt3DCore::QEntity* starEntity = nullptr;
    for (QObject* parent = starTransform->parent(); parent; parent = parent->parent()) {
        if ((starEntity = qobject_cast<Qt3DCore::QEntity*>(parent))) {
            break;
        }
    }

    // disable picking so that star doesn't block future picks
    if (starEntity) {
        disablePickingOnStar(starEntity);
    }

    // "inside" a star => can rotate in place
    m_isInsideViewMode = true;
    m_focusTimer->stop();  // Stop the timer so it won't reset the view

    // Adjust clipping
    adjustCameraForImmersion(true);

    // Calculate view direction based on previous star
    QVector3D viewDirection = calculateViewDirection(starPos);

    // Animate the camera to the star's position and look in calculated direction
    animateCameraToPosition(starPos, starPos + viewDirection);
}

void FirstPersonCameraController::handleSunClick(Qt3DCore::QTransform *sunTransform)
{
    if (!m_camera || !sunTransform || !m_isEnabled)
        return;

    enablePickingOnCurrentStar();

    if (m_bgMusic) {
        m_bgMusic->playSoundEffect(nullptr,
                                   QStringLiteral("qrc:/BackgroundMusic/star_click.wav"));
    }

    QVector3D sunPos = sunTransform->translation();

    // Update previous star position if we have one
    if (m_isInsideViewMode) {
        // Set previous position to current star's position
        m_previousStarPosition = m_camera->position();
        m_hasPreviousStar = true;
    }

    Qt3DCore::QEntity* sunEntity = nullptr;
    for (QObject* parent = sunTransform->parent(); parent; parent = parent->parent()) {
        if ((sunEntity = qobject_cast<Qt3DCore::QEntity*>(parent))) {
            break;
        }
    }

    if (sunEntity) {
        disablePickingOnStar(sunEntity);
    }

    // We are now inside the sun
    m_isInsideViewMode = true;

    adjustCameraForImmersion(true);

    // Calculate view direction based on previous star
    QVector3D viewDirection = calculateViewDirection(sunPos);

    animateCameraToPosition(sunPos, sunPos + viewDirection);

    emit starSelected(QStringLiteral("Sun"), sunPos);
}

// Remove star's picker
void FirstPersonCameraController::disablePickingOnStar(Qt3DCore::QEntity* starEntity)
{
    if (!starEntity) return;

    m_currentInsideStar   = starEntity;
    m_currentInsidePicker = nullptr;

    const auto comps = starEntity->components();
    for (Qt3DCore::QComponent* c : comps) {
        if (auto picker = qobject_cast<Qt3DRender::QObjectPicker*>(c)) {
            starEntity->removeComponent(picker);
            m_currentInsidePicker = picker;
            break;
        }
    }
}

// Re-add the removed picker
void FirstPersonCameraController::enablePickingOnCurrentStar()
{
    if (!m_currentInsideStar) return;

    if (m_currentInsidePicker) {
        m_currentInsideStar->addComponent(m_currentInsidePicker);
        m_currentInsidePicker = nullptr;
    }
    m_currentInsideStar = nullptr;
}


void FirstPersonCameraController::teleportToStar(const QVector3D &coordinates,
                                                 const QString &starId)
{
    if (!m_camera || !m_isEnabled) return;

    // Not inside star => re-enable picking on old star
    enablePickingOnCurrentStar();

    if (m_bgMusic) {
        m_bgMusic->playSoundEffect(nullptr,
                                   QStringLiteral("qrc:/BackgroundMusic/star_click.wav"));
    }

    const float scaleFactor = 10.0f;
    QVector3D scaled = coordinates * scaleFactor;

    // Update previous star position if we have one
    if (m_isInsideViewMode) {
        // Set previous position to current star's position
        m_previousStarPosition = m_camera->position();
        m_hasPreviousStar = true;
    }

    // Find that star entity
    Qt3DCore::QEntity* starEntity = nullptr;
    if (m_camera->parentEntity()) {
        Qt3DCore::QEntity* rootEntity =
            qobject_cast<Qt3DCore::QEntity*>(m_camera->parentEntity()->parentEntity());
        if (rootEntity) {
            for (QObject* child : rootEntity->children()) {
                Qt3DCore::QEntity* e = qobject_cast<Qt3DCore::QEntity*>(child);
                if (!e) continue;
                for (Qt3DCore::QComponent* comp : e->components()) {
                    if (auto tr = qobject_cast<Qt3DCore::QTransform*>(comp)) {
                        if (tr->translation() == scaled) {
                            starEntity = e;
                            break;
                        }
                    }
                }
                if (starEntity) break;
            }
        }
    }

    if (starEntity) {
        disablePickingOnStar(starEntity);
    }

    // We're now inside view mode again
    m_isInsideViewMode = true;
    adjustCameraForImmersion(true);

    // Calculate view direction based on previous star
    QVector3D viewDirection = calculateViewDirection(scaled);

    // Animate camera there, looking in the direction away from previous star
    animateCameraToPosition(scaled, scaled + viewDirection);

    if (!starId.isEmpty()) {
        emit starTeleported(starId, scaled);
    }
}






void FirstPersonCameraController::onKeyPressed(Qt3DInput::QKeyEvent *event)
{
    if (!m_isEnabled) return;

    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        // Just set the key as pressed and accept the event
        m_keysPressed[event->key()] = true;
        event->setAccepted(true);
        break;
    default:
        break;
    }
}




void FirstPersonCameraController::onKeyReleased(Qt3DInput::QKeyEvent *event)
{
    if (!m_isEnabled) return;

    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        m_keysPressed[event->key()] = false;
        event->setAccepted(true);
        break;
    default:
        break;
    }
}

