#include "common_csv_editor.h"
#include <QApplication>

class MonsterEditor : public CommonCsvEditor
{
    Q_OBJECT
public:
    explicit MonsterEditor(QWidget* parent = nullptr)
        : CommonCsvEditor("data/MDATA5.csv", "Dejenol Monster Editor", parent)
    {
    }
};

#include "monstereditor.moc"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MonsterEditor w;
    w.show();
    return app.exec();
}
