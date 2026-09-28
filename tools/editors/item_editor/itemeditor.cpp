#include "../common_csv_editor.h"
#include <QApplication>

class ItemEditor : public CommonCsvEditor
{
public:
    ItemEditor(QWidget* parent = nullptr)
        : CommonCsvEditor("../../itemconverter/data/MDATA3.csv",
                           "Dejenol Item Editor",
                           parent)
    {
    }
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    ItemEditor editor;
    editor.show();
    return app.exec();
}
