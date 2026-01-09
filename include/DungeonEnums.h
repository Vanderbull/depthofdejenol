#include <QObject>
#include <QtGlobal> // Contains QFlags definition

// Define a namespace to contain your enums/flags (Good practice)
namespace Dungeon 
{

    // The enum class defines the individual bits
    enum class DungeonTileFlag
    {
        None             = 0,
        WallEast         = 1 << 0,  // 0x00000001
        WallNorth        = 1 << 1,  // 0x00000002
        DoorEast         = 1 << 2,  // 0x00000004
        DoorNorth        = 1 << 3,  // 0x00000008
        SecretDoorEast   = 1 << 4,  // 0x00000010
        SecretDoorNorth  = 1 << 5,  // 0x00000020
        FaceNorth        = 1 << 6,  // 0x00000040
        FaceEast         = 1 << 7,  // 0x00000080
        FaceSouth        = 1 << 8,  // 0x00000100
        FaceWest         = 1 << 9,  // 0x00000200
        Extinguisher     = 1 << 10, // 0x00000400
        Pit              = 1 << 11, // 0x00000800
        StairsUp         = 1 << 12, // 0x00001000
        StairsDown       = 1 << 13, // 0x00002000
        Teleporter       = 1 << 14, // 0x00004000
        Water            = 1 << 15, // 0x00008000
        Quicksand        = 1 << 16, // 0x00010000
        Rotator          = 1 << 17, // 0x00020000
        Antimagic        = 1 << 18, // 0x00040000
        Rock             = 1 << 19, // 0x00080000
        Fog              = 1 << 20, // 0x00100000
        Chute            = 1 << 21, // 0x00200000
        Stud             = 1 << 22, // 0x00400000
        Explored         = 1 << 23  // 0x00800000
    };

    // This macro defines the DungeonTileFlags type, which is a QFlags<DungeonTileFlag>
    Q_DECLARE_FLAGS(DungeonTileFlags, DungeonTileFlag)

} // namespace Dungeon
