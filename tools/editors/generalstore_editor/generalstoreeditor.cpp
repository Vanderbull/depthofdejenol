#include "generalstoreeditor.h"
#include <QApplication>

GeneralStoreEditor::GeneralStoreEditor(QWidget* parent)
    : CommonCsvEditor(
          "tools/generalstoreconverter/data/MDATA_Store.csv",
          "Dejenol General Store Editor",
          parent)
{
}

QString GeneralStoreEditor::detailHtml(int row) const
{
    QString itemName = table->item(row, 1) ? table->item(row, 1)->text() : "";
    QString rarity   = table->item(row, 5) ? table->item(row, 5)->text() : "";

    QString rarityColor;
    if (rarity == "Common")      rarityColor = "#9cdcfe";
    else if (rarity == "Uncommon") rarityColor = "#c586c0";
    else if (rarity == "Rare")     rarityColor = "#569cd6";
    else if (rarity == "Epic")     rarityColor = "#d16969";
    else if (rarity == "Legendary") rarityColor = "#ffcc00";
    else                            rarityColor = "#d4d4d4";

    QString html = "<html><body style='font-family: Consolas, monospace; font-size: 13px;'>";
    html += QString("<h2 style='color: %1; margin: 0 0 8px 0;'>%2</h2>")
                 .arg(rarityColor).arg(itemName);
    html += "<table width='100%' cellpadding='3' cellspacing='0' border='1' "
            "style='border-collapse: collapse; border-color: #3a3a3c;'>";

    struct { int col; const char* label; } cols[] = {
        {0, "Item ID"}, {2, "Category"}, {3, "Price (GP)"},
        {4, "Weight (lbs)"}, {5, "Rarity"}
    };
    for (auto& c : cols) {
        QString bg = (c.col % 2 == 0) ? "#2d2d2d" : "#252526";
        QString val = table->item(row, c.col) ? table->item(row, c.col)->text() : "";
        html += QString("<tr bgcolor='%1'><td style='color:#9cdcfe;'><b>%2</b></td>"
                        "<td style='color:#d4d4d4;'>%3</td></tr>")
                    .arg(bg).arg(c.label).arg(val);
    }
    html += "</table></body></html>";
    return html;
}



int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    GeneralStoreEditor editor;
    editor.show();
    return app.exec();
}
