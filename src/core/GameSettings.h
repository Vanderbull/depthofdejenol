#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include <QFont>
#include <QString>

class QSettings;

// Single source of truth for the player-facing options.
//
// One file (game_settings.ini), one section ([Game]), typed accessors with
// defaults. Before v6.6.0 the Options dialog wrote [Game] No* keys that nothing
// ever read, while the same ini also carried [Graphics]/[Audio]/[Gameplay] from
// an older scheme. migrateLegacyKeys() folds those in once and drops them, so
// afterwards there is exactly one schema.
//
// applyAll() pushes every value into the running game; it is called at startup
// so a choice survives a restart. That is the whole point of this class.
class GameSettings {
public:
    static GameSettings& instance();

    // --- Audio (0.0 .. 1.0) ---
    float musicVolume() const;
    void  setMusicVolume(float v);
    float sfxVolume() const;
    void  setSfxVolume(float v);
    bool  audioMuted() const;
    void  setAudioMuted(bool muted);

    // --- Gameplay ---
    bool autosaveEnabled() const;
    void setAutosaveEnabled(bool on);
    bool revealAutomap() const;
    void setRevealAutomap(bool on);
    int  combatSpeedMs() const;                 // combat FX timer interval
    void setCombatSpeedMs(int ms);

    // --- Display ---
    bool fullscreen() const;
    void setFullscreen(bool on);
    QString proportionalFontFamily() const;     // "" = keep the application default
    int     proportionalFontSize() const;
    void    setProportionalFont(const QString& family, int size);
    void    setProportionalFont(const QFont& font);
    QString fixedFontFamily() const;            // "" = keep the default monospace
    int     fixedFontSize() const;
    void    setFixedFont(const QString& family, int size);
    void    setFixedFont(const QFont& font);

    // --- Lifecycle ---
    void applyAll();          // push every value into the running game
    void resetToDefaults();   // restore every default, then applyAll()
    void sync();              // flush to disk

    // Defaults, exposed so the dialog and the tests share one source.
    static float defaultMusicVolume();
    static float defaultSfxVolume();
    static bool  defaultAutosaveEnabled();
    static bool  defaultRevealAutomap();
    static int   defaultCombatSpeedMs();
    static bool  defaultFullscreen();
    // The font the game itself uses (gameSettings owns it now that the Options
    // dialog can change it). Used whenever no font has been stored, so Default
    // puts the game's own font back rather than Qt's platform default.
    static QFont defaultProportionalFont();

    // Fold the pre-v6.6.0 [Graphics]/[Audio]/[Gameplay] sections into [Game]
    // and drop them. Public + static so a test can run it on a throwaway file.
    static void migrateLegacyKeys(QSettings* settings);

private:
    GameSettings();
    ~GameSettings();
    GameSettings(const GameSettings&) = delete;
    GameSettings& operator=(const GameSettings&) = delete;

    QSettings* m_settings = nullptr;
};

#endif // GAMESETTINGS_H
