#ifndef GLYPHEDITOR_H
#define GLYPHEDITOR_H

#include <QDialog>

#include "WebSocketPair.h"
#include "GlyphItem.h"
#include "ComboBoxDelegate.h"

#include "ColorMapEditor.h"

namespace Ui
{
class GlyphEditor;
}

class GlyphEditor : public QDialog
{
    Q_OBJECT

public:
    explicit GlyphEditor( WebSocketPair* websockets, QWidget *parent = nullptr );
    ~GlyphEditor();

private:
    // メンバ変数群
    Ui::GlyphEditor *ui;

    // 通信関連
    WebSocketPair* m_web_sockets = nullptr;

    int m_number_of_vector = -1;

    QStandardItemModel *m_model = nullptr;
    QStandardItemModel* m_size_model = nullptr;
    QStandardItemModel* m_color_data_model = nullptr;
    QStringList m_vector_list;

    // ウィジェット群
    ColorMapEditor m_color_map_editor;

    //  メソッド群
    void initialize();

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

public slots:
    void updateNumberOfVector( const int numberOfVector );
    void updateOperatorState( bool operatorState ); // true:権限あり
    void reset();
    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );
};

#endif // GLYPHEDITOR_H
