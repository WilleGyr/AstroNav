#ifndef STARCREATOR_H
#define STARCREATOR_H

#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/Qt3DWindow>
#include <QTimer>
#include <QVector3D>
#include <QEasingCurve>
#include <Qt3DCore/QTransform>
#include <QSqlQuery>
#include <Qt3DRender/QPointLight>
#include <Qt3DExtras/QText2DEntity>
#include <QColor>

class StarCreator {
public:
    static void hoverStar(Qt3DExtras::QSphereMesh *starMesh,
                          const QVector<Qt3DCore::QEntity *> &starEntities,
                          const QVector<Qt3DExtras::QPhongMaterial *> &starMaterials,
                          Qt3DExtras::QPhongMaterial *starMaterial,
                          Qt3DExtras::QPhongMaterial *sunMaterial,
                          Qt3DExtras::QText2DEntity *starLabel);
    static void resetStar(Qt3DExtras::QSphereMesh *starMesh,
                          const QVector<Qt3DCore::QEntity *> &starEntities,
                          const QVector<Qt3DExtras::QPhongMaterial *> &starMaterials,
                          Qt3DExtras::QPhongMaterial *starMaterial,
                          const QColor colors[],
                          Qt3DExtras::QPhongMaterial *sunMaterial,
                          Qt3DExtras::QText2DEntity *starLabel);
    static void hoverSun(Qt3DExtras::QSphereMesh *sunMesh,
                         float originalSunRadius,
                         const QVector<Qt3DExtras::QPhongMaterial *> &starMaterials,
                         Qt3DExtras::QPhongMaterial *sunMaterial,
                         Qt3DExtras::QText2DEntity *sunLabel);
    static void resetSun(Qt3DExtras::QSphereMesh *sunMesh,
                         float originalSunRadius,
                         const QVector<Qt3DExtras::QPhongMaterial *> &starMaterials,
                         const QColor colors[],
                         Qt3DExtras::QPhongMaterial *sunMaterial,
                         Qt3DExtras::QText2DEntity *sunLabel);
    static void pressStar(Qt3DCore::QTransform *starTransform,
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
                          QTimer *focusTimer);
    static void pressSun(Qt3DCore::QTransform *sunTransform,
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
                         QTimer *focusTimer);
    static void createStar(QVector<Qt3DCore::QEntity *> *starEntities,
                           QVector<Qt3DExtras::QPhongMaterial *> *starMaterials,
                           QVector<QString> *starIds,
                           QVector<Qt3DExtras::QText2DEntity *> *starLabels,
                           QSqlQuery *query,
                           Qt3DCore::QEntity *rootEntity, Qt3DRender::QCamera *camera);
    static void updateLabelsOnCameraMove(const QVector<Qt3DCore::QEntity*>& starEntities,
                                         const QVector<Qt3DExtras::QText2DEntity*>& starLabels,
                                         const QVector3D& cameraPosition,
                                         const QVector3D& viewDirection,
                                         float maxAngle = 60.0f,
                                         float maxDistance = 500.0f);
    static void updateLabels(Qt3DExtras::Qt3DWindow* view,
                                      const QVector<Qt3DCore::QEntity*>& starEntities,
                                      const QVector<Qt3DExtras::QText2DEntity*>& starLabels);

    static void addGlowEffect(Qt3DCore::QEntity *starEntity, const QColor &color);
    static void updateGlowEffect(Qt3DCore::QEntity *starEntity, float intensity);

    static void addStarLight(Qt3DCore::QEntity *rootEntity,
                                          Qt3DRender::QCamera *camera,
                                          const QVector3D &position,
                                          const QString &texturePath,
                                          float starRadius);

    static QColor colorFromSpectralType(const QString &spectralType);
};

#endif // STARCREATOR_H
