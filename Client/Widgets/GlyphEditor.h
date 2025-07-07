#ifndef GLYPHEDITOR_H
#define GLYPHEDITOR_H

#include <QDialog>
#include <QStandardItemModel>

#include "ComboBoxDelegate.h"
#include "ColorMapEditor.h"
#include "GlyphItem.h"

namespace Ui {
class GlyphEditor;
}

class GlyphEditor : public QDialog
{
    Q_OBJECT

public:
    explicit GlyphEditor( QWidget *parent = nullptr );
    ~GlyphEditor();

private:
    Ui::GlyphEditor *ui;
    QStandardItemModel *m_model = nullptr;
    QStandardItemModel* m_size_model = nullptr;
    QStandardItemModel* m_color_data_model = nullptr;
    QStringList m_vector_list;

    void initialize();

signals:
    void updateGlyphParameterClientMessage( QStandardItemModel* model );
    void requestReplaceServerGlyphObject();

public slots:
    void updateNumberOfVector( const int numberOfVector );

private slots:
    void onDirectionComboBoxIndexChanged( int index );

    void onSizeConstantRadioButtonClicked();
    void onSizeVariablesRadioButtonClicked();
    void onSizeNumberOfVariableValueChanged();

    void onEditColorMap();

    void onColorDataConstantRadioButtonClicked();
    void onColorDataVariablesRadioButtonClicked();
    void onColorDataNumberOfVariableValueChanged();

    void onApply();
};

#endif // GLYPHEDITOR_H
