// In your header file (e.g., DoorEnums.h)

#include <QObject>

class DoorEnums : public QObject
{
    Q_OBJECT // Required for the Q_ENUM macro

public:
    enum LockedState
    {
        NotLocked = -1,
        Unlocked = 0,
        Locked = 1
    };
    Q_ENUM(LockedState) // Registers the enum with Qt's meta-object system
};
