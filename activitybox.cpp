#include "activitybox.h"
#include "ui_activitybox.h"
#include <QMessageBox>
#include <QIcon>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QListWidget>
#include <QMessageBox>
#include <QInputDialog>

// Set the stylesheet for give button
void ActivityBox::setButtonImage(QPushButton* button, const QString& normalPath, const QString& pressedPath) {
    QString style = QString(
                        "QPushButton {"
                        "   border: none;"
                        "   border-image: url(%1) 0 0 0 0 stretch stretch;"
                        "}"
                        "QPushButton:pressed {"
                        "   border-image: url(%3) 0 0 0 0 stretch stretch;"
                        "}")
                        .arg(normalPath)
                        .arg(pressedPath);

    button->setStyleSheet(style);
};

// Shows given list of widgets.
void ActivityBox::showWidgets(const QList<QWidget*>& widgets) {
    for (QWidget* widget : widgets) {
        widget->setVisible(true);   // Make the widget visible
    }
}

// Hides given list of widgets.
void ActivityBox::hideWidgets(const QList<QWidget*>& widgets) {
    for (QWidget* widget : widgets) {
        widget->setVisible(false);  // Make the widget hidden
    }
}

// Visually marks the selected menu button as pressed and resets others.
void ActivityBox::setMenuButtonPressed(QPushButton* pressedButton)
{
    // Map each menu button to its corresponding icon base path
    QMap<QPushButton*, QString> buttonMap = {
        { ui->usersMenuButton, USER_MENU },
        { ui->favoriteMenuButton, FAVO },
        { ui->helpMenuButton, HELP_MENU },
        { ui->searchMenuButton, SEARCH_MENU }
    };

    // Reset all buttons to the "not pressed" state
    for (auto it = buttonMap.begin(); it != buttonMap.end(); ++it) {
        setButtonImage(it.key(), it.value() + NOT_PRESSED, it.value() + MID_PRESS);
    }

    // Set the selected button to the "pressed" state
    if (buttonMap.contains(pressedButton)) {
        setButtonImage(pressedButton, buttonMap[pressedButton] + PRESSED, buttonMap[pressedButton] + MID_PRESS);
    }
}

// Initialize the static database connection
QSqlDatabase ActivityBox::m_starsDb = QSqlDatabase::database("starsConnection");

