#ifndef version_H
#define version_H

namespace GameConstants {
    // FULL_VERSION is parsed by UpdateManager::parseVersionNumber(), which
    // expects a plain integer with an optional v prefix (e.g. v642). Do NOT
    // concatenate the git hash here — v6421ed6b1a fails to parse and silently
    // disables the update check. The hash is separate metadata.
    static const char* const GIT_HASH = "1ed6b1a";
    static const char* const BUILD_TIMESTAMP = "2026-10-10_06:58:05";
    static const char* const FULL_VERSION = "v642";
}

#endif
