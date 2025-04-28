#include "includeqt.h"
#include "includepr.h"
#include <functional>
#include <Qt3DInput/QInputAspect>

/*
Function to initialize the database and run a query to the star data

Input:
- QString with the path to the database

Output:
- QSqlQuery with the star data
*/
QSqlQuery getStars (const QString &database_path) {

    //fixing clumped stars (körs bara en gång, kommenteras bort efter det)
    SeparateStars(database_path.toStdString());

    QSqlDatabase database = openDatabase(database_path.toStdString());
    if (!database.isOpen()) {
        QMessageBox::critical(nullptr, "Fatal Error", "Could not open stars database.");
        return QSqlQuery(); // Returnera tom query om misslyckad anslutning
    }

    QSqlQuery db_query(database);  // Använd rätt databas
    if (!db_query.exec("SELECT MAIN_ID, x_koord, y_koord, z_koord, SP_TYPE FROM stars")) {
        qWarning() << "Error: Query failed:" << db_query.lastError().text();
        QMessageBox::critical(nullptr, "Query Error", "Failed to retrieve star data from the database.");
        return QSqlQuery();
    }

    return db_query;
}

/*
Function to load background music

Input:
- QApplication pointer
- QString with the filename of the song (without extension, in the resource folder)
- float with the volume (0.0f to 1.0f)

Output:
- BackgroundMusic pointer
*/
BackgroundMusic* loadMusic(QApplication *app, const QString &songName, float volume) {
    BackgroundMusic *bgMusic = new BackgroundMusic(app);

    QString bgMusicFilepath = "qrc:/BackgroundMusic/" + songName + ".mp3";

    if (!bgMusic->loadMusic(bgMusicFilepath)) {
        qWarning() << "Failed to load background music";
        delete bgMusic;
        return nullptr; // Return null if loading fails
    }

    bgMusic->setVolume(volume);  // Set volume
    bgMusic->setLooping(true);   // Loop continuously

    return bgMusic;
}

/*
Function to handle creation of the 3D view
Input:
- none

Output:
- Qt3DExtras::Qt3DWindow pointer (view)
*/
Qt3DExtras::Qt3DWindow* create3DView() {
    Qt3DExtras::Qt3DWindow *view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(Qt::black));

    return view;
}

/*
Function to create window container for the 3D view

Input:
- Qt3DExtras::Qt3DWindow pointer (view)

Output:
- QWidget pointer (container)
*/
QWidget* createContainer(Qt3DExtras::Qt3DWindow *view) {
    QWidget *container = QWidget::createWindowContainer(view);
    container->setMinimumSize(800, 600);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    return container;
}

/*
Function to create the skybox

Input:
- Qt3DCore::QEntity pointer (rootEntity)

Output:
- none (void function)
*/
void createSkybox(Qt3DCore::QEntity *rootEntity) {
    Qt3DExtras::QSkyboxEntity *skybox = new Qt3DExtras::QSkyboxEntity(rootEntity);
    skybox->setBaseName("qrc:/textures/skybox");
    skybox->setExtension(".png");
    Qt3DCore::QTransform *skyboxTransform = new Qt3DCore::QTransform();
    skyboxTransform->setScale(2500.0f);
    skybox->addComponent(skyboxTransform);
}

/*
Function to create the sunlight

Input:
- Qt3DCore::QEntity pointer (sunEntity)

Output:
- none (void function)
*/
void createSunLight(Qt3DCore::QEntity *sunEntity) {
    Qt3DCore::QEntity *sunLightEntity = new Qt3DCore::QEntity(sunEntity);
    Qt3DRender::QPointLight *sunLight = new Qt3DRender::QPointLight();
    sunLight->setColor(QColor(Qt::yellow));
    sunLight->setIntensity(50.0f);
    sunLight->setConstantAttenuation(1.0f);
    sunLight->setLinearAttenuation(0.2f);
    sunLight->setQuadraticAttenuation(0.05f);
    sunLightEntity->addComponent(sunLight);
}