ActivityBox::ActivityBox(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ActivityBox)
{
    ui->setupUi(this);

    // Makes lists with items to resize and hide
    QList<QAbstractButton*> widgetsToResize = {ui->usersMenuButton, ui->favoriteMenuButton, ui->helpMenuButton, ui->searchMenuButton, ui->deleteButton, ui->favoriteButton, ui->quitButton, ui->BackToLogin, ui->sunButton, ui->passwordButton};
    QList<QWidget*> widgetsToHide = {ui->favouritesLabel, ui->Regisrationlable, ui->favoriteList, ui->usersList, ui->deleteButton, ui->favoriteButton, ui->passwordButton, ui->helpLabel, ui->searchButton, ui->Help1_label, ui->Help2_label, ui->Help3_label, ui->Help4_label, ui->Help5_label, ui->Help6_label, ui->Icon_1, ui->Icon_2, ui->Icon_3, ui->Icon_4, ui->Icon_5};

    // Assign widgets to lists for each menu
    SearchWidgets = {ui->massMax, ui->massMin, ui->searchButton, ui->typeBox, ui->searchLineEdit, ui->massLabel, ui->typeLabel};
    UsersWidgets = {ui->usersList, ui->Regisrationlable, ui->deleteButton, ui->passwordButton};
    FavoriteWidgets = {ui->favouritesLabel, ui->favoriteList, ui->favoriteButton};
    HelpWidgets = {ui->helpLabel, ui->Help1_label, ui->Help2_label, ui->Help3_label, ui->Help4_label, ui->Help5_label, ui->Help6_label, ui->Icon_1, ui->Icon_2, ui->Icon_3, ui->Icon_4, ui->Icon_5};

    // Hide every widget in the list
    for (QWidget* widget : widgetsToHide) {
        widget->setVisible(false);
    }

    // Resize every widget in the list
    for (QAbstractButton* button : widgetsToResize) {
        button->setIconSize(QSize(60, 60));
    }

    searchById("Sun");

    // Connect all buttons to functions
    connect(ui->quitButton, &QPushButton::clicked, this, &ActivityBox::onQuitButtonClicked);    // Quit
    connect(ui->BackToLogin, &QPushButton::clicked, this, &ActivityBox::onBackToLoginClicked);  // Back To Login
    connect(ui->sunButton, &QPushButton::clicked, this, &ActivityBox::onSunButtonClicked);  // Back To Sun
    connect(ui->searchMenuButton, &QPushButton::clicked, this, &ActivityBox::onSearchMenuButtonClicked);    // Search Menu
    connect(ui->searchButton, &QPushButton::clicked, this, &ActivityBox::onSearchButtonClicked);    // Search
    connect(ui->favoriteMenuButton, &QPushButton::clicked, this, &ActivityBox::onFavoriteMenuButtonClicked);    // Favorite Menu
    connect(ui->favoriteButton, &QPushButton::clicked, this, &ActivityBox::onFavoriteButtonClicked);    // Favorite
    connect(ui->usersMenuButton, &QPushButton::clicked, this, &ActivityBox::onUsersMenuButtonClicked);  // Users Menu
    connect(ui->deleteButton, &QPushButton::clicked, this, &ActivityBox::deleteSelectedUser);    // Delete User
    connect(ui->passwordButton, &QPushButton::clicked, this, &ActivityBox::onPasswordButtonClicked);    // Change Password
    connect(ui->helpMenuButton, &QPushButton::clicked, this, &ActivityBox::onHelpMenuButtonClicked);// Help Menu
    connect(ui->searchResultsList, &QListWidget::itemDoubleClicked, this, &ActivityBox::onSearchResultsDoubleClicked);

    // Connect single and double click in favorite list
    connect(ui->favoriteList, &QListWidget::itemClicked, this, &ActivityBox::onFavoriteItemSingleClicked);  // Favorite List Single Click
    connect(ui->favoriteList, &QListWidget::itemDoubleClicked, this, &ActivityBox::onFavoriteItemDoubleClicked);    // Favorite List Double Click

    // Connect the toggle camera mode button
    connect(ui->toggleCameraModeButton, &QPushButton::clicked, this, &ActivityBox::onToggleCameraModeClicked);
    connect(ui->typeBox, &QComboBox::currentTextChanged,
            this, &ActivityBox::onTypeBoxChanged);

    // Set the initial button images
    setButtonImage(ui->usersMenuButton, USER_MENU+NOT_PRESSED, USER_MENU+MID_PRESS);
    setButtonImage(ui->favoriteMenuButton, FAVO+NOT_PRESSED, FAVO+MID_PRESS);
    setButtonImage(ui->helpMenuButton, HELP_MENU+NOT_PRESSED, HELP_MENU+MID_PRESS);
    setButtonImage(ui->searchMenuButton, SEARCH_MENU+NOT_PRESSED, SEARCH_MENU+MID_PRESS);

    QObject::connect(ui->searchLineEdit, &QLineEdit::textChanged, [=](const QString &text){
        ui->searchButton->setVisible(!text.trimmed().isEmpty());
    });
}

ActivityBox::~ActivityBox()
{
    delete ui;
}

// Emits signal when button is clicked
void ActivityBox::onToggleCameraModeClicked()
{
    emit toggleCameraMode();
}

// Updates the cameramode depending on selected mode
void ActivityBox::updateCameraModeButton(int mode)
{
    // Update button text based on camera mode
    if (mode == 0) { // First-person mode
        ui->toggleCameraModeButton->setText("Switch to Third-Person View");
    } else { // Third-person mode
        ui->toggleCameraModeButton->setText("Switch to First-Person View");
    }
}

// Updates infobox to current stars info
void ActivityBox::setCurrentStarId(const QString& starId) {
    currentStarId = starId;
    qDebug() << "Current star ID set to:" << currentStarId;
}

// Goes back to login screen
void ActivityBox::onBackToLoginClicked()
{
    // Confirm with user
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Logout",
                                  "Return to login screen?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        emit backToLoginRequested();
    }
}

void ActivityBox::onSearchButtonClicked()
{
    // Get the text from the search bar
    QString searchText = ui->searchLineEdit->text();

    // Call your search function with the search text
    searchById(searchText);
}

// Switches to search menu
void ActivityBox::onSearchMenuButtonClicked(){
    // Hide/show widgets
    showWidgets(SearchWidgets);
    hideWidgets(UsersWidgets);
    hideWidgets(FavoriteWidgets);
    hideWidgets(HelpWidgets);

    ui->searchButton->setVisible(false);

    // Set menu button states
    setMenuButtonPressed(ui->searchMenuButton);
}

