// Companion.h
class Companion : public QObject {
    Q_OBJECT
public:
    enum class Status { Active, Wayfaring, Exhausted, Rogue, Deceased };
    Q_ENUM(Status)

    QString id;
    QString name;
    int morale = 100;
    int loyalty = 50;
    Status currentStatus = Status::Wayfaring;

    void updateTick(); // Runs independently of player input
signals:
    void moraleUpdated(int newMorale);
};
