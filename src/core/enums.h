#include <QObject>

class GameEnums : public QObject
{
    Q_OBJECT // Required for the meta-object system
public:
    enum class ChestType
    {
        None = 0,
        Box = 2,
        Chest // Value is 3
    };
    Q_ENUM_NS(ChestType) // Register the enum within this class's namespace
// Define the enum directly inside the class
    enum ChestType
    {
        None = 0,
        Box = 2,
        Chest // Automatically assigned the value 3
    };
    Q_ENUM(ChestType) // Registers the enum with Qt's meta-object system
};

// Access the enum members like: GameEnums::ChestType::Box
