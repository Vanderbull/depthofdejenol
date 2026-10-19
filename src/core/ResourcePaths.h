#ifndef RESOURCEPATHS_H
#define RESOURCEPATHS_H

#include <QString>

/**
 * Runtime lookup for asset files (stylesheets, sounds, images).
 *
 * Historically the game located .qss files with
 * QFileInfo(__FILE__).absolutePath(). __FILE__ expands to the *build-time*
 * source path, which only exists on the machine that compiled the binary, so
 * an installed or moved build silently lost every stylesheet. locate() searches
 * a list of plausible locations instead and returns the first hit.
 */
namespace ResourcePaths {

/**
 * Find @p fileName on disk.
 *
 * Searched in order:
 *   1. <application dir>
 *   2. <application dir>/..
 *   3. <application dir>/../..
 *   4. <application dir>/../../src
 *   5. the current working directory
 *   6. the current working directory + "/src"
 *   7. the compile-time source directory (dev builds, BLACKLANDS_SOURCE_DIR)
 *
 * Returns an absolute path, or an empty string when the file is not found
 * anywhere.
 */
QString locate(const QString &fileName);

} // namespace ResourcePaths

#endif // RESOURCEPATHS_H
