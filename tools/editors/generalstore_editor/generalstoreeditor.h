#ifndef GENERALSTOREEDITOR_H
#define GENERALSTOREEDITOR_H

#include "../../editors/common_csv_editor.h"

class GeneralStoreEditor : public CommonCsvEditor
{
    Q_OBJECT
public:
    explicit GeneralStoreEditor(QWidget* parent = nullptr);
protected:
    QString detailHtml(int row) const override;
};

#endif
