#include "cameramanager.h"

CameraManager::CameraManager(Qt3DRender::QCamera *camera, Qt3DCore::QEntity *rootEntity,
                             BackgroundMusic *bgMusic, QObject *parent)
    : QObject(parent),
    m_camera(camera),
    m_cameraMode(ThirdPersonMode),
    m_currentStarTransform(nullptr),
    m_isViewingSun(false)
{
    // Create both controllers
    m_firstPersonController = new FirstPersonCameraController(camera, rootEntity, bgMusic, this);
    m_thirdPersonController = new ThirdPersonCameraController(camera, rootEntity, bgMusic, this);

    // Initially set to third-person mode (default)
    m_firstPersonController->setEnabled(false);
    m_thirdPersonController->setEnabled(true);

    // Connect signals from controllers to track star focus
    connect(m_firstPersonController, &FirstPersonCameraController::starTeleported,
            this, [this](const QString &starId, const QVector3D &position) {
                m_currentStarId = starId;
                m_isViewingSun = false;
                emit starTeleported(starId, position);
            });
    /*
    connect(m_firstPersonController, &FirstPersonCameraController::starSelected,
            this, [this](const QString &starId, const QVector3D &position) {
                if (starId == "Sun") {
                    m_isViewingSun = true;
                }
                emit starSelected(starId, position);
            });
    */

    connect(m_thirdPersonController, &ThirdPersonCameraController::starTeleported,
            this, [this](const QString &starId, const QVector3D &position) {
                m_currentStarId = starId;
                m_isViewingSun = false;
                emit starTeleported(starId, position);
            });
    /*
    connect(m_thirdPersonController, &ThirdPersonCameraController::starSelected,
            this, [this](const QString &starId, const QVector3D &position) {
                if (starId == "Sun") {
                    m_isViewingSun = true;
                }
                emit starSelected(starId, position);
            });
    */
}

CameraManager::~CameraManager()
{
    // Controllers are QObjects parented to this, so they'll be cleaned up automatically
}

void CameraManager::setCameraMode(CameraMode mode)
{
    if (m_cameraMode == mode)
        return;

    // Save the current mode before changing
    CameraMode previousMode = m_cameraMode;

    // Update the mode
    m_cameraMode = mode;

    if (mode == FirstPersonMode) {
        // Disable third-person first to avoid controller conflicts
        m_thirdPersonController->setEnabled(false);
        m_firstPersonController->setEnabled(true);

        // If we were already looking at a star, maintain focus by simulating a click
        if (m_currentStarTransform) {
            m_firstPersonController->handleStarClick(m_currentStarTransform);
        } else if (m_isViewingSun) {

            Qt3DCore::QEntity* rootEntity = m_camera->parentEntity()->parentEntity();
            if (rootEntity) {
                for (QObject* child : rootEntity->children()) {
                    Qt3DCore::QEntity* entity = qobject_cast<Qt3DCore::QEntity*>(child);
                    if (!entity) continue;

                    bool isSun = false;
                    Qt3DCore::QTransform* transform = nullptr;

                    for (Qt3DCore::QComponent* comp : entity->components()) {
                        if (auto tr = qobject_cast<Qt3DCore::QTransform*>(comp)) {
                            if (tr->translation() == QVector3D(0, 0, 0)) {
                                transform = tr;
                                isSun = true;
                                break;
                            }
                        }
                    }

                    if (isSun && transform) {
                        m_firstPersonController->handleSunClick(transform);
                        break;
                    }
                }
            }
        } else if (!m_currentStarId.isEmpty()) {

            m_firstPersonController->teleportToStar(QVector3D(0, 0, 0), m_currentStarId);
        }
    } else { // ThirdPersonMode
        // Disable first-person first to avoid controller conflicts
        m_firstPersonController->setEnabled(false);
        m_thirdPersonController->setEnabled(true);

        // If we were already looking at a star, maintain focus by simulating a click
        if (m_currentStarTransform) {
            m_thirdPersonController->handleStarClick(m_currentStarTransform);
        } else if (m_isViewingSun) {

            Qt3DCore::QEntity* rootEntity = m_camera->parentEntity()->parentEntity();
            if (rootEntity) {
                for (QObject* child : rootEntity->children()) {
                    Qt3DCore::QEntity* entity = qobject_cast<Qt3DCore::QEntity*>(child);
                    if (!entity) continue;

                    bool isSun = false;
                    Qt3DCore::QTransform* transform = nullptr;

                    for (Qt3DCore::QComponent* comp : entity->components()) {
                        if (auto tr = qobject_cast<Qt3DCore::QTransform*>(comp)) {
                            if (tr->translation() == QVector3D(0, 0, 0)) {
                                transform = tr;
                                isSun = true;
                                break;
                            }
                        }
                    }

                    if (isSun && transform) {
                        m_thirdPersonController->handleSunClick(transform);
                        break;
                    }
                }
            }
        } else if (!m_currentStarId.isEmpty()) {
            m_thirdPersonController->teleportToStar(QVector3D(0, 0, 0), m_currentStarId);
        }
    }

    emit cameraModeChanged(mode);
}

void CameraManager::toggleCameraMode()
{
    setCameraMode(m_cameraMode == FirstPersonMode ? ThirdPersonMode : FirstPersonMode);
}

void CameraManager::teleportToStar(const QVector3D &coordinates, const QString &starId)
{
    // Store star info for mode switching
    m_currentStarId = starId;
    m_isViewingSun = (starId.toLower() == "sun");
    m_currentStarTransform = nullptr; // Reset transform since we're teleporting

    // Forward the call to the active controller
    if (m_cameraMode == FirstPersonMode) {
        m_firstPersonController->teleportToStar(coordinates, starId);
    } else {
        m_thirdPersonController->teleportToStar(coordinates, starId);
    }
}

void CameraManager::handleStarClick(Qt3DCore::QTransform *starTransform)
{
    // Store the star transform for mode switching
    m_currentStarTransform = starTransform;
    m_isViewingSun = false;

    // Forward the call to the active controller
    if (m_cameraMode == FirstPersonMode) {
        m_firstPersonController->handleStarClick(starTransform);
    } else {
        m_thirdPersonController->handleStarClick(starTransform);
    }
}