/*
Function to handle input for the first person camera
Input:
- QWidget pointer (container)

Output:
- none (void function)
*/
void handleFirstPersonCameraInput(QWidget *container, Qt3DExtras::Qt3DWindow *view) {
    auto inputAspect = new Qt3DInput::QInputAspect();
    view->registerAspect(inputAspect);
    container->setFocusPolicy(Qt::StrongFocus);
    container->setFocus();
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Load background music
    BackgroundMusic *bgMusic = loadMusic(&app, "BackgroundMusic", 0.3f);

    // Set the icon for the application
    app.setWindowIcon(QIcon(":/images/astronav_icon.ico"));

    // Create and show the login dialog
    LoginDialog login;

    // 3D view and container setup
    Qt3DExtras::Qt3DWindow *view = create3DView();
    QWidget *container = createContainer(view);

    // UI panels
    InfoBox *topPanel = new InfoBox();
    ActivityBox *bottomPanel = new ActivityBox();
    QWidget *rightBox = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightBox);

    qtmanager::setRightLayout(rightLayout, topPanel, bottomPanel);

    QWidget *centralWidget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    qtmanager::setMainLayout(mainLayout, rightBox, container);

    QMainWindow mainWindow;
    qtmanager::createMainWindow(mainWindow, centralWidget);
    mainWindow.hide();

    // Create root entity and skybox
    Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
    createSkybox(rootEntity);

    // Function to handle login dialog acceptance
    QObject::connect(&login, &LoginDialog::dialogAccepted, [&]() {

        bool isAdmin = bottomPanel->isAdminUser(login.getUsername());
        topPanel->setEditButtonVisibleForAdmin(isAdmin);

        // Hide the login dialog
        login.hide();

        // Log in the user
        if (login.isUserLoggedIn()) {
            bottomPanel->userLogin(true, login.getUsername());
        } else {
            bottomPanel->userLogin(false, "Guest");
        }

        // Show the main window
        mainWindow.showFullScreen();

        // Play the background music
        if (bgMusic) {
            bgMusic->play();
        }
    });

    if (login.exec() != QDialog::Accepted) {
        return 0; // Exit if login is not successful
    }

    // For first person camera keyboard
    handleFirstPersonCameraInput(container, view);

    // Connect the back to login signal
    QObject::connect(bottomPanel, &ActivityBox::backToLoginRequested, [&]() {
        // Hide the main window
        mainWindow.hide();
    
        // Show the login dialog again
        login.show();

        // Set the login input fields to be empty
        login.clearFields();
    
        // Stop the background music
        if (bgMusic) {
            bgMusic->stop();
        }
    });

    // Create the sun entity
    Qt3DCore::QEntity *sunEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DExtras::QSphereMesh *sunMesh = new Qt3DExtras::QSphereMesh();
    sunMesh->setRadius(1.5f);

    Qt3DCore::QTransform *sunTransform = new Qt3DCore::QTransform();
    sunTransform->setTranslation(QVector3D(0, 0, 0));

    Qt3DExtras::QPhongMaterial *sunMaterial = new Qt3DExtras::QPhongMaterial();
    sunMaterial->setDiffuse(QColor(Qt::yellow));
    sunMaterial->setAmbient(QColor(255, 255, 0, 150));

    sunEntity->addComponent(sunMesh);
    sunEntity->addComponent(sunTransform);
    sunEntity->addComponent(sunMaterial);

    // Set up sunlight
    createSunLight(sunEntity);

    // Sun picker
    Qt3DRender::QObjectPicker *sunPicker = new Qt3DRender::QObjectPicker(sunEntity);
    sunPicker->setHoverEnabled(true);
    sunPicker->setDragEnabled(false);
    sunEntity->addComponent(sunPicker);

    // Set up camera
    Qt3DRender::QCamera *camera = view->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0, 10, 10));
    camera->setViewCenter(QVector3D(0, 0, 0));

    // Create camera manager to manage both camera modes
    CameraManager *cameraManager = new CameraManager(view->camera(), rootEntity, bgMusic);

    // Set initial camera mode to ThirdPersonMode when program starts
    cameraManager->setCameraMode(CameraManager::ThirdPersonMode);
    cameraManager->handleSunClick(sunTransform);

    // Connect camera mode toggle signal from ActivityBox to CameraManager
    QObject::connect(bottomPanel, &ActivityBox::toggleCameraMode,
                     cameraManager, &CameraManager::toggleCameraMode);

    // Connect camera mode change signal to update the UI button
    QObject::connect(cameraManager, &CameraManager::cameraModeChanged, [bottomPanel, container](CameraManager::CameraMode mode)
                     {
                         bottomPanel->updateCameraModeButton(static_cast<int>(mode));
                         // Give focus back to the container after mode change
                         QTimer::singleShot(50, [container]() {container->setFocus();});
                     });


    // Connect sun picker to camera manager and InfoBox
    QObject::connect(sunPicker, &Qt3DRender::QObjectPicker::clicked,
                     [cameraManager, sunTransform, topPanel](Qt3DRender::QPickEvent *pick) {
                         if (pick->button() == Qt3DRender::QPickEvent::LeftButton) {
                             if(topPanel->getStarId()=="Sun"){
                                 cameraManager->handleSunClick(sunTransform);
                             }
                             topPanel->setStarInfo("Sun", "0", "0", "0", "G2V");                         
                         }
                     });

    QObject::connect(bottomPanel, &ActivityBox::teleportToStar,
                     cameraManager, &CameraManager::teleportToStar);

    // Database operations: get stars (now med SP_TYPE)
    QSqlQuery query = getStars(argv[0]);

    // Create stars from database
    QVector<Qt3DCore::QEntity *> starEntities;
    QVector<Qt3DExtras::QPhongMaterial *> starMaterials;
    QVector<QString> starIds;
    QVector<Qt3DExtras::QText2DEntity *> starLabels;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> colorDist(0, 2);

    const QColor colors[] = {Qt::red, Qt::yellow, Qt::blue};

    while (query.next()) {
        QColor starColor = colors[colorDist(gen)];
        // StarCreator::createStar använder nu även spType för att räkna ut rätt radie
        StarCreator::createStar(&starEntities, &starMaterials, &starIds, &starLabels, &query, rootEntity,camera);

    }

    // Define the label update lambda
    auto updateLabels = [view, starEntities, starLabels]() {
        StarCreator::updateLabels(view, starEntities, starLabels);
    };

    // Connect camera movements to update labels
    QObject::connect(view->camera(), &Qt3DRender::QCamera::positionChanged, updateLabels);
    QObject::connect(view->camera(), &Qt3DRender::QCamera::viewCenterChanged, updateLabels);

    // Sätt upp pickers och anslut händelser för alla stjärnor
    for (int j = 0; j < starEntities.size(); ++j) {
        Qt3DCore::QEntity *starEntity = starEntities[j];
        Qt3DCore::QTransform *starTransform = nullptr;
        Qt3DExtras::QSphereMesh *starMesh = nullptr;

        // Hämta komponenter
        for (Qt3DCore::QComponent *component : starEntity->components()) {
            if (auto mesh = qobject_cast<Qt3DExtras::QSphereMesh *>(component)) {
                starMesh = mesh;
            } else if (auto transform = qobject_cast<Qt3DCore::QTransform *>(component)) {
                starTransform = transform;
            }
        }

        Qt3DExtras::QPhongMaterial *starMaterial = starMaterials[j];

        Qt3DRender::QObjectPicker *picker = new Qt3DRender::QObjectPicker(starEntity);
        picker->setHoverEnabled(true);
        picker->setDragEnabled(false);

        // Connect picker to camera manager and InfoBox
        QObject::connect(picker, &Qt3DRender::QObjectPicker::clicked,
                         [cameraManager, starTransform, bottomPanel, topPanel, starId = starIds[j]](Qt3DRender::QPickEvent *pick) {
                             if (pick->button() == Qt3DRender::QPickEvent::LeftButton) {
                                if(topPanel->getStarId()==starId){
                                    cameraManager->handleStarClick(starTransform);
                                 }
                                 bottomPanel->setCurrentStarId(starId);
                                 bottomPanel->updateFavoriteButtonIcon();
                                
                                 QVector3D position = starTransform->translation();
                                 QSqlQuery query(QSqlDatabase::database("starsConnection"));

                                 query.prepare("SELECT SP_TYPE FROM stars WHERE MAIN_ID = ?");
                                 query.addBindValue(starId);
                                 if (query.exec() && query.next()) {
                                     QString spType = query.value(0).toString();
                                     topPanel->setStarInfo(starId,
                                                           QString::number(position.x()),
                                                           QString::number(position.y()),
                                                           QString::number(position.z()),
                                                           spType);
                                 }
                             }
                         });

        QObject::connect(picker, &Qt3DRender::QObjectPicker::entered,
                         [starMesh, starEntities, starMaterials, starMaterial, sunMaterial, starLabel = starLabels[j], &updateLabels]() {
                             StarCreator::hoverStar(starMesh, starEntities, starMaterials, starMaterial, sunMaterial, starLabel);
            // Force show label regardless of camera position
            if (starLabel) {
                starLabel->setEnabled(true);
                // Force immediate update
                QTimer::singleShot(0, updateLabels);
            }
        });

        QObject::connect(picker, &Qt3DRender::QObjectPicker::exited,
                         [starMesh, starEntities, starMaterials, starMaterial, colors, sunMaterial, starLabel = starLabels[j]]() {
                             StarCreator::resetStar(starMesh, starEntities, starMaterials, starMaterial, colors, sunMaterial, starLabel);
                         });

        starEntity->addComponent(picker);
    }

    view->setRootEntity(rootEntity);

    return app.exec();
}
