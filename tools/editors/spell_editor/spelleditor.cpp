#include "common_csv_editor.h"
#include <QApplication>

class SpellEditor : public CommonCsvEditor
{
    Q_OBJECT
public:
    explicit SpellEditor(QWidget* parent = nullptr)
        : CommonCsvEditor("data/MDATA2.csv", "Dejenol Spell Editor", parent)
    {
    }
};

#include "spelleditor.moc"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    SpellEditor w;
    w.show();
    return app.exec();
}
