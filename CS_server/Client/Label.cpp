#include "Label.h"

#include "Command.h"

#include "kvs/RendererBase"

using namespace kvs::visclient;

#if 1 // FEAST modify 20160128
extern int g_curStep;
#endif // FEAST modify 20160128 end


StepLabel::StepLabel( Command* command ) : kvs::glut::Label( command->m_screen ), m_command( command ), m_fonttype( kvs::glut::LegendBar::BITMAP_8_BY_13 )
{
    setMargin( 10 );
}

void StepLabel::screenUpdated()
{
    std::stringstream step;
    // FEAST modify 20160128
    //step << std::setprecision(4) << command->param.m_time_step;
    if ( g_curStep < 0 )
        step << "---";
    else
        step << std::setprecision( 4 ) << g_curStep;
    // FEAST modify 20160128 end
    setText( std::string( "time step: " + step.str() ).c_str() );
    setCharacterType( m_fonttype );
}

void StepLabel::setFontType( const FontType& ftype )
{
    m_fonttype = ftype;
}

SizeLabel::SizeLabel( Command* command ) : kvs::glut::Label( command->m_screen )
{
    setMargin( 10 );
}

void SizeLabel::screenUpdated()
{
    setText( std::string( "Data size: 4.2 GB/step" ).c_str() );
}

NodesLabel::NodesLabel( kvs::visclient::Command* command ): kvs::glut::Label( command->m_screen )
{
    setMargin( 10 );
}

void NodesLabel::screenUpdated()
{
    setText( std::string( "Nodes    :  26,047,774" ).c_str() );
}

ElementsLabel::ElementsLabel( kvs::visclient::Command* command ): kvs::glut::Label( command->m_screen )
{
    setMargin( 10 );
}

void ElementsLabel::screenUpdated()
{
    setText( std::string( "Elements : 126,713,831" ).c_str() );
}

FPSLabel::FPSLabel( kvs::ScreenBase* screen, const kvs::RendererBase& m_renderer ):
    kvs::glut::Label( screen ),
    m_renderer( &m_renderer ),
    m_fonttype( kvs::glut::LegendBar::BITMAP_8_BY_13 )
{
    setMargin( 10 );
}

void FPSLabel::setFontType( const FontType& ftype )
{
    m_fonttype = ftype;
}

void FPSLabel::screenUpdated()
{
    std::stringstream fps;
    fps << std::setprecision( 4 ) << m_renderer->timer().fps();
    setText( std::string( "fps: " + fps.str() ).c_str() );
    setCharacterType( m_fonttype );
}

void LegendBar::setFontType( FontType ftype )
{
    m_fonttype = ftype;
}

void LegendBar::screenUpdated()
{
    kvs::visclient::ExtendedTransferFunctionMessage m_doc;
    kvs::ColorMap color_map;

    m_doc = m_command->m_parameter.m_parameter_extend_transfer_function;
    // modify by @hira at 2016/12/01
    // color_map = m_doc.m_transfer_function[m_selected_transfer_function].colorMap();
    NamedTransferFunctionParameter* color_trans = m_doc.getColorTransferFunction(m_selected_transfer_function);
    if (color_trans == NULL) {
        return;
    }
    color_map = color_trans->colorMap();

    setRange( color_trans->m_color_variable_min, color_trans->m_color_variable_max );
    setColorMap( color_map );

    setCharacterType( m_fonttype );
}

void LegendBar::screenResizedAfterSelectTransferFunction( const int i )
{
    m_selected_transfer_function = i;
    screenResized();
}

void LegendBar::screenResized()
{
    if ( m_orientation == Horizontal )
    {
        setWidth( 200 );
        setHeight( 45 );
        setX( 15 );
        setY( 15 );
        m_reset_height = true;
        m_reset_width = true;
    }
    else
    {
        setWidth( 150 );
        setHeight( 220 );
        setX( 15 );
        setY( 15 );
        m_reset_height = true;
        m_reset_width = true;
    }
    screenUpdated();
}
