#ifndef PBVR__KVS__VISCLIENT__LABEL_H_INCLUDE
#define PBVR__KVS__VISCLIENT__LABEL_H_INCLUDE

#include <kvs/glut/Label>
#include <kvs/glut/Screen>
#include <kvs/glut/LegendBar>
#include <kvs/RGBFormulae>
#include "Command.h"
#include "TransferFunctionEditorMain.h"

namespace kvs
{
namespace visclient
{
class Command;

class StepLabel : public kvs::glut::Label
{
private:
    Command* m_command;
    FontType m_fonttype;

public:

    StepLabel( Command* command );

    void screenUpdated();
    void setFontType( const FontType& ftype );
};

class SizeLabel : public kvs::glut::Label
{
public:

    SizeLabel( Command* command );

    void screenUpdated();
};

class NodesLabel : public kvs::glut::Label
{
public:

    NodesLabel( kvs::visclient::Command* command );

    void screenUpdated();
};


class ElementsLabel : public kvs::glut::Label
{
public:

    ElementsLabel( kvs::visclient::Command* command );

    void screenUpdated();
};

class FPSLabel : public kvs::glut::Label
{
private:
    const kvs::RendererBase* m_renderer;
    FontType m_fonttype;

public:

    FPSLabel( kvs::ScreenBase* screen, const kvs::RendererBase& m_renderer );

    void screenUpdated();
    void setFontType( const FontType& ftype );
};

// 2016.1.14 Add by FEAST
class LegendBar : public kvs::glut::LegendBar
{
private:
    const Command* m_command;
    int m_selected_transfer_function;
    FontType m_fonttype;

public:

    LegendBar( kvs::ScreenBase* screen, const Command& command ) :
        kvs::glut::LegendBar( screen ), m_command( &command ), m_fonttype( kvs::glut::LegendBar::BITMAP_8_BY_13 )
    {
        setWidth( 200 );
        setHeight( 50 );
    }

    void screenUpdated();

    void screenResizedAfterSelectTransferFunction( const int i );
    void setFontType( FontType ftype );

    void screenResized();

    void set_hide();
    void set_show();
    const Command* getCommand() const {return m_command;};
};

}
}

#endif    // PBVR__KVS__VISCLIENT__LABEL_H_INCLUDE

