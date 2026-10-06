#include "Tutorial.h"

bool Tutorial::s_active = false;
int Tutorial::s_currentIndex = -1;

Tutorial::Tutorial(QObject *parent) : QObject(parent) {}

Tutorial::~Tutorial() = default;

QList<TutorialStep> Tutorial::steps() {
    QList<TutorialStep> steps;

    auto make = [](int index, const QString& id, const QString& title,
                   const QString& instruction, const QString& completesOn, int target = 0) {
        TutorialStep s;
        s.index = index;
        s.id = id;
        s.title = title;
        s.instruction = instruction;
        s.completesOn = completesOn;
        s.targetValue = target;
        return s;
    };

    steps.append(make(0, "welcome", "Welcome to the Dungeon",
                      "Use W/A/S/D or arrow keys to move. Explore the first floor.",
                      "move"));

    steps.append(make(1, "find_stairs", "Find the Stairs",
                      "Search the floor until you find stairs leading down. Press T to use them.",
                      "stairs"));

    steps.append(make(2, "descend", "Descend Deeper",
                      "You are now on floor 2. Monsters are stronger here. Be careful.",
                      "level", 2));

    steps.append(make(3, "first_fight", "Your First Fight",
                      "Press F to fight a monster. Use your weapons and spells wisely.",
                      "fight"));

    steps.append(make(4, "cast_spell", "Cast a Spell",
                      "Press S to cast a spell. Spells cost mana but can turn the tide.",
                      "spell"));

    steps.append(make(5, "rest", "Rest and Recover",
                      "Press R to rest. Restoring HP and mana is essential for survival.",
                      "rest"));

    steps.append(make(6, "pickup", "Pick Up Loot",
                      "Defeated monsters drop items. Press P to pick them up.",
                      "pickup"));

    steps.append(make(7, "open_door", "Open a Door",
                      "Press O to open doors and chests. Some are locked — you may need a key.",
                      "door"));

    steps.append(make(8, "reach_floor_5", "Reach Floor 5",
                      "Descend to floor 5. A guardian awaits there. Prepare well.",
                      "level", 5));

    steps.append(make(9, "defeat_guardian", "Defeat the Guardian",
                      "The Grotto Warden guards floor 5. Defeat it to complete the tutorial.",
                      "boss", 5));

    steps.append(make(10, "victory", "Tutorial Complete",
                      "You have learned the basics. The dungeon awaits — good luck!",
                      "done"));

    return steps;
}

TutorialStep Tutorial::step(int index) {
    QList<TutorialStep> all = steps();
    if (index < 0 || index >= all.size()) {
        TutorialStep invalid;
        invalid.index = -1;
        return invalid;
    }
    return all[index];
}

bool Tutorial::isActive() {
    return s_active;
}

void Tutorial::start() {
    s_active = true;
    s_currentIndex = 0;
}

void Tutorial::stop() {
    s_active = false;
    s_currentIndex = -1;
}

int Tutorial::currentStepIndex() {
    return s_currentIndex;
}

TutorialStep Tutorial::currentStep() {
    return step(s_currentIndex);
}

bool Tutorial::advance() {
    if (!s_active) return false;
    s_currentIndex++;
    if (s_currentIndex >= steps().size()) {
        stop();
        return false;
    }
    return true;
}

bool Tutorial::reportEvent(const QString& event, int value) {
    if (!s_active) return false;
    TutorialStep current = currentStep();
    if (current.index < 0) return false;

    bool completed = false;
    if (current.completesOn == "move" && event == "move") {
        completed = true;
    } else if (current.completesOn == "stairs" && event == "stairs") {
        completed = true;
    } else if (current.completesOn == "level" && event == "level" && value >= current.targetValue) {
        completed = true;
    } else if (current.completesOn == "fight" && event == "fight") {
        completed = true;
    } else if (current.completesOn == "spell" && event == "spell") {
        completed = true;
    } else if (current.completesOn == "rest" && event == "rest") {
        completed = true;
    } else if (current.completesOn == "pickup" && event == "pickup") {
        completed = true;
    } else if (current.completesOn == "door" && event == "door") {
        completed = true;
    } else if (current.completesOn == "boss" && event == "boss" && value >= current.targetValue) {
        completed = true;
    } else if (current.completesOn == "done") {
        // The final step is a completion message; any event finishes it.
        completed = true;
    }

    if (completed) {
        advance();
        return true;
    }
    return false;
}

void Tutorial::reset() {
    s_active = false;
    s_currentIndex = -1;
}

QString Tutorial::progressText() {
    if (!s_active) return "Tutorial not active";
    return QString("Step %1 / %2").arg(s_currentIndex + 1).arg(steps().size());
}