// Searches for a star by its ID and moves the view to its coordinates if found.
void ActivityBox::searchById(const QString &id) {
    // Check for empty input
    if (id.isEmpty()) {
        QMessageBox::warning(this, "Search Error", "Please enter a star ID");
        return;
    }

    // Special case: If searching for the Sun, teleport to origin
    if (id.compare("sun", Qt::CaseInsensitive) == 0) {
        emit teleportToStar(QVector3D(0, 0, 0), "Sun");
        setCurrentStarId("Sun");
        return;
    }

    // Get the existing open connection to the stars database
    QSqlDatabase db = QSqlDatabase::database("starsConnection");

    // Prepare a query to retrieve the star's coordinates by ID
    QSqlQuery query(db);
    query.prepare("SELECT x_koord, y_koord, z_koord FROM stars WHERE MAIN_ID = ?");
    query.addBindValue(id);

    // Execute the query and check for errors
    if (!query.exec()) {
        QMessageBox::warning(this, "Query Error",
                             "Failed to execute query: " + query.lastError().text());
        return;
    }

    // If a match is found, extract coordinates and teleport to the star
    if (query.next()) {
        QVector3D starCoordinates(
            query.value(0).toFloat(),
            query.value(1).toFloat(),
            query.value(2).toFloat()
            );
        emit teleportToStar(starCoordinates, id);  // Emit both coordinates and ID
        setCurrentStarId(id);  // Update the current star ID
    } else {
        // If no match found, inform the user
        QMessageBox::information(this, "Search Result", "No star found with ID: " + id);
    }

    // Update favorite icon based on whether this star is favorited
    updateFavoriteButtonIcon();

}

// Toggles the favorite status of the selected or current star for the logged-in user.
void ActivityBox::onFavoriteButtonClicked()
{
    // Determine which star ID to use: selected from list or current
    QString starId;
    if (ui->favoriteList->selectedItems().isEmpty()) {
        starId = currentStarId;
    } else {
        starId = ui->favoriteList->selectedItems().first()->text();
    }


    // Get the current star ID and username
    QString username = loggedInUsername;  // set during login

    // Obtain the database connection (using a named connection "usersConnection")
    QSqlDatabase db = QSqlDatabase::database("usersConnection");
    if (!db.isOpen()) {
        QMessageBox::warning(this, "Database Error", "Database connection is not open.");
        return;
    }

    // Check if the star is already favorited by this user
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM favourites WHERE username = :username AND favourite_id = :starId");
    query.bindValue(":username", username);
    query.bindValue(":starId", starId);
    if (!query.exec()) {
        QMessageBox::warning(this, "Database Error", "Failed to check favourite status: " + query.lastError().text());
        return;
    }
    bool alreadyFavorited = false;
    if (query.next()) {
        alreadyFavorited = query.value(0).toInt() > 0;
    }

    // Toggle the favorite status
    if (alreadyFavorited) {
        // Remove the favorite entry from the database
        QSqlQuery deleteQuery(db);
        deleteQuery.prepare("DELETE FROM favourites WHERE username = :username AND favourite_id = :starId");
        deleteQuery.bindValue(":username", username);
        deleteQuery.bindValue(":starId", starId);
        if (!deleteQuery.exec()) {
            QMessageBox::warning(this, "Database Error", "Failed to remove favourite: " + deleteQuery.lastError().text());
        } else {
            // Update icon and local list on successful removal
            ui->favoriteButton->setIcon(QIcon(":/icons/favorite_empty.png"));
            favoritesList.removeOne(starId);
        }
    } else {
        // Insert a new favorite entry into the database
        QSqlQuery insertQuery(db);
        insertQuery.prepare("INSERT INTO favourites (username, favourite_id) VALUES (:username, :starId)");
        insertQuery.bindValue(":username", username);
        insertQuery.bindValue(":starId", starId);
        if (!insertQuery.exec()) {
            QMessageBox::warning(this, "Database Error", "Failed to add favourite: " + insertQuery.lastError().text());
        } else {
            // Update icon and local list on successful addition
            ui->favoriteButton->setIcon(QIcon(":/icons/favorite_filled.png"));
            favoritesList.append(starId);
        }
    }

    // Refresh the favorite list shown in the UI
    updateFavoriteList();

    // Update the favorite button icon based on current state
    updateFavoriteButtonIcon();
}

