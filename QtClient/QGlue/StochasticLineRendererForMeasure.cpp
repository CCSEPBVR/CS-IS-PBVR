/*****************************************************************************/
/**
 *  @file   StochasticLineRenderer.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#include "StochasticLineRendererForMeasure.h"

#include "TimeMeasureUtility.h"

#include "objnameutil.h"

#include <cmath>
#include <kvs/OpenGL>
#include <kvs/LineObject>
#include <kvs/Camera>
#include <kvs/Light>
#include <kvs/Assert>
#include <kvs/Message>
#include <kvs/Xorshift128>


namespace
{

/*===========================================================================*/
/**
 *  @brief  Returns a random number as integer value.
 *  @return random number
 */
/*===========================================================================*/
int RandomNumber()
{
    const int C = 12347;
    static kvs::Xorshift128 R;
    return C * R.randInteger();
}

/*===========================================================================*/
/**
 *  @brief  Returns vertex-color array.
 *  @param  line [in] pointer to the line object
 */
/*===========================================================================*/
kvs::ValueArray<kvs::UInt8> VertexColors( const kvs::LineObject* line )
{
    if ( line->colorType() == kvs::LineObject::VertexColor ) return line->colors();

    const size_t nvertices = line->numberOfVertices();
    const kvs::RGBColor color = line->color();

    kvs::ValueArray<kvs::UInt8> colors( nvertices * 3 );
    for ( size_t i = 0; i < nvertices; i++ )
    {
        colors[ 3 * i + 0 ] = color.r();
        colors[ 3 * i + 1 ] = color.g();
        colors[ 3 * i + 2 ] = color.b();
    }

    return colors;
}

}

namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new StochasticLineRenderer class.
 */
/*===========================================================================*/
StochasticLineRenderer::StochasticLineRenderer():
    StochasticRendererBase( new Engine() )
{
}

/*===========================================================================*/
/**
 *  @brief  Sets a line offset value.
 *  @param  offset [in] offset value
 */
/*===========================================================================*/
void StochasticLineRenderer::setLineOffset( const float offset )
{
    static_cast<Engine&>( engine() ).setLineOffset( offset );
}

/*===========================================================================*/
/**
 *  @brief  Sets an opacity value.
 *  @param  opacity [in] opacity value
 */
