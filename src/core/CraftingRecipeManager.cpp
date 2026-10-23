#include "CraftingRecipeManager.h"

CraftingRecipeManager& CraftingRecipeManager::instance()
{
    static CraftingRecipeManager inst;
    return inst;
}

bool CraftingRecipeManager::load()
{
    // Try to locate the crafting recipes JSON file
    QString filePath = QCoreApplication::applicationDirPath() + "/resources/crafting_recipes.json";
    QFile file(filePath);
    if (!file.exists()) {
        // Fallback: same directory as executable
        filePath = QCoreApplication::applicationDirPath() + "/crafting_recipes.json";
        file.setFileName(filePath);
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open crafting recipes file:" << filePath;
        return false;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse crafting recipes JSON:" << parseError.errorString();
        return false;
    }
    if (!doc.isArray()) {
        qWarning() << "Crafting recipes JSON is not an array";
        return false;
    }
    QJsonArray arr = doc.array();
    m_recipes.clear();
    for (const QJsonValue &v : arr) {
        if (v.isObject()) {
            QJsonObject o = v.toObject();
            CraftingRecipe r;
            r.name = o.value("name").toString();
            r.description = o.value("description").toString();
            QJsonArray inputsArr = o.value("inputs").toArray();
            for (const QJsonValue &input : inputsArr) {
                if (input.isString())
                    r.inputs.append(input.toString());
            }
            m_recipes.append(r);
        }
    }

    qInfo() << "Loaded crafting recipes: " << m_recipes.size() << " recipes";
    return true;
}

QString CraftingRecipeManager::getResult(const QStringList& inputs) const
{
    // We expect exactly two inputs for now (to match existing UI), but we can handle any number.
    // For simplicity, we'll look for a recipe whose inputs match exactly (as a multiset) the given inputs.
    // Since the UI passes two items, we'll just compare sorted lists.
    QStringList sortedInputs = inputs;
    std::sort(sortedInputs.begin(), sortedInputs.end());

    for (const CraftingRecipe &recipe : m_recipes) {
        QStringList sortedRecipeInputs = recipe.inputs;
        std::sort(sortedRecipeInputs.begin(), sortedRecipeInputs.end());
        if (sortedInputs == sortedRecipeInputs) {
            return recipe.name;
        }
    }
    return QString();
}
