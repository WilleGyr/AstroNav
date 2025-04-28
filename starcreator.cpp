#include "starcreator.h"
#include "databasehandler.h"
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QCamera>
#include <QTimer>
#include <QVector3D>
#include <QEasingCurve>
#include <Qt3DCore/QTransform>
#include <qtext2dentity.h>
#include <QFont>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QTextureImage>
#include <Qt3DExtras/QTextureMaterial>

/*
 * Returnerar en färg enligt Wikipedia-tabellen för O, B, A, F, G, K, M.
 * Exempel på hex-koderna:
 *  O: #9bb0ff (blå)
 *  B: #aabfff (bluish white)
 *  A: #cad8ff (white)
 *  F: #f8f7ff (yellowish white)
 *  G: #fff4ea (gul)
 *  K: #ffd2a1 (ljus orange)
 *  M: #ffcc6f (ljus röd-orange)
 */
 QColor StarCreator::colorFromSpectralType(const QString &spType)
{
    // Hämta första bokstaven (O, B, A, F, G, K, M) och gör den versal
    QChar c = spType.isEmpty() ? 'G' : spType[0].toUpper();

    switch (c.toLatin1()) {
    case 'O': return QColor(0x9bb0ff); // blå
    case 'B': return QColor(0xaabfff); // bluish white
    case 'A': return QColor(0xcad8ff); // white
    case 'F': return QColor(0xf8f7ff); // yellowish white
    case 'G': return QColor(0xfff4ea); // gul
    case 'K': return QColor(0xffd2a1); // ljus orange
    case 'M': return QColor(0xffcc6f); // ljus röd-orange
    default:  return QColor(Qt::white); // fallback
    }
}

/*
 * Beräknar stjärnans radie baserat på dess spektraltyp.
 * Justera värdena efter behov om du vill ha andra storleksförhållanden.
 */
static float getStarRadius(const QString &spType) {
    float baseRadius = 1.5f;  // Exempel: standard för G-stjärnor

    if (spType.startsWith("O", Qt::CaseInsensitive))
        baseRadius = 2.5f;
    else if (spType.startsWith("B", Qt::CaseInsensitive))
        baseRadius = 2.0f;
    else if (spType.startsWith("A", Qt::CaseInsensitive))
        baseRadius = 1.8f;
    else if (spType.startsWith("F", Qt::CaseInsensitive))
        baseRadius = 1.6f;
    else if (spType.startsWith("G", Qt::CaseInsensitive))
        baseRadius = 1.5f;
    else if (spType.startsWith("K", Qt::CaseInsensitive))
        baseRadius = 1.3f;
    else if (spType.startsWith("M", Qt::CaseInsensitive))
        baseRadius = 1.0f;

    // Skala upp om stjärnan är jätte/superjätte (III, II, I)
    if (spType.contains("III", Qt::CaseInsensitive))
        baseRadius *= 3.0f;
    else if (spType.contains("II", Qt::CaseInsensitive))
        baseRadius *= 4.0f;
    else if (spType.contains("I", Qt::CaseInsensitive))
        baseRadius *= 5.0f;

    return baseRadius;
}

void StarCreator::hoverStar(Qt3DExtras::QSphereMesh *starMesh,
                            const QVector<Qt3DCore::QEntity *> &starEntities,
                            const QVector<Qt3DExtras::QPhongMaterial *> &starMaterials,
                            Qt3DExtras::QPhongMaterial *starMaterial,
                            Qt3DExtras::QPhongMaterial *sunMaterial,
                            Qt3DExtras::QText2DEntity *starLabel)
{
    float originalRadius = starMesh->radius();
    starMesh->setRadius(originalRadius * 1.5f);

    // Visa etiketten
    if (starLabel) {
        starLabel->setEnabled(true);
    }

    // Mörklägg övriga stjärnor + solen
    for (int i = 0; i < starEntities.size(); ++i) {
        if (starMaterials[i] != starMaterial) {
            starMaterials[i]->setDiffuse(QColor(Qt::black));
            starMaterials[i]->setAmbient(QColor(Qt::black));
            sunMaterial->setDiffuse(QColor(Qt::black));
            sunMaterial->setAmbient(QColor(Qt::black));
        }
    }
}

