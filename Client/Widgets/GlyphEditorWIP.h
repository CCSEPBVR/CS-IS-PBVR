#ifndef GLYPHEDITORWIP_H
#define GLYPHEDITORWIP_H

#include <QDialog>
#include <QSpinBox>
#include <QGridLayout>

#include "WebSocketPair.h"
#include "../../Shared/GlyphParameter.h"

#include "ColorMapEditor.h"

namespace Ui {
class GlyphEditorWIP;
}

class GlyphEditorWIP : public QDialog
{
    Q_OBJECT

public:
    explicit GlyphEditorWIP( WebSocketPair* websockets, QWidget *parent = nullptr );
    ~GlyphEditorWIP();

public slots:
    void updateOperatorState( bool operatorState ); // true:権限あり
    void reset();
    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );

    void updateNumberOfVector( const int numberOfVector );
    void receiveGlyphParameter( const QJsonObject& dataArray );

private:
    Ui::GlyphEditorWIP *ui;

    WebSocketPair* m_web_sockets = nullptr;
    bool m_is_operator;

    QStringList m_vector_list;



    ColorMapEditor m_color_map_editor;

    void initialize();
    void clearLayout( QLayout* layout );
    void updateUiEnableState( const bool isVariableArray, QSpinBox* spinBox, QGridLayout* gridLayout );
    void updateVariableGridLayout( QSpinBox* spinBox, QGridLayout* grid );

    // FIXME:toHogeHoge作るぐらいなら最初からColorMap,OpacityMapクラス側を変えた方がいい気がします
    // FIXME:このインラインは本来不要なのでなくても動作するようにしてください
    inline QVector<QColor> toQVectorColors( const std::vector<kvs::RGBColor>& colors )
    {
        QVector<QColor> qcolors;
        qcolors.reserve( static_cast<int>( colors.size() ) );

        for( const auto& c : colors )
        {
            qcolors.append( QColor( c.r(), c.g(), c.b() ) );
        }

        return qcolors;
    }

    inline QVector<float> toQVectorOpacities( const std::vector<float>& values )
    {
        QVector<float> qvalues;
        qvalues.reserve( static_cast<int>( values.size() ) );

        for( const auto& v : values )
        {
            qvalues.append( v );
        }

        return qvalues;
    }

    inline std::vector<kvs::RGBColor> toStdVectorColors( const QVector<QColor>& colors )
    {
        std::vector<kvs::RGBColor> vec;
        vec.reserve( colors.size() );

        for( const auto& c : colors )
        {
            vec.emplace_back( c.red(), c.green(), c.blue() );
        }

        return vec;
    }

    inline std::vector<float> toStdVectorOpacities( const QVector<float>& values )
    {
        std::vector<float> vec;
        vec.reserve( values.size() );

        for( const auto& v : values )
        {
            vec.push_back( v );
        }

        return vec;
    }

    inline QVector<QColor> defaultColor()
    {
        QVector<QColor> defaultColor { QColor(5, 48, 97), QColor(6, 50, 100), QColor(7, 52, 102), QColor(8, 54, 105), QColor(9, 56, 108), QColor(10, 58, 111), QColor(11, 60, 114), QColor(12, 62, 116), QColor(14, 64, 119), QColor(15, 66, 122), QColor(16, 68, 125), QColor(17, 70, 128), QColor(18, 72, 131), QColor(19, 74, 134), QColor(20, 76, 136), QColor(21, 78, 139), QColor(22, 80, 142), QColor(23, 83, 145), QColor(24, 85, 148), QColor(25, 87, 151), QColor(27, 89, 154), QColor(28, 91, 157), QColor(29, 93, 160), QColor(30, 95, 163), QColor(31, 98, 166), QColor(32, 100, 169), QColor(33, 102, 172), QColor(35, 104, 173), QColor(37, 105, 174), QColor(38, 107, 175), QColor(40, 109, 176), QColor(41, 111, 177), QColor(43, 113, 178), QColor(45, 114, 178), QColor(46, 116, 179), QColor(47, 118, 180), QColor(49, 120, 181), QColor(50, 121, 182), QColor(51, 123, 183), QColor(53, 125, 184), QColor(54, 127, 185), QColor(55, 129, 186), QColor(57, 130, 187), QColor(58, 132, 188), QColor(59, 134, 189), QColor(60, 136, 189), QColor(61, 138, 190), QColor(63, 140, 191), QColor(64, 141, 192), QColor(65, 143, 193), QColor(66, 145, 194), QColor(67, 147, 195), QColor(71, 149, 196), QColor(74, 151, 197), QColor(78, 153, 198), QColor(81, 154, 199), QColor(85, 156, 200), QColor(88, 158, 201), QColor(91, 160, 202), QColor(95, 162, 203), QColor(98, 164, 204), QColor(101, 166, 205), QColor(104, 168, 206), QColor(107, 170, 207), QColor(110, 172, 209), QColor(113, 174, 210), QColor(116, 175, 211), QColor(118, 177, 212), QColor(121, 179, 213), QColor(124, 181, 214), QColor(127, 183, 215), QColor(130, 185, 216), QColor(132, 187, 217), QColor(135, 189, 218), QColor(138, 191, 219), QColor(141, 193, 220), QColor(143, 195, 221), QColor(146, 197, 222), QColor(149, 198, 223), QColor(151, 200, 223), QColor(154, 201, 224), QColor(157, 202, 225), QColor(159, 203, 226), QColor(162, 205, 226), QColor(164, 206, 227), QColor(167, 207, 228), QColor(169, 208, 228), QColor(172, 210, 229), QColor(174, 211, 230), QColor(177, 212, 231), QColor(179, 214, 231), QColor(182, 215, 232), QColor(184, 216, 233), QColor(187, 217, 234), QColor(189, 219, 234), QColor(192, 220, 235), QColor(194, 221, 236), QColor(197, 223, 236), QColor(199, 224, 237), QColor(202, 225, 238), QColor(204, 226, 239), QColor(207, 228, 239), QColor(209, 229, 240), QColor(210, 230, 240), QColor(212, 230, 241), QColor(213, 231, 241), QColor(215, 232, 241), QColor(216, 232, 241), QColor(218, 233, 242), QColor(219, 234, 242), QColor(221, 235, 242), QColor(222, 235, 242), QColor(224, 236, 243), QColor(225, 237, 243), QColor(227, 237, 243), QColor(228, 238, 244), QColor(230, 239, 244), QColor(231, 239, 244), QColor(233, 240, 244), QColor(234, 241, 245), QColor(235, 241, 245), QColor(237, 242, 245), QColor(238, 243, 245), QColor(240, 244, 246), QColor(241, 244, 246), QColor(243, 245, 246), QColor(244, 246, 246), QColor(246, 246, 247), QColor(247, 247, 247), QColor(247, 246, 245), QColor(248, 245, 243), QColor(248, 244, 241), QColor(248, 243, 240), QColor(249, 242, 238), QColor(249, 241, 236), QColor(249, 239, 234), QColor(250, 238, 232), QColor(250, 237, 230), QColor(250, 236, 228), QColor(250, 235, 227), QColor(251, 234, 225), QColor(251, 233, 223), QColor(251, 232, 221), QColor(251, 231, 219), QColor(251, 230, 217), QColor(252, 229, 215), QColor(252, 228, 214), QColor(252, 227, 212), QColor(252, 225, 210), QColor(252, 224, 208), QColor(252, 223, 206), QColor(253, 222, 204), QColor(253, 221, 203), QColor(253, 220, 201), QColor(253, 219, 199), QColor(253, 217, 196), QColor(253, 215, 193), QColor(252, 212, 191), QColor(252, 210, 188), QColor(252, 208, 185), QColor(252, 206, 182), QColor(252, 204, 179), QColor(251, 202, 177), QColor(251, 200, 174), QColor(251, 197, 171), QColor(250, 195, 168), QColor(250, 193, 165), QColor(250, 191, 163), QColor(249, 189, 160), QColor(249, 187, 157), QColor(248, 184, 154), QColor(248, 182, 152), QColor(248, 180, 149), QColor(247, 178, 146), QColor(247, 176, 143), QColor(246, 174, 141), QColor(246, 171, 138), QColor(245, 169, 135), QColor(245, 167, 133), QColor(244, 165, 130), QColor(243, 162, 128), QColor(242, 160, 126), QColor(241, 157, 124), QColor(240, 155, 122), QColor(239, 152, 119), QColor(238, 149, 117), QColor(237, 147, 115), QColor(235, 144, 113), QColor(234, 142, 111), QColor(233, 139, 109), QColor(232, 136, 107), QColor(231, 134, 105), QColor(230, 131, 103), QColor(229, 128, 101), QColor(227, 126, 99), QColor(226, 123, 97), QColor(225, 120, 95), QColor(224, 118, 93), QColor(223, 115, 91), QColor(221, 112, 89), QColor(220, 110, 87), QColor(219, 107, 85), QColor(218, 104, 83), QColor(217, 102, 81), QColor(215, 99, 79), QColor(214, 96, 77), QColor(213, 94, 76), QColor(211, 91, 74), QColor(210, 89, 73), QColor(208, 86, 71), QColor(207, 84, 70), QColor(206, 82, 68), QColor(204, 79, 67), QColor(203, 77, 66), QColor(201, 74, 64), QColor(200, 72, 63), QColor(198, 69, 62), QColor(197, 66, 60), QColor(196, 64, 59), QColor(194, 61, 57), QColor(193, 58, 56), QColor(191, 55, 55), QColor(190, 53, 53), QColor(188, 50, 52), QColor(187, 46, 51), QColor(185, 43, 49), QColor(184, 40, 48), QColor(182, 36, 47), QColor(181, 33, 46), QColor(179, 29, 44), QColor(178, 24, 43), QColor(175, 23, 43), QColor(172, 22, 42), QColor(169, 21, 42), QColor(166, 20, 41), QColor(162, 19, 41), QColor(159, 18, 40), QColor(156, 17, 40), QColor(153, 15, 39), QColor(150, 14, 39), QColor(147, 13, 38), QColor(144, 12, 38), QColor(141, 11, 37), QColor(138, 10, 37), QColor(135, 9, 36), QColor(132, 8, 36), QColor(129, 7, 35), QColor(126, 6, 35), QColor(123, 5, 34), QColor(120, 4, 34), QColor(117, 3, 33), QColor(115, 2, 33), QColor(112, 2, 33), QColor(109, 1, 32), QColor(106, 1, 32), QColor(103, 0, 31) };
        return defaultColor;
    }

private slots:
    // Direction
    void onDirectionComboBoxIndexChanged();

    // Size
    void onSizeUiState();
    void onSizeNumberOfVariableValueChanged();

    // ColorMap
    void onColorMapEditorClicked();

    // Color Data
    void onColorDataUiState();
    void onColorDataNumberOfVariableValueChanged();

    void onApply();
};

#endif // GLYPHEDITORWIP_H