// Updates the favorite list
void ActivityBox::updateFavoriteList()
{
    // Clear the current list
    ui->favoriteList->clear();

    // Add each favorited star to the list with a dash
    for (const QString &starId : favoritesList) {
        ui->favoriteList->addItem(starId);
    }
}

// Switches to user list menu
void ActivityBox::onUsersMenuButtonClicked(){
    // Hide/show widgets
    showWidgets(UsersWidgets);
    hideWidgets(SearchWidgets);
    hideWidgets(FavoriteWidgets);
    hideWidgets(HelpWidgets);

    // Set menu button states
    setMenuButtonPressed(ui->usersMenuButton);
}

// Returns true if the given username belongs to an admin user.
bool ActivityBox::isAdminUser(const QString& username)
{
    // "Guest" is never an admin
    if (username == "Guest"){
        return false;
    }

    // Get the database connection for users
    QSqlDatabase db = QSqlDatabase::database("usersConnection");

    // If the database isn't open, show an error and return false
    if (!db.isOpen()) {
        QMessageBox::warning(this, "Database Error", "Database connection is not open.");
        return false;
    }

    // Prepare and execute a query to retrieve the 'Admin' flag for the user
    QSqlQuery query(db);
    query.prepare("SELECT Admin FROM users WHERE USERNAME = :username");
    query.bindValue(":username", username);

    // Handle query execution failure
    if (!query.exec()) {
        qWarning() << "Database query failed:" << query.lastError().text();
        return false;
    }

    // If a matching user was found, return their admin status (true/false)
    if (query.next()) {
        return query.value(0).toBool(); // Return true if Admin is 1 (true), false otherwise
    }

    // If user not found, return false
    return false;
}

// Logs in the user and updates UI elements based on their role (user or admin).
void ActivityBox::userLogin(bool user, QString username)
{
    loggedInUsername = username; // Store the username
    bool isAdmin = isAdminUser(username); // Check admin status

    // Show or hide the favorite menu button based on whether the user is a regular user
    if (user) {
        ui->favoriteMenuButton->setVisible(true);
    } else {
        ui->favoriteMenuButton->setVisible(false);
    }

    // Show or hide the users menu button based on admin privileges
    if (isAdmin) {
        ui->usersMenuButton->setVisible(true);
    } else {
        ui->usersMenuButton->setVisible(false);
    }

    // If the user is an admin, fetch all usernames from the database
    if (isAdmin == true){
        // Connect to the users database
        QSqlDatabase db = QSqlDatabase::database("usersConnection");

        // Check if the database is open
        if (!db.isOpen()) {
            QMessageBox::warning(this, "Database Error", "Database connection is not open.");
            return;
        }

        // Prepare a query to select all usernames from the users table
        QSqlQuery query(db);
        query.prepare("SELECT USERNAME FROM users");  // Select all usernames

        // Execute the query and handle any errors
        if (!query.exec()) {
            qWarning() << "Database query failed:" << query.lastError().text();
            QMessageBox::warning(this, "Query Error", "Failed to retrieve users.");
            return;
        }

        // Clear any existing entries in the users list UI component
        ui->usersList->clear();

        // Loop through the results and add each username to the UI list
        while (query.next()) {
            QString username = query.value(0).toString();
            ui->usersList->addItem(username);
        }
    }

    // Hide the users list by default (it may be shown later depending on app logic)
    ui->usersList->setVisible(false);

}