void StarCreator::resetStar(Qt3DExtras::QSphereMesh *starMesh,
                            const QVector<Qt3DCore::QEntity *> &starEntities,
                            const QVector<Qt3DExtras::QPhongMaterial *> &starMaterials,
                            Qt3DExtras::QPhongMaterial *starMaterial,
                            const QColor colors[],
                            Qt3DExtras::QPhongMaterial *sunMaterial,
                            Qt3DExtras::QText2DEntity *starLabel)
{
    float originalRadius = starMesh->radius() / 1.5f;
    starMesh->setRadius(originalRadius);

    // Dölj etiketten
    if (starLabel) {
        starLabel->setEnabled(false);
    }

    // Återställ färgerna på övriga stjärnor + solen
    for (int i = 0; i < starEntities.size(); ++i) {
        if (starMaterials[i] != starMaterial) {
            // OBS: Om du vill återställa exakt rätt färg för var och en
            // måste du antingen spara dess ursprungliga färg i en separat struktur,
            // eller köra "colorFromSpectralType" igen för respektive stjärna.
            // Här används en enkel fallback (colors[]).
            starMaterials[i]->setDiffuse(colors[i % 3]);
            starMaterials[i]->setAmbient(colors[i % 3].lighter(50));
            sunMaterial->setDiffuse(QColor(Qt::yellow));
            sunMaterial->setAmbient(QColor(Qt::yellow));
        }
    }
}

void StarCreator::hoverSun(Qt3DExtras::QSphereMesh *sunMesh,
                           float originalSunRadius,
                           const QVector<Qt3DExtras::QPhongMaterial *> &starMaterials,
                           Qt3DExtras::QPhongMaterial *sunMaterial,
                           Qt3DExtras::QText2DEntity *sunLabel)
{
    sunMesh->setRadius(originalSunRadius * 1.5f);

    // Visa solens etikett
    sunLabel->setEnabled(true);

    // Mörklägg stjärnorna
    for (int i = 0; i < starMaterials.size(); ++i) {
        starMaterials[i]->setDiffuse(QColor(Qt::black));
        starMaterials[i]->setAmbient(QColor(Qt::black));
    }
}

void StarCreator::resetSun(Qt3DExtras::QSphereMesh *sunMesh,
                           float originalSunRadius,
                           const QVector<Qt3DExtras::QPhongMaterial *> &starMaterials,
                           const QColor colors[],
                           Qt3DExtras::QPhongMaterial *sunMaterial,
                           Qt3DExtras::QText2DEntity *sunLabel)
{
    sunMesh->setRadius(originalSunRadius);

    // Dölj solens etikett
    sunLabel->setEnabled(false);

    // Återställ färgerna på stjärnorna
    for (int i = 0; i < starMaterials.size(); ++i) {
        starMaterials[i]->setDiffuse(colors[i % 3]);
        starMaterials[i]->setAmbient(colors[i % 3].lighter(50));
    }

    sunMaterial->setDiffuse(QColor(Qt::yellow));
    sunMaterial->setAmbient(QColor(Qt::yellow));
}

void StarCreator::pressStar(Qt3DCore::QTransform *starTransform,
                            Qt3DExtras::Qt3DWindow *view,
                            QTimer *cameraTimer,
                            bool &isAnimating,
                            QVector3D &startPosition,
                            QVector3D &targetPosition,
                            QVector3D &startViewCenter,
                            QVector3D &targetViewCenter,
                            float &elapsedTime,
                            float duration,
                            QEasingCurve &easingCurve,
                            QTimer *focusTimer)
{
    if (isAnimating) {
        cameraTimer->stop();
    }
    focusTimer->stop();

    QVector3D starPosition = starTransform->translation();
    QVector3D cameraPosition = view->camera()->position();
    QVector3D direction = starPosition - cameraPosition;

    float distanceFromStar = 7.5f;
    direction.normalize();
    QVector3D offset = direction * distanceFromStar;

    startPosition = cameraPosition;
    targetPosition = starPosition - offset;
    startViewCenter = view->camera()->viewCenter();
    targetViewCenter = starPosition;
    elapsedTime = 0.0f;
    isAnimating = true;

    cameraTimer->start();
}

