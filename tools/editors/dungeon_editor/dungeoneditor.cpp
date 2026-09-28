#include "common_csv_editor.h"
#include <QApplication>

class DungeonEditor : public CommonCsvEditor
{
    Q_OBJECT
public:
    explicit DungeonEditor(QWidget* parent = nullptr)
        : CommonCsvEditor("data/MDATA11.csv", "Dejenol Dungeon Editor", parent)
    {
    }
};

#include "dungeoneditor.moc"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    DungeonEditor w;
    w.show();
    return app.exec();
}
