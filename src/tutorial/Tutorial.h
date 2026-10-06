#ifndef TUTORIAL_H
#define TUTORIAL_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>

// One step in the guided first dungeon run.
struct TutorialStep {
    int index = 0;
    QString id;
    QString title;
    QString instruction;
    // What completes this step.
    QString completesOn;  // "move", "fight", "stairs", "rest", "spell", "pickup", "door", "level"
    int targetValue = 0;  // e.g. floor level for "level"
};

// Guided first dungeon run: walks the player through movement, combat,
// spells, rest, stairs and the first boss.
class Tutorial : public QObject {
    Q_OBJECT

public:
    explicit Tutorial(QObject *parent = nullptr);
    ~Tutorial() override;

    // All tutorial steps.
    static QList<TutorialStep> steps();

    // The step at an index, or an invalid step (index -1) when out of range.
    static TutorialStep step(int index);

    // Is the tutorial active?
    static bool isActive();

    // Start the tutorial.
    static void start();

    // Stop the tutorial.
    static void stop();

    // Current step index, or -1 when not active.
    static int currentStepIndex();

    // Current step, or an invalid step when not active.
    static TutorialStep currentStep();

    // Advance to the next step. Returns false when the tutorial is done.
    static bool advance();

    // Report an event that may complete the current step.
    // Returns true when the event advanced the tutorial.
    static bool reportEvent(const QString& event, int value = 0);

    // Reset all tutorial state (for testing).
    static void reset();

    // Progress as "step / total".
    static QString progressText();

signals:
    void stepChanged(int index);
    void tutorialFinished();

private:
    static bool s_active;
    static int s_currentIndex;
};

#endif // TUTORIAL_H
