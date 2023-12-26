#include "TransferFunctionEditor.h"
#include "ui_TransferFunctionEditor.h"
#include <kvs/ColorMapBar>
#include <kvs/OpacityMapBar>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
class TFColorMapBar : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    TFColorMapBar(QWidget *parent = nullptr) : QOpenGLWidget(parent)
    {
        this->setMinimumHeight(80);
        this->setMaximumHeight(80);

        m_colormap.setResolution( 256 );
        m_colormap.create();
    }

protected:
    void initializeGL() override
    {
        initializeOpenGLFunctions();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Set clear color to white
    }

    void paintGL() override
    {
        if( !m_texture_downloaded )
        {
            const size_t nchannels = 3;
            const size_t width = m_colormap.resolution();
            const size_t height = 1;
            const kvs::UInt8* data = m_colormap.table().data();

            m_texture.release();
            m_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
            m_texture.setMinFilter( GL_NEAREST );
            m_texture.setMagFilter( GL_NEAREST );
            m_texture.create( width, height, data );
            m_texture_downloaded = true;
        }

        kvs::OpenGL::WithPushedAttrib attrib( GL_ALL_ATTRIB_BITS );
        attrib.disable( GL_BLEND );
        attrib.disable( GL_DEPTH_TEST );
        attrib.disable( GL_TEXTURE_3D );
        attrib.enable( GL_TEXTURE_2D );

        const kvs::Vec2 p0 = kvs::Vec2( -1, -1 );
        const kvs::Vec2 p1 = kvs::Vec2(  1, -1 );
        const kvs::Vec2 p2 = kvs::Vec2(  1,  1 );
        const kvs::Vec2 p3 = kvs::Vec2( -1,  1 );

        kvs::Texture::Binder binder( m_texture );

        kvs::OpenGL::Begin( GL_QUADS );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 1.0f ), p0 );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 1.0f ), p1 );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 1.0f, 0.0f ), p2 );
        kvs::OpenGL::TexCoordVertex( kvs::Vec2( 0.0f, 0.0f ), p3 );
        kvs::OpenGL::End();
    }

    void resizeGL(int w, int h) override
    {
        glViewport(0, 0, w, h);
    }
private:
    kvs::ColorMap m_colormap; ///< color map
    kvs::Texture2D m_texture; ///< texture data
    bool m_texture_downloaded; ///< check flag for texture
};

TransferFunctionEditor::TransferFunctionEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferFunctionEditor)
{
    ui->setupUi(this);
    TFColorMapBar* tr1 = new TFColorMapBar();    
;

    ui->colorMapBarGLayout->addWidget( tr1, 0,0,1,1 );    
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    delete ui;
}


