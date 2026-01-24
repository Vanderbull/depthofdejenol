Manifest JSON schema (host this on your update server)

    URL: https://example.com/depthofdejenol/manifest.json
    Example content:

JSON

{
  "version": "1.0.2",
  "notes": "Bugfixes: fixed save-game crash; new map generation improvements.",
  "patchUrl": "https://example.com/depthofdejenol/patches/depthofdejenol-1.0.2.zip",
  "sha256": "f0a1b2c3... (lowercase hex string)"
}

How the update works (workflow)

    App (AboutDialog) opens UpdateDialog; user clicks "Check".
    UpdateManager downloads manifest JSON and compares the remote version to local version (it attempts to read version.txt from the application dir; you can instead read GameStateManager::getGameValue("GameVersion")).
    If newer, dialog displays notes and enables Download.
    User clicks "Download & Apply": UpdateManager downloads the patch file and verifies sha256.
    If OK, UpdateDialog attempts to extract the ZIP into the app directory using platform facilities:
        Linux/macOS: spawns system 'unzip -o' into application dir
        Windows: spawns PowerShell Expand-Archive
    User is prompted to restart the application (or you can automate this later).

Security & robustness notes

    Always sign/verify updates. I used SHA256 verification; for production consider code signing or HTTPS + signature verification.
    Overwriting application files while the app is running can be risky. Better approach: download an updater helper executable that runs detached, extracts patch, and restarts the app. The example uses unzip/Expand-Archive as a minimal approach.
    Consider atomic update: download to temp + extraction to temp + move into place or use an updater program.
    Consider adding rollback: keep previous files or create a backup before applying.

Build integration

    Add new files to your project build (your .pro or CMakeLists.txt). Example for qmake (.pro):
        SOURCES += src/update/UpdateManager.cpp
        src/update/UpdateDialog.cpp
        HEADERS += src/update/UpdateManager.h
        src/update/UpdateDialog.h
    Update include paths if necessary.

Testing locally

    Host a local manifest/zip or use file:// URLs for quick testing.
    Create a simple version.txt in the app folder with a version string to simulate current version.
    Use a small zip containing one changed file for testing.

Next steps I can take for you

    Create a safer/atomic updater that uses a small separate updater executable (recommended for production).
    Hook the UpdateManager to GameStateManager::getGameValue("GameVersion") instead of version.txt so it reads the same internal version info.
    Add UI polish (progress dialog, cancellation, release notes formatting).
    Prepare a sample manifest and a sample patch zip and add CI steps to publish them to a GitHub release or static hosting.

If you want, I can:

    Open a GitHub issue with this feature plan and attach the files as a proposed patch (I can draft the issue for review).
    Create a PR/branch and push these files (I will need repo owner confirmation or push rights).
