#include <QObject>

class TrapEnums : public QObject
{
    Q_OBJECT // Required for the Q_ENUM macro

public:
    enum TrapType
    {
        None = 0,
        Poison,
        Disease,
        Exploding,
        Fire,
        Slime,
        Fate,
        Teleport,
        Blackout,
        Fear,
        Withering
    };
    Q_ENUM(TrapType) // Registers the enum with Qt's meta-object system
};
