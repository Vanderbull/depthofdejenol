#ifndef GAME_RESOURCES_H
#define GAME_RESOURCES_H

#include <QHash>
#include <QPixmap>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
/**
 * @brief Manages all game-related assets (images/pixmaps).
 *
 * It uses a static pattern for single-point access and lazy loading.
 */
class GameResources {
public:
    /**
     * @brief Retrieves the QPixmap associated with a given key.
     * @param key The unique string identifier for the image (e.g., "player_sprite").
     * @return The requested QPixmap. Returns a null QPixmap if not found.
     */
    static QPixmap getPixmap(const QString& key) {
        // Lazy initialization: Load resources only on the first call
        if (s_resources.isEmpty()) {
            loadResources();
        }

        // Return the pixmap (QHash::value returns a default/null value if key doesn't exist)
        return s_resources.value(key);
    }

    /**
     * @brief Explicitly triggers the loading of all game resources from the file system.
     *
     * This function should be called once during game startup.
     */
    static void loadAllResources() {
        if (s_resources.isEmpty()) {
            _loadResources();
        } else {
            qDebug() << "Game resources already loaded. Skipping loadAllResources() call.";
        }
    }

private:
    // Declaration of the static storage container (must be defined in a .cpp file)
    static QHash<QString, QPixmap> s_resources;

/**
     * @brief Internal function to handle the resource loading logic from the file system.
     */
    static void _loadResources() {
        // --- 1. Define the base path for your resources ---
        // IMPORTANT: This path is RELATIVE to where your executable is run from.
        // A common setup places the assets folder next to the executable.
        const QString RESOURCE_PATH = "resources/images/";

        QDir dir(RESOURCE_PATH);

        // Check if the directory is valid before proceeding
        if (!dir.exists()) {
            qWarning() << "CRITICAL: Resource directory not found:" << QDir::currentPath() + "/" + RESOURCE_PATH;
            return;
        }

        // --- 2. Set up filters for common image files ---
        QStringList nameFilters;
        nameFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.gif" << "*.bmp";

        QFileInfoList fileList = dir.entryInfoList(nameFilters, QDir::Files | QDir::Readable);

        // --- 3. Iterate and load each file ---
        foreach (const QFileInfo &fileInfo, fileList) {
            QString filePath = fileInfo.absoluteFilePath();
            // Use the filename WITHOUT the extension as the lookup key
            QString key = fileInfo.baseName();

            QPixmap pixmap(filePath);

            if (!pixmap.isNull()) {
                s_resources.insert(key, pixmap);
            } else {
                qWarning() << "Failed to load image:" << filePath;
            }
        }

        // --- 4. Final check and reporting ---
        if (s_resources.isEmpty()) {
            qWarning() << "No game resources were loaded from:" << RESOURCE_PATH;
        } else {
            qDebug() << "Group Successfully loaded" << s_resources.count() << "game resources from the file system.";
        }
    }

    /**
     * @brief Loads all game assets into the static hash.
     * * IMPORTANT: Use the Qt Resource System path (`:/images/file.png`) for deployment.
     */
    static void loadResources() {
        // --- YOUR IMAGE LOADING GOES HERE ---
        
        // Example paths (replace these with your actual files/resource paths)
        s_resources.insert("bank", QPixmap("resources/images/bank.png"));
        s_resources.insert("confinement", QPixmap("resources/images/confinement.png"));
        s_resources.insert("dungeon", QPixmap("resources/images/dungeon.png"));
        s_resources.insert("exit_icon", QPixmap("resources/images/exit_icon.png"));
        s_resources.insert("general_store", QPixmap("resources/images/general_store.png"));
        s_resources.insert("guilds", QPixmap("resources/images/guilds.png"));
        s_resources.insert("mordor_art", QPixmap("resources/images/mordor_art.png"));
        s_resources.insert("morgue", QPixmap("resources/images/morgue.png"));
        s_resources.insert("seer", QPixmap("resources/images/seer.png"));
        s_resources.insert("antimagic", QPixmap("resources/images/antimagic.png"));
        s_resources.insert("box", QPixmap("resources/images/box.png"));
        s_resources.insert("chest", QPixmap("resources/images/chest.png"));
        s_resources.insert("chute", QPixmap("resources/images/chute.png"));
        s_resources.insert("dig", QPixmap("resources/images/dig.png"));
        s_resources.insert("door", QPixmap("resources/images/door.png"));
        s_resources.insert("dungeonsprites", QPixmap("resources/images/dungeonsprites.png"));
        s_resources.insert("extinguisher", QPixmap("resources/images/extinguisher.png"));
        s_resources.insert("face_east", QPixmap("resources/images/faceeast.png"));
        s_resources.insert("face_north", QPixmap("resources/images/facenorth.png"));
        s_resources.insert("face_south", QPixmap("resources/images/facesouth.png"));
        s_resources.insert("face_west", QPixmap("resources/images/facewest.png"));
        s_resources.insert("filtersand", QPixmap("resources/images/filter_sand.png"));
        s_resources.insert("filterwater", QPixmap("resources/images/filter_water.png"));
        s_resources.insert("floorsand", QPixmap("resources/images/floor_sand.png"));
        s_resources.insert("floorstone", QPixmap("resources/images/floor_stone.png"));
        s_resources.insert("floorwater", QPixmap("resources/images/floor_water.png"));
        s_resources.insert("fog", QPixmap("resources/images/fog.png"));
        s_resources.insert("hostile", QPixmap("resources/images/hostile.png"));
        s_resources.insert("introtitle", QPixmap("resources/images/introtitle.png"));

        // Debug output to confirm loading
        qDebug() << "Game resources loaded:" << s_resources.count() << "items.";
        if (s_resources.value("general_store").isNull()) {
             qWarning() << "CRITICAL: 'general_store' failed to load. Check path in loadResources().";
        }
    }

    // Delete constructor/destructor to prevent instantiation (it's a static utility class)
    GameResources() = delete;
    ~GameResources() = delete;
};

#endif // GAME_RESOURCES_H