// Tries to delete selected user
void ActivityBox::deleteSelectedUser() {
    QListWidgetItem *item = ui->usersList->currentItem();
    if (!item) {
        qDebug() << "No user selected.";
        return;
    }

    QString username = item->text();
    qDebug() << "Attempting to delete user:" << username;

    // Prevent deleting the currently logged-in user
    if (username == loggedInUsername) {
        QMessageBox errorBox;
        errorBox.setWindowTitle("Error");
        errorBox.setText("You cannot delete the currently logged-in user.");
        errorBox.setIcon(QMessageBox::Warning);
        errorBox.setWindowIcon(QIcon(":/icons/warning_icon.png"));  // Set custom warning icon
        errorBox.exec();
        return;
    }

    // Get the existing database connection
    QSqlDatabase db = QSqlDatabase::database("usersConnection");

    if (!db.isOpen()) {
        qDebug() << "Error: Database connection is not open.";
        QMessageBox errorBox;
        errorBox.setWindowTitle("Database Error");
        errorBox.setText("Could not open the database.");
        errorBox.setIcon(QMessageBox::Critical);
        errorBox.setWindowIcon(QIcon(":/icons/error_icon.png"));
        errorBox.exec();
        return;
    }

    // Prepare the DELETE query
    QSqlQuery query(db);
    query.prepare("DELETE FROM users WHERE USERNAME = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error deleting user:" << query.lastError().text();
        QMessageBox errorBox;
        errorBox.setWindowTitle("Error");
        errorBox.setText("Failed to delete the user.");
        errorBox.setIcon(QMessageBox::Critical);
        errorBox.setWindowIcon(QIcon(":/icons/error_icon.png"));
        errorBox.exec();
    } else {
        qDebug() << "User deleted successfully!";
        delete item;  // Remove the item from QListWidget

        // Show success popup
        QMessageBox::information(this, "User Deleted", "User '" + username + "' has been successfully deleted.");
    }

}

// Switches to the favorite menu
void ActivityBox::onFavoriteMenuButtonClicked(){
    // Hide/show widgets
    showWidgets(FavoriteWidgets);
    hideWidgets(SearchWidgets);
    hideWidgets(UsersWidgets);
    hideWidgets(HelpWidgets);

    // Set menu button states
    setMenuButtonPressed(ui->favoriteMenuButton);

    loadUserFavorites();

    updateFavoriteButtonIcon();

}

// Shuts down the program when the quit buttn is clicked
void ActivityBox::onQuitButtonClicked() {
    // Confirm with user
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Exit",
                                  "Exit programm?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QApplication::quit();
    }
}

// Loads the current users favorites into the favorite list
void ActivityBox::loadUserFavorites()
{
    // Clear previously loaded favorites
    favoritesList.clear();
    ui->favoriteList->clear();

    // Connect to the users database
    QSqlDatabase db = QSqlDatabase::database("usersConnection");
    if (!db.isOpen()) {
        QMessageBox::warning(this, "Database Error", "Database connection is not open.");
        return;
    }

    // Prepare a query to get all favorite star IDs for the current user
    QSqlQuery query(db);
    query.prepare("SELECT favourite_id FROM favourites WHERE username = :username");
    query.bindValue(":username", loggedInUsername);

    // Execute the query and handle errors
    if (!query.exec()) {
        QMessageBox::warning(this, "Database Error", "Failed to retrieve favorites: " + query.lastError().text());
        return;
    }

    // Loop through the results and add each favorite to the list and UI
    while (query.next()) {
        QString starId = query.value(0).toString();
        favoritesList.append(starId);   // Store in internal list
        ui->favoriteList->addItem(starId);  // Show in UI
    }
}

// Teleports to star when double clicked in favorite list
void ActivityBox::onFavoriteItemDoubleClicked(QListWidgetItem* item)
{
    if (!item) return;

    QString starId = item->text(); // Each item in the list is just a star ID
    searchById(starId);
}

// Updates the add/remove favorite button when selecting a star in the favorite list
void ActivityBox::onFavoriteItemSingleClicked(QListWidgetItem* item)
{
    if (!item) return;

    QString starId = item->text(); // Each item in the list is just a star ID
    if (favoritesList.contains(starId)) {
        setButtonImage(ui->favoriteButton, MAKE_FAVO+PRESSED, MAKE_FAVO+"(MidPressRemove).png");
    } else {
        setButtonImage(ui->favoriteButton, MAKE_FAVO+NOT_PRESSED, MAKE_FAVO+"(MidPressAdd).png");
    }
}

// Updates the add/remove favorite button depending on if the star is a favorite or not
void ActivityBox::updateFavoriteButtonIcon()
{
    if (favoritesList.contains(currentStarId)) {
        setButtonImage(ui->favoriteButton, MAKE_FAVO+PRESSED, MAKE_FAVO+"(MidPressRemove).png");
    } else {
        setButtonImage(ui->favoriteButton, MAKE_FAVO+NOT_PRESSED, MAKE_FAVO+"(MidPressAdd).png");
    }
}