void StarCreator::pressSun(Qt3DCore::QTransform *sunTransform,
                           Qt3DExtras::Qt3DWindow *view,
                           QTimer *cameraTimer,
                           bool &isAnimating,
                           QVector3D &startPosition,
                           QVector3D &targetPosition,
                           QVector3D &startViewCenter,
                           QVector3D &targetViewCenter,
                           float &elapsedTime,
                           float duration,
                           QEasingCurve &easingCurve,
                           QTimer *focusTimer)
{
    if (isAnimating) {
        cameraTimer->stop();
    }
    focusTimer->stop();

    QVector3D sunPosition = sunTransform->translation();
    QVector3D cameraPosition = view->camera()->position();
    QVector3D direction = sunPosition - cameraPosition;

    float distanceFromSun = 7.5f;
    direction.normalize();
    QVector3D offset = direction * distanceFromSun;

    startPosition = cameraPosition;
    targetPosition = sunPosition - offset;
    startViewCenter = view->camera()->viewCenter();
    targetViewCenter = sunPosition;
    elapsedTime = 0.0f;
    isAnimating = true;

    cameraTimer->start();
}

void StarCreator::updateLabelsOnCameraMove(
    const QVector<Qt3DCore::QEntity*>& starEntities,
    const QVector<Qt3DExtras::QText2DEntity*>& starLabels,
    const QVector3D& cameraPosition,
    const QVector3D& viewDirection,
    float maxAngle,
    float maxDistance)
{
    for (int i = 0; i < starEntities.size(); ++i) {
        if (!starLabels[i]) continue;

        // Get star transform
        Qt3DCore::QTransform* starTransform = nullptr;
        Q_FOREACH(Qt3DCore::QComponent* component, starEntities[i]->components()) {
            if ((starTransform = qobject_cast<Qt3DCore::QTransform*>(component)))
                break;
        }

        if (!starTransform) continue;

        QVector3D starPos = starTransform->translation();
        float distance = (cameraPosition - starPos).length();

        // Get label transform
        Qt3DCore::QTransform* labelTransform = nullptr;
        Q_FOREACH(Qt3DCore::QComponent* component, starLabels[i]->components()) {
            if ((labelTransform = qobject_cast<Qt3DCore::QTransform*>(component)))
                break;
        }

        if (labelTransform) {
            // Adaptive label positioning
            float labelDistance = qMax(3.0f, distance * 0.3f);
            QVector3D labelPos = starPos + QVector3D(0, 1, 0).normalized() * labelDistance;

            if (distance < 15.0f) {
                QVector3D toCamera = (cameraPosition - starPos).normalized();
                QVector3D up = QVector3D(0, 1, 0);
                up = up - QVector3D::dotProduct(up, toCamera) * toCamera;
                up.normalize();

                labelTransform->setRotation(QQuaternion::fromDirection(toCamera, up));
            } else {
                labelTransform->setRotation(QQuaternion::fromDirection(
                    (cameraPosition - labelPos).normalized(),
                    QVector3D(0, 1, 0)
                    ));
            }

            labelTransform->setTranslation(labelPos);

            float desiredScreenSize = 0.003f;
            float scale = qBound(0.1f, desiredScreenSize * distance, 1.0f);
            labelTransform->setScale(scale);
        }

        // Visibility calculation
        QVector3D toStar = (starPos - cameraPosition).normalized();
        float angle = qRadiansToDegrees(qAcos(QVector3D::dotProduct(viewDirection, toStar)));
        starLabels[i]->setEnabled((angle < maxAngle) && (distance < maxDistance));
    }
}

