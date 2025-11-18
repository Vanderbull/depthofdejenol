// In your header file (e.g., SpellEnums.h)

#include <QObject>

class SpellEnums : public QObject
{
    Q_OBJECT // Necessary for the Q_ENUM macro to work

public:
    enum SpellCategory
    {
        Fire = 0,
        Cold,
        Electrical,
        Mind,
        Damage,
        Element,
        Kill,
        Charm,
        Bind,
        Heal,
        Movement,
        Banish,
        Dispell,
        Resistant,
        Visual,
        Magical,
        Location,
        Protection
    };
    Q_ENUM(SpellCategory) // Registers the enum with Qt's meta-object system
};