// When change password button is clicked
void ActivityBox::onPasswordButtonClicked(){
    // Get the currently selected user from the users list
    QListWidgetItem *selectedItem = ui->usersList->currentItem();
    if (!selectedItem) {
        // Show warning if no user is selected
        QMessageBox::warning(this, "No User Selected", "Please select a user to change their password.");
        return;
    }

    QString username = selectedItem->text();

    // Prompt the admin to enter a new password
    bool ok;
    QString newPassword = QInputDialog::getText(
        this,
        "Change Password",
        "Enter a new password for " + username + ":",
        QLineEdit::Password,
        "",
        &ok
        );

    // If the input was cancelled or empty, abort
    if (!ok || newPassword.isEmpty()) {
        return; // Cancelled or empty
    }

    // Connect to the users database
    QSqlDatabase db = QSqlDatabase::database("usersConnection");
    if (!db.isOpen()) {
        QMessageBox::warning(this, "Database Error", "Database connection is not open.");
        return;
    }

    // Prepare and execute the update query
    QSqlQuery query(db);
    query.prepare("UPDATE users SET PASSWORD = :newPassword WHERE USERNAME = :username");
    query.bindValue(":newPassword", newPassword);
    query.bindValue(":username", username);

    // Show result based on success or failure of the query
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "Failed to update password: " + query.lastError().text());
    } else {
        QMessageBox::information(this, "Password Changed", "Password updated successfully for " + username + ".");
    }
}

// Switches to the help menu
void ActivityBox::onHelpMenuButtonClicked(){
    // Hide/show widgets
    showWidgets(HelpWidgets);
    hideWidgets(SearchWidgets);
    hideWidgets(UsersWidgets);
    hideWidgets(FavoriteWidgets);

    // Set menu button states
    setMenuButtonPressed(ui->helpMenuButton);

}

// Teleports back to sun
void ActivityBox::onSunButtonClicked(){
    searchById("Sun");
    updateFavoriteButtonIcon();
}

void ActivityBox::searchByType(const QString &typeFilter)
{
    QSqlDatabase db = QSqlDatabase::database("starsConnection");
    if (!db.isOpen()) {
        QMessageBox::warning(this, "Database Error", "Database connection is not open.");
        return;
    }

    QSqlQuery query(db);
    QString pattern;

    // Skapa mönster baserat på valt filter
    if (typeFilter == "O" || typeFilter == "B" || typeFilter == "A" ||
        typeFilter == "F" || typeFilter == "G" || typeFilter == "K" || typeFilter == "M") {
        pattern = typeFilter + "%";
    }
    else if (typeFilter == "Supergiant (I)") {
        pattern = "%I";
    }
    else if (typeFilter == "Bright Giant (II)") {
        pattern = "%II";
    }
    else if (typeFilter == "Giant (III)") {
        pattern = "%III";
    }
    else if (typeFilter == "Subgiant (IV)") {
        pattern = "%IV";
    }
    else {
        // Om inget giltigt filter, hämta alla stjärnor
        pattern = "%";
    }

    query.prepare("SELECT MAIN_ID, x_koord, y_koord, z_koord, SP_TYPE FROM stars WHERE SP_TYPE LIKE ?");
    query.addBindValue(pattern);

    if (!query.exec()) {
        QMessageBox::warning(this, "Query Error", "Failed to execute query: " + query.lastError().text());
        return;
    }

    // Rensa tidigare sökresultat och se till att listwidgeten syns
    ui->searchResultsList->clear();
    ui->searchResultsList->setVisible(true);

    bool foundAny = false;
    while (query.next()) {
        QString starId = query.value(0).toString();
        QString spType = query.value(4).toString();
        // Bygg en beskrivande text, t.ex. "StarID (spType)"
        QString displayText = starId + " (" + spType + ")";
        ui->searchResultsList->addItem(displayText);
        foundAny = true;
    }

    if (!foundAny) {
        QMessageBox::information(this, "Search Result", "No star found with type: " + typeFilter);
    }
}

void ActivityBox::onSearchResultsDoubleClicked(QListWidgetItem* item)
{
    QString text = item->text();
    int parenIndex = text.indexOf(" (");
    QString starId = (parenIndex > 0) ? text.left(parenIndex) : text;
    searchById(starId);
}

void ActivityBox::onTypeBoxChanged(const QString &text)
{
    QString trimmed = text.trimmed();

    // Hoppa över tomt val eller platshållaren ”Type”
    if (trimmed.isEmpty() || trimmed == "Type")
        return;

    // Nollställ eventuell ID‑sökning
    ui->searchLineEdit->clear();

    // Kör filtreringen direkt
    searchByType(trimmed);
}