/*===========================================================================*/
void StochasticLineRenderer::setOpacity( const kvs::UInt8 opacity )
{
    static_cast<Engine&>( engine() ).setOpacity( opacity );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Engine class.
 */
/*===========================================================================*/
StochasticLineRenderer::Engine::Engine():
    m_line_opacity( 255 ),
    m_has_connection( false ),
    m_line_offset( 0.0f )
{
}

/*===========================================================================*/
/**
 *  @brief  Releases the GPU resources.
 */
/*===========================================================================*/
void StochasticLineRenderer::Engine::release()
{
#ifdef ENABLE_SHADER_PROGRAM_RELEASE
    m_shader_program.release();
#endif
    m_vbo_manager.release();
}

/*===========================================================================*/
/**
 *  @brief  Create.
 *  @param  object [in] pointer to the line object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void StochasticLineRenderer::Engine::create(
    kvs::ObjectBase* object,
    kvs::Camera* camera,
    kvs::Light* light )
{
#ifdef ENABLE_TIME_MEASURE
    const std::string name = object->name();
    std::string label_index = "0";
    if(name == OBJ_NAME_CONTROL_SPHERE){
        label_index = "2";
    } else if(name == OBJ_NAME_BOUNDING_BOX){
        label_index = "3";
    } else if(name == OBJ_NAME_BOUNDING_BOX_SUBMESH){
        label_index = "4";
    }
    const std::string label_11 = "(A_1_2_4_" + label_index + "_6_1)";
    const std::string label_21 = "(A_2_2_4_" + label_index + "_6_1)";
    const std::string label_12 = "(A_1_2_4_" + label_index + "_6_2)";
    const std::string label_22 = "(A_2_2_4_" + label_index + "_6_2)";
    const std::string label_13 = "(A_1_2_4_" + label_index + "_6_3)";
    const std::string label_23 = "(A_2_2_4_" + label_index + "_6_3)";
    MAKE_AND_START_TIMER(A_XXX_1);
#endif
    kvs::LineObject* line = kvs::LineObject::DownCast( object );
    kvs::LineObject::LineType type = line->lineType();
    m_has_connection = line->numberOfConnections() > 0 &&
        ( type == kvs::LineObject::Segment || type == kvs::LineObject::Uniline );

    attachObject( object );
    createRandomTexture();
#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(A_XXX_1, label_11 + "  " + name + " createRandomTexture", label_21 + "  " + name + " createRandomTexture");
#endif

#ifdef ENABLE_TIME_MEASURE
    MAKE_AND_START_TIMER(A_XXX_2);
#endif
    this->create_shader_program();
#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(A_XXX_2, label_12 + "  " + name + " create_shader_program", label_22 + "  " + name + " create_shader_program");
#endif

#ifdef ENABLE_TIME_MEASURE
    MAKE_AND_START_TIMER(A_XXX_3);
#endif
    this->create_buffer_object( line );
#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(A_XXX_3, label_13 + "  " + name + " create_buffer_object", label_23 + "  " + name + " create_buffer_object");
#endif
}

/*===========================================================================*/
/**
 *  @brief  Update.
 *  @param  object [in] pointer to the object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void StochasticLineRenderer::Engine::update(
    kvs::ObjectBase* object,
    kvs::Camera* camera,
    kvs::Light* light )
{
}

/*===========================================================================*/
/**
 *  @brief  Set up.
 *  @param  object [in] pointer to the object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void StochasticLineRenderer::Engine::setup(
    kvs::ObjectBase* object,
    kvs::Camera* camera,
    kvs::Light* light )
{
    const kvs::Mat4 M = kvs::OpenGL::ModelViewMatrix();
    const kvs::Mat4 PM = kvs::OpenGL::ProjectionMatrix() * M;
    m_shader_program.bind();
    m_shader_program.setUniform( "ModelViewProjectionMatrix", PM );
    m_shader_program.setUniform( "random_texture_size_inv", 1.0f / randomTextureSize() );
    m_shader_program.setUniform( "random_texture", 0 );
    m_shader_program.setUniform( "opacity", m_line_opacity / 255.0f );
    m_shader_program.setUniform( "line_offset", m_line_offset );
    m_shader_program.unbind();
}

/*===========================================================================*/
/**
 *  @brief  Draw an ensemble.
 *  @param  object [in] pointer to the line object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void StochasticLineRenderer::Engine::draw(
    kvs::ObjectBase* object,
    kvs::Camera* camera,
    kvs::Light* light )
{
    kvs::LineObject* line = kvs::LineObject::DownCast( object );

    kvs::VertexBufferObjectManager::Binder bind1( m_vbo_manager );
    kvs::ProgramObject::Binder bind2( m_shader_program );
    kvs::Texture::Binder bind3( randomTexture() );
    {
        kvs::OpenGL::WithEnabled d( GL_DEPTH_TEST );

        const size_t size = randomTextureSize();
        const int count = repetitionCount() * ::RandomNumber();
        const float offset_x = static_cast<float>( ( count ) % size );
        const float offset_y = static_cast<float>( ( count / size ) % size );
        const kvs::Vec2 random_offset( offset_x, offset_y );
        m_shader_program.setUniform( "random_offset", random_offset );

        kvs::OpenGL::SetLineWidth( line->size() );
        if ( m_has_connection )
        {
            const size_t nlines = line->numberOfConnections();
            switch ( line->lineType() )
            {
            case kvs::LineObject::Uniline:
                m_vbo_manager.drawElements( GL_LINE_STRIP, nlines );
                break;
            case kvs::LineObject::Segment:
                m_vbo_manager.drawElements( GL_LINES, 2 * nlines );
                break;
            default:
                break;
            }
        }
        else
        {
            switch ( line->lineType() )
            {
            case kvs::LineObject::Polyline:
                m_vbo_manager.drawArrays( GL_LINE_STRIP, m_first_array, m_count_array );
                break;
            case kvs::LineObject::Strip:
                m_vbo_manager.drawArrays( GL_LINE_STRIP, 0, line->numberOfVertices() );
                break;
            default:
                break;
            }
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Creates shader program.
 */
/*===========================================================================*/
void StochasticLineRenderer::Engine::create_shader_program()
{
#ifdef ENABLE_TIME_MEASURE
    const std::string name = object()->name();
    std::string label_index = "0";
    if(name == OBJ_NAME_CONTROL_SPHERE){
        label_index = "2";
    } else if(name == OBJ_NAME_BOUNDING_BOX){
        label_index = "3";
    } else if(name == OBJ_NAME_BOUNDING_BOX_SUBMESH){
        label_index = "4";
    }
    const std::string label_11 = "(A_1_2_4_" + label_index + "_6_2_1)";
    const std::string label_21 = "(A_2_2_4_" + label_index + "_6_2_1)";
    const std::string label_12 = "(A_1_2_4_" + label_index + "_6_2_2)";
    const std::string label_22 = "(A_2_2_4_" + label_index + "_6_2_2)";
    MAKE_AND_START_TIMER(A_XXX_1);
#endif
#ifdef ENABLE_LOAD_SHADER_STATIC
    static kvs::ShaderSource vert( "SR_line.vert" );
    static kvs::ShaderSource frag( "SR_line.frag" );
#else
    kvs::ShaderSource vert( "SR_line.vert" );
    kvs::ShaderSource frag( "SR_line.frag" );
#endif
#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(A_XXX_1, label_11 + "  " + name + " read_shader_source", label_21 + "  " + name + " read_shader_source");
#endif

#ifdef ENABLE_TIME_MEASURE
    MAKE_AND_START_TIMER(A_XXX_2);
#endif
#ifdef ENABLE_LOAD_SHADER_STATIC
    bool isLinked = m_shader_program.isLinked();
#else
    bool isLinked = false;
#endif
    if(!isLinked){
        m_shader_program.build( vert, frag );
    }
#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(A_XXX_2, label_12 + "  " + name + " m_shader_program.build()", label_22 + "  " + name + " m_shader_program.build()");
#endif
}

/*===========================================================================*/
/**
 *  @brief  Create buffer objects.
 *  @param  line [in] pointer to the line object
 */
/*===========================================================================*/
void StochasticLineRenderer::Engine::create_buffer_object( const kvs::LineObject* line )
{
    if ( line->numberOfColors() != 1 && line->colorType() == kvs::LineObject::LineColor )
    {
        kvsMessageError() << "Not supported line color type." << std::endl;
        return;
    }

    const size_t nvertices = line->numberOfVertices();
    kvs::ValueArray<kvs::UInt16> indices( nvertices * 2 );
    for ( size_t i = 0; i < nvertices; i++ )
    {
        const unsigned int count = i * 12347;
        indices[ 2 * i + 0 ] = static_cast<kvs::UInt16>( ( count ) % randomTextureSize() );
        indices[ 2 * i + 1 ] = static_cast<kvs::UInt16>( ( count / randomTextureSize() ) % randomTextureSize() );
    }
    kvs::ValueArray<kvs::Real32> coords = line->coords();
    kvs::ValueArray<kvs::UInt8> colors = ::VertexColors( line );

    m_vbo_manager.setVertexAttribArray( indices, m_shader_program.attributeLocation("random_index"), 2 );
    m_vbo_manager.setVertexArray( coords, 3 );
    m_vbo_manager.setColorArray( colors, 3 );
    if ( m_has_connection ) { m_vbo_manager.setIndexArray( line->connections() ); }
    m_vbo_manager.create();

    if ( ( !m_has_connection ) && ( line->lineType() == kvs::LineObject::Polyline ) )
    {
        const kvs::UInt32* pconnections = line->connections().data();
        m_first_array.allocate( line->numberOfConnections() );
        m_count_array.allocate( m_first_array.size() );
        for ( size_t i = 0; i < m_first_array.size(); ++i )
        {
            m_first_array[i] = pconnections[ 2 * i ];
            m_count_array[i] = pconnections[ 2 * i + 1 ] - pconnections[ 2 * i ] + 1;
        }
    }
}

} // end of namespace kvs
