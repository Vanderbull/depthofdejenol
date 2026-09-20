How to Create Patch Files for the Depth of Dejenol Updater
===========================================================

The updater in src/update/ works in two modes: a simple manifest server
and GitHub Releases. Both expect a ZIP archive of files to overwrite plus
a SHA256 checksum. Here is how to produce them.

1. Decide what you are patching
-------------------------------
A patch is a ZIP containing only the files that changed, with paths
relative to the game's installation directory. For example, if you fixed
a typo in theCity.qss and corrected a monster stat in gamedata.json, your
ZIP would contain:

    theCity.qss
    gamedata.json

Do NOT include the blacklands binary itself unless you are replacing it.
Do NOT include directories outside the app dir. Paths inside the ZIP must
match paths the game actually reads.

2. Create the ZIP
-----------------
From inside the game's data or root directory (whatever contains the files
the game reads at runtime), run:

    cd /path/to/game/root
    zip -r /tmp/depthofdejenol-627.zip theCity.qss gamedata.json

Or use the project's helper script:

    ./tools/patches/create_test_patch.sh

The ZIP name does not matter to the code, but naming it by version helps
humans. The script uses python3 as a fallback if `zip` is not installed and
prints the SHA256 of the result.

3. Compute the SHA256 checksum
------------------------------
    sha256sum /tmp/depthofdejenol-627.zip

Copy the hex string. This goes into the manifest as lowercase hex. If you
use the helper script, it prints the SHA256 for you automatically.

4. Write the manifest JSON
---------------------------
Simple manifest server (the non-GitHub path):

    {
      "version": "v627",
      "notes": "Fixed theCity style and monster stats.",
      "patchUrl": "https://your-host/patches/depthofdejenol-627.zip",
      "sha256": "abcd1234..."
    }

The version field must match what your game reports as its current version
(look at version.h FULL_VERSION). The updater compares local vs remote and
only offers the download when remote is newer.

GitHub Releases path (alternate):
- Upload the ZIP as a release asset.
- The updater reads tag_name as the version and the asset download URL.
- It also looks for a second asset whose name contains "sha256" to get the
  checksum. If no such asset exists, checksum verification is skipped.

5. Host the manifest and the ZIP
---------------------------------
For testing, you can host them anywhere reachable by the game. Options:

- A static web server you control (nginx, python -m http.server, etc.)
- GitHub Releases if you prefer the GitHub path
- An embedded HTTP server in the test harness (see testing section)

For pure local (no network) testing, point the manifest URL at a file URL:

    file:///home/rickard/patches/manifest.json

and ensure the ZIP path inside the manifest is also a file URL or absolute
path that the updater process can reach.

6. Test locally without a real server
-------------------------------------
The project includes a standalone test harness in test/ that spins up a
temporary HTTP server, serves a fake manifest and patch zip, drives the
real UpdateManager through check + download + verify, and reports pass/fail.

Build and run it:

    cd test
    /usr/bin/qmake6 test_updater2.pro -o Makefile
    make -j$(nproc)
    ./test_updater2

The harness exercises these scenarios automatically:

    - Simple manifest, remote version newer -> update offered + download + verify
    - Simple manifest, remote version older -> no update
    - Malformed manifest (missing version field) -> no crash, update skipped
    - Bad SHA256 (mismatch) -> download finishes with error
    - GitHub release manifest path -> update offered + download
    - Equal version -> no update

Each test also verifies the downloaded ZIP contents using python3 zipfile,
so the harness confirms that the patch data you would ship is structurally
sound before you upload it.

For manual local testing without the harness:

    cd /some/test/dir
    python3 -m http.server 8080

Point the UpdateDialog at http://localhost:8080/manifest.json and let it
fetch the zip from the same server. For the GitHub path, use a fake GitHub
manifest that points back to localhost — the updater doesn't care where the
URLs come from as long as they are reachable.

7. Anti-tamper and integrity
----------------------------
Currently the updater provides:

- SHA256 verification of the downloaded ZIP against the manifest field.
  If the hash does not match, downloadFinished emits an error and the patch
  is not applied.
- Version comparison so a patch built for v628 is not offered to v627 or
  v629 unless the version numbers say so.

Missing for production hardening:

- The manifest itself is not signed. Anyone who can MITM the HTTP path can
  replace both the zip and the manifest. For real deployments, sign the
  manifest with a private key and ship the public key inside the game binary
  (or in a pinned file checked against a known fingerprint).
- The ZIP overwrites files in place. If extraction is interrupted, you may
  end up with half-written files. A safer pattern is to extract into a temp
  directory, verify all files extracted cleanly, then swap them atomically.
- There is no backup of files before overwrite, so a bad patch cannot be
  rolled back automatically.

8. Common mistakes
------------------
- Version string mismatch: manifest says "v627" but game reports "v626" or
  "627" without the v. The update is skipped. Keep the format consistent.
- Wrong file paths inside the ZIP: paths must be relative to the app dir,
  not absolute, and must use forward slashes on all platforms.
- Forgetting to update the SHA256 after rebuilding the ZIP.
- Uploading the ZIP to GitHub but not including a sha256-named asset, then
  being surprised that checksum verification is skipped (it still downloads,
  just without the extra verify step in that code path).
- Including files in the ZIP that the game does not actually read. The updater
  does not validate that the files matter; it just overwrites whatever is in
  the archive.

9. Patch format summary
-----------------------
Patch ZIP: any name, contains changed files with app-relative paths.
Manifest: JSON with version (matches game version format), notes, patchUrl,
          sha256 (lowercase hex of the ZIP).
Hosting: any HTTP server or file:// for local testing.
Verification: SHA256 of the downloaded ZIP compared against manifest.
Application: ZIP extracted into the application directory via system unzip
             (Linux/macOS) or PowerShell Expand-Archive (Windows). The user
             must restart the game to pick up the changes.

10. Test harness layout
-----------------------
test/test_updater.pro      - builds the first harness (basic check/download)
test/test_updater2.pro     - builds the extended harness with 6 scenarios
test/test_updater.cpp      - source for the first harness
test/test_updater2.cpp     - source for the extended harness
tools/patches/create_test_patch.sh - helper to build a sample zip + sha256
