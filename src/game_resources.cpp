#include "game_resources.h"
#include <QHash>
#include <QPixmap>

/**
 * @brief Definition/Initialization of the static member s_resources.
 *
 * This allocates the actual memory for the QHash. It MUST be done in ONE .cpp file.
 */
QHash<QString, QPixmap> GameResources::s_resources;