void StarCreator::updateLabels(Qt3DExtras::Qt3DWindow* view,
                                        const QVector<Qt3DCore::QEntity*>& starEntities,
                                        const QVector<Qt3DExtras::QText2DEntity*>& starLabels) {
    Qt3DRender::QCamera* camera = view->camera();
    QVector3D cameraPos = camera->position();
    QVector3D cameraUp = camera->upVector();

    for (int i = 0; i < starEntities.size(); ++i) {
        Qt3DExtras::QText2DEntity* label = starLabels[i];
        if (!label || !label->isEnabled()) continue;

        // Get star transform
        Qt3DCore::QTransform* starTransform = nullptr;
        for (Qt3DCore::QComponent* component : starEntities[i]->components()) {
            if ((starTransform = qobject_cast<Qt3DCore::QTransform*>(component))) {
                break;
            }
        }
        if (!starTransform) continue;

        QVector3D starPos = starTransform->translation();
        QVector3D labelPos = starPos + QVector3D(0, 1.0f, 0); // Slightly above the star

        // Calculate distance from camera to star
        float distance = (cameraPos - starPos).length();

        // New scale formula to make text appear the same size on screen
        float desiredScreenSize = 0.003f; // Adjust this value as needed
        float scale = qBound(0.1f, desiredScreenSize * distance, 1.0f);

        // Calculate label orientation (always facing the camera)
        QVector3D toCamera = (cameraPos - labelPos).normalized();
        QVector3D right = QVector3D::crossProduct(toCamera, cameraUp).normalized();
        QVector3D up = QVector3D::crossProduct(right, toCamera).normalized();
        QQuaternion rotation = QQuaternion::fromDirection(toCamera, up);

        // Get label transform
        Qt3DCore::QTransform* labelTransform = nullptr;
        for (Qt3DCore::QComponent* component : label->components()) {
            if ((labelTransform = qobject_cast<Qt3DCore::QTransform*>(component))) {
                break;
            }
        }

        if (labelTransform) {
            labelTransform->setTranslation(labelPos);
            labelTransform->setRotation(rotation);
            labelTransform->setScale(scale);
        }
    }
}

void StarCreator::addStarLight(Qt3DCore::QEntity *rootEntity,
                               Qt3DRender::QCamera *camera,
                               const QVector3D &position,
                               const QString &texturePath,
                               float starRadius)
{
    // Create entity and components
    auto *billboard = new Qt3DCore::QEntity(rootEntity);
    auto *plane = new Qt3DExtras::QPlaneMesh();
    auto *transform = new Qt3DCore::QTransform();
    auto *material = new Qt3DExtras::QTextureMaterial();

    // Calculate PNG size as 2x star radius
    float pngSize = starRadius * 2.0f;
    plane->setWidth(pngSize);
    plane->setHeight(pngSize);

    transform->setTranslation(position);
    material->setAlphaBlendingEnabled(true);

    // Setup texture
    auto *texture = new Qt3DRender::QTexture2D(material);
    auto *textureImage = new Qt3DRender::QTextureImage(texture);
    textureImage->setSource(QUrl(texturePath));
    texture->addTextureImage(textureImage);
    material->setTexture(texture);

    // Add components
    billboard->addComponent(plane);
    billboard->addComponent(transform);
    billboard->addComponent(material);

    // Update function
    auto updateBillboard = [=]() {
        QVector3D toCamera = (camera->position() - position).normalized();
        transform->setRotation(QQuaternion::rotationTo(QVector3D(0, 1, 0), toCamera));

        // Optional: Additional distance-based scaling if desired
        float distance = camera->position().distanceToPoint(position);
        float distanceScale = qBound(0.5f, distance * 0.1f, 3.0f);
        transform->setScale(distanceScale);
    };

    // Initial update and connections
    updateBillboard();
    QObject::connect(camera, &Qt3DRender::QCamera::positionChanged, billboard, updateBillboard);
    QObject::connect(camera, &Qt3DRender::QCamera::viewCenterChanged, billboard, updateBillboard);
}

/*
 * Skapar en stjärna baserat på radie och färg som härleds från stjärnans spektraltyp (spType).
 * Förutsätter att din SQL-fråga returnerar:
 *   0: MAIN_ID
 *   1: x_koord
 *   2: y_koord
 *   3: z_koord
 *   4: SP_TYPE
 */
