#include "common_csv_editor.h"
#include <QApplication>

class CharacterEditor : public CommonCsvEditor
{
    Q_OBJECT
public:
    explicit CharacterEditor(QWidget* parent = nullptr)
        : CommonCsvEditor("data/MDATA4.csv", "Dejenol Character Editor", parent)
    {
    }
};

#include "charmereditor.moc"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    CharacterEditor editor;
    editor.show();
    return app.exec();
}
