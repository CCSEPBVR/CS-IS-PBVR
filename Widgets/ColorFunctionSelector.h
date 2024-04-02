#ifndef COLORFUNCTIONSELECTOR_H
#define COLORFUNCTIONSELECTOR_H

#include <QWidget>
#include "ExtendedKVS/Screen.h"
#include "ExtendedTransferFunctionMessage.h"
#include <kvs/ColorMapBar>

namespace Ui {
class ColorFunctionSelector;
}

class ColorFunctionSelector : public QWidget
{
    Q_OBJECT

public:
    explicit ColorFunctionSelector(QWidget *parent = nullptr);
    ~ColorFunctionSelector();
    void setExtendedTransferFunctionMessage( ExtendedTransferFunctionMessage* extended_transfer_function_message ){ m_extended_transfer_function_message = extended_transfer_function_message; }
    void setScreen( kvs::qt::jaea::Screen* screen ){ m_screen = screen; };
    void setColorMapBar( kvs::ColorMapBar* colorMapBar ) { m_color_map_bar = colorMapBar;        }
    ExtendedTransferFunctionMessage* getExtendedTransferFunctionMessage() { return m_extended_transfer_function_message; }
    void populateColorFunctionLists(int n);
    void onColorFunctionChanged( int index );

private:
    Ui::ColorFunctionSelector *ui;
    kvs::qt::jaea::Screen* m_screen;
    kvs::ColorMapBar* m_color_map_bar;
    ExtendedTransferFunctionMessage* m_extended_transfer_function_message;
};

#endif // COLORFUNCTIONSELECTOR_H