void StarCreator::createStar(QVector<Qt3DCore::QEntity *> *starEntities,
                             QVector<Qt3DExtras::QPhongMaterial *> *starMaterials,
                             QVector<QString> *starIds,
                             QVector<Qt3DExtras::QText2DEntity *> *starLabels,
                             QSqlQuery *query,
                             Qt3DCore::QEntity *rootEntity, Qt3DRender::QCamera* camera)
{
    // Hämta data från databasen
    QString mainId = query->value(0).toString();
    float x = query->value(1).toFloat();
    float y = query->value(2).toFloat();
    float z = query->value(3).toFloat();
    QString spType = query->value(4).toString();

    // Skala koordinaterna för att passa din 3D-scen
    float scaleFactor = 15.0f;
    x *= scaleFactor;
    y *= scaleFactor;
    z *= scaleFactor;
    QVector3D starPosition(x, y, z);

    // Beräkna radien och färg
    float calculatedRadius = getStarRadius(spType);
    QColor starColor = colorFromSpectralType(spType);

    // Skapa en entitet för stjärnan
    Qt3DCore::QEntity *starEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DExtras::QSphereMesh *starMesh = new Qt3DExtras::QSphereMesh();
    starMesh->setRadius(calculatedRadius);

    Qt3DCore::QTransform *starTransform = new Qt3DCore::QTransform();
    starTransform->setTranslation(starPosition);

    Qt3DExtras::QPhongMaterial *starMaterial = new Qt3DExtras::QPhongMaterial();
    starMaterial->setDiffuse(starColor);
    starMaterial->setAmbient(starColor.lighter(50));

    // Koppla ihop allt
    starEntity->addComponent(starMesh);
    starEntity->addComponent(starTransform);
    starEntity->addComponent(starMaterial);

    // Lägg till en ljuskälla
    Qt3DCore::QEntity *starLightEntity = new Qt3DCore::QEntity(starEntity);
    Qt3DRender::QPointLight *starLight = new Qt3DRender::QPointLight();
    starLight->setColor(starColor);
    starLight->setIntensity(30.0f);
    starLight->setConstantAttenuation(1.0f);
    starLight->setLinearAttenuation(0.2f);
    starLightEntity->addComponent(starLight);

    // Skapa en etikett (text) för stjärnan
    Qt3DExtras::QText2DEntity *textEntity = new Qt3DExtras::QText2DEntity(rootEntity);
    textEntity->setText(mainId);
    textEntity->setFont(QFont("Arial", 8, QFont::Bold));
    textEntity->setHeight(20);
    textEntity->setWidth(mainId.length() * 20);

    QColor labelColor(173, 216, 230); // ljus blå för texten
    labelColor.setAlpha(127); // gör texten transparent

    textEntity->setColor(labelColor);
    textEntity->setEnabled(false);  // gömd i början

    // skappar matrial för labeln
    Qt3DExtras::QPhongMaterial *labelMaterial = new Qt3DExtras::QPhongMaterial();
    labelMaterial->setDiffuse(labelColor);
    labelMaterial->setAmbient(labelColor.lighter(50)); // Ambient är lite mörkare

    // lägger till matrialet till texten
    textEntity->addComponent(labelMaterial);

    // Placera etiketten en bit ovanför stjärnan
    Qt3DCore::QTransform *labelTransform = new Qt3DCore::QTransform();
    labelTransform->setTranslation(starPosition + QVector3D(0, 1.0f, 0));
    labelTransform->setRotation(QQuaternion::fromDirection(
        QVector3D(0, 0, 1),  // fram
        QVector3D(0, 1, 0)   // up
        ));

    textEntity->addComponent(labelTransform);

    float starRadius = getStarRadius(spType);
    addStarLight(rootEntity, camera, starPosition, "qrc:/glow/starLight2.png", starRadius);

    // Spara referenser
    starEntities->append(starEntity);
    starMaterials->append(starMaterial);
    starIds->append(mainId);
    starLabels->append(textEntity);
}
