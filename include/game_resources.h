#ifndef GAME_RESOURCES_H
#define GAME_RESOURCES_H

#include <QHash>
#include <QPixmap>
#include <QString>
#include <QDebug>

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

private:
    // Declaration of the static storage container (must be defined in a .cpp file)
    static QHash<QString, QPixmap> s_resources;

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
