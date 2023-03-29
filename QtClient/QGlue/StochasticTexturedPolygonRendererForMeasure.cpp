/*****************************************************************************/
/**
 *  @file   StochasticTexturedPolygonRenderer.cpp
 *  @author Naohisa Sakamoto, Zhao Kun
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
#include "StochasticTexturedPolygonRendererForMeasure.h"
#include <cmath>
#include <kvs/OpenGL>
#include <kvs/Camera>
#include <kvs/Light>
#include <kvs/Assert>
#include <kvs/Message>
#include <kvs/Xorshift128>

#include "TimeMeasureUtility.h"
#include "objnameutil.h"

#define TEX_INDEX_RANDOM 0
#define TEX_INDEX_MATERIAL 1

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
 *  @brief  Returns true if the textured polygon object has the connectivity.
 *  @param  texturedPolygon [in] pointer to the textured polygon object
 *  @return true if the textured polygon object has the connectivity
 */
/*===========================================================================*/
bool HasConnections( const kvs::jaea::TexturedPolygonObject* texturedPolygon )
{
    bool has_connection = texturedPolygon->numberOfConnections() > 0;
    
    // In the following cases, the connection stored in the polygon object will be ignored.
    if ( texturedPolygon->normalType() == kvs::jaea::TexturedPolygonObject::PolygonNormal ) { has_connection = false; }
    if ( texturedPolygon->colorType() == kvs::jaea::TexturedPolygonObject::PolygonColor ) { has_connection = false; }
    
    return has_connection;
}

/*===========================================================================*/
/**
 *  @brief  Returns number of vertices of the textured polygon object
 *  @param  texturedPolygon [in] pointer to the textured polygon object
 *  @param  texture_id [in] texture id
 *  @return number of vertices
 */
/*===========================================================================*/
size_t NumberOfVertices( const kvs::jaea::TexturedPolygonObject* texturedPolygon, kvs::UInt32 texture_id )
{
    if ( texturedPolygon->connections().size() > 0 )
    {
        const size_t nfaces = texturedPolygon->numberOfConnections();
        const kvs::UInt32* polygon_connections = texturedPolygon->connections().data();
        const kvs::UInt32* vertex_texture_id = texturedPolygon->textureIds().data();
        
        size_t nvertices = 0;
        for ( size_t i = 0; i < nfaces; i++ )
        {
            const kvs::UInt32 id0 = polygon_connections[ i * 3 + 0 ];
            const kvs::UInt32 id1 = polygon_connections[ i * 3 + 1 ];
            const kvs::UInt32 id2 = polygon_connections[ i * 3 + 2 ];
            
            const kvs::UInt32 texture_id0 = vertex_texture_id[ id0 ];
            const kvs::UInt32 texture_id1 = vertex_texture_id[ id1 ];
            const kvs::UInt32 texture_id2 = vertex_texture_id[ id2 ];
            
            if( texture_id == texture_id0 && texture_id == texture_id1 && texture_id == texture_id2 ){
                nvertices += 3;
            } else if(texture_id0 != texture_id1 || texture_id0 != texture_id2 || texture_id1 != texture_id2 ) {
                std::cerr << "!!! texture id mismatch !!!" << std::endl;
            }
        }
        
        return nvertices;
    }
    
    return 0;
}

/*===========================================================================*/
/**
 *  @brief  Returns coordinate array.
 *  @param  texturedPolygon [in] pointer to the textured polygon object
 *  @param  texture_id [in] texture id
 *  @param  nvertices [in] number of vertices
 */
/*===========================================================================*/
kvs::ValueArray<kvs::Real32> VertexCoords(
        const kvs::jaea::TexturedPolygonObject* texturedPolygon,
        const kvs::UInt32 texture_id,
        const size_t nvertices )
{
    const size_t nfaces = texturedPolygon->numberOfConnections();
    const kvs::Real32* vertex_coords = texturedPolygon->coords().data();
    const kvs::UInt32* vertex_texture_id = texturedPolygon->textureIds().data();
    const kvs::UInt32* polygon_connections = texturedPolygon->connections().data();
    
    kvs::ValueArray<kvs::Real32> coords( nvertices * 3 );
    size_t offset = 0;
    for ( size_t i = 0; i < nfaces; i++ )
    {
        const kvs::UInt32 id0 = polygon_connections[ i * 3 + 0 ];
        const kvs::UInt32 id1 = polygon_connections[ i * 3 + 1 ];
        const kvs::UInt32 id2 = polygon_connections[ i * 3 + 2 ];
        
        const kvs::UInt32 texture_id0 = vertex_texture_id[ id0 ];
        const kvs::UInt32 texture_id1 = vertex_texture_id[ id1 ];
        const kvs::UInt32 texture_id2 = vertex_texture_id[ id2 ];
        
        if( texture_id == texture_id0 && texture_id == texture_id1 && texture_id == texture_id2 ){
            coords[ offset + 0 ] = vertex_coords[ id0 * 3 + 0 ];
            coords[ offset + 1 ] = vertex_coords[ id0 * 3 + 1 ];
            coords[ offset + 2 ] = vertex_coords[ id0 * 3 + 2 ];
            
            coords[ offset + 3 ] = vertex_coords[ id1 * 3 + 0 ];
            coords[ offset + 4 ] = vertex_coords[ id1 * 3 + 1 ];
            coords[ offset + 5 ] = vertex_coords[ id1 * 3 + 2 ];
            
            coords[ offset + 6 ] = vertex_coords[ id2 * 3 + 0 ];
            coords[ offset + 7 ] = vertex_coords[ id2 * 3 + 1 ];
            coords[ offset + 8 ] = vertex_coords[ id2 * 3 + 2 ];
            offset += 9;
        } else if(texture_id0 != texture_id1 || texture_id0 != texture_id2 || texture_id1 != texture_id2 ) {
            std::cerr << "!!! texture id mismatch !!!" << std::endl;
        }
    }
    
    return coords;
}

/*===========================================================================*/
/**
 *  @brief  Returns vertex-normal array.
 *  @param  texturedPolygon [in] pointer to the textured polygon object
 *  @param  texture_id [in] texture id
 *  @param  nvertices [in] number of vertices
 */
/*===========================================================================*/
kvs::ValueArray<kvs::Real32> VertexNormals(
        const kvs::jaea::TexturedPolygonObject* texturedPolygon,
        const kvs::UInt32 texture_id,
        const size_t nvertices )
{
    if ( texturedPolygon->normals().size() == 0 )
    {
        return kvs::ValueArray<kvs::Real32>();
    }
    
    const size_t nfaces = texturedPolygon->numberOfConnections();
    const kvs::Real32* vertex_coords = texturedPolygon->coords().data();
    const kvs::UInt32* vertex_texture_id = texturedPolygon->textureIds().data();
    const kvs::UInt32* polygon_connections = texturedPolygon->connections().data();
    //const kvs::Real32* vertex_normals = texturedPolygon->normals().data();
    
    kvs::ValueArray<kvs::Real32> normals( nvertices * 3 );
    size_t offset = 0;
    for ( size_t i = 0; i < nfaces; i++ )
    {
        const kvs::UInt32 id0 = polygon_connections[ i * 3 + 0 ];
        const kvs::UInt32 id1 = polygon_connections[ i * 3 + 1 ];
        const kvs::UInt32 id2 = polygon_connections[ i * 3 + 2 ];
        
        const kvs::UInt32 texture_id0 = vertex_texture_id[ id0 ];
        const kvs::UInt32 texture_id1 = vertex_texture_id[ id1 ];
        const kvs::UInt32 texture_id2 = vertex_texture_id[ id2 ];
        
        if( texture_id == texture_id0 && texture_id == texture_id1 && texture_id == texture_id2 ){
            {
                // use normals of vertices
                //normals[ offset + 0 ] = vertex_normals[ id0 * 3 + 0 ];
                //normals[ offset + 1 ] = vertex_normals[ id0 * 3 + 1 ];
                //normals[ offset + 2 ] = vertex_normals[ id0 * 3 + 2 ];
                //
                //normals[ offset + 3 ] = vertex_normals[ id1 * 3 + 0 ];
                //normals[ offset + 4 ] = vertex_normals[ id1 * 3 + 1 ];
                //normals[ offset + 5 ] = vertex_normals[ id1 * 3 + 2 ];
                //
                //normals[ offset + 6 ] = vertex_normals[ id2 * 3 + 0 ];
                //normals[ offset + 7 ] = vertex_normals[ id2 * 3 + 1 ];
                //normals[ offset + 8 ] = vertex_normals[ id2 * 3 + 2 ];
            }
            {
                // compute normal of triangle
                float x0 = vertex_coords[ id0 * 3 + 0 ];
                float y0 = vertex_coords[ id0 * 3 + 1 ];
                float z0 = vertex_coords[ id0 * 3 + 2 ];
                float x1 = vertex_coords[ id1 * 3 + 0 ];
                float y1 = vertex_coords[ id1 * 3 + 1 ];
                float z1 = vertex_coords[ id1 * 3 + 2 ];
                float x2 = vertex_coords[ id2 * 3 + 0 ];
                float y2 = vertex_coords[ id2 * 3 + 1 ];
                float z2 = vertex_coords[ id2 * 3 + 2 ];
                
                float v1x = x1 - x0;
                float v1y = y1 - y0;
                float v1z = z1 - z0;
                float v2x = x2 - x0;
                float v2y = y2 - y0;
                float v2z = z2 - z0;
                float nx = v1y * v2z - v1z * v2y;
                float ny = v1z * v2x - v1x * v2z;
                float nz = v1x * v2y - v1y * v2x;
                float length = sqrt( nx * nx + ny * ny + nz * nz );
                if(length > 0){
                    nx /= length;
                    ny /= length;
                    nz /= length;
                }
                
                normals[ offset + 0 ] = nx;
                normals[ offset + 1 ] = ny;
                normals[ offset + 2 ] = nz;
                
                normals[ offset + 3 ] = nx;
                normals[ offset + 4 ] = ny;
                normals[ offset + 5 ] = nz;
                
                normals[ offset + 6 ] = nx;
                normals[ offset + 7 ] = ny;
                normals[ offset + 8 ] = nz;
            }
            
            offset += 9;
        } else if(texture_id0 != texture_id1 || texture_id0 != texture_id2 || texture_id1 != texture_id2 ) {
            std::cerr << "!!! texture id mismatch !!!" << std::endl;
        }
    }
    
    return normals;
}

/*===========================================================================*/
/**
 *  @brief  Returns vertex-color array.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
kvs::ValueArray<kvs::UInt8> VertexColors(
        const kvs::jaea::TexturedPolygonObject* texturedPolygon,
        const kvs::UInt32 texture_id,
        const size_t nvertices )
{
    const size_t nfaces = texturedPolygon->numberOfConnections();
    const kvs::UInt8* vertex_colors = texturedPolygon->colors().data();
    const kvs::UInt32* vertex_texture_id = texturedPolygon->textureIds().data();
    const kvs::UInt32* polygon_connections = texturedPolygon->connections().data();
    
    kvs::ValueArray<kvs::UInt8> colors( nvertices * 4 );
    size_t offset = 0;
    for ( size_t i = 0; i < nfaces; i++ )
    {
        const kvs::UInt32 id0 = polygon_connections[ i * 3 + 0 ];
        const kvs::UInt32 id1 = polygon_connections[ i * 3 + 1 ];
        const kvs::UInt32 id2 = polygon_connections[ i * 3 + 2 ];
        
        const kvs::UInt32 texture_id0 = vertex_texture_id[ id0 ];
        const kvs::UInt32 texture_id1 = vertex_texture_id[ id1 ];
        const kvs::UInt32 texture_id2 = vertex_texture_id[ id2 ];
        
        if( texture_id == texture_id0 && texture_id == texture_id1 && texture_id == texture_id2 ){
            colors[ offset + 0 ] = vertex_colors[ id0 * 3 + 0 ];
            colors[ offset + 1 ] = vertex_colors[ id0 * 3 + 1 ];
            colors[ offset + 2 ] = vertex_colors[ id0 * 3 + 2 ];
            colors[ offset + 3 ] = 255;
            
            colors[ offset + 4 ] = vertex_colors[ id1 * 3 + 0 ];
            colors[ offset + 5 ] = vertex_colors[ id1 * 3 + 1 ];
            colors[ offset + 6 ] = vertex_colors[ id1 * 3 + 2 ];
            colors[ offset + 7 ] = 255;
            
            colors[ offset + 8 ] = vertex_colors[ id2 * 3 + 0 ];
            colors[ offset + 9 ] = vertex_colors[ id2 * 3 + 1 ];
            colors[ offset + 10 ] = vertex_colors[ id2 * 3 + 2 ];
            colors[ offset + 11 ] = 255;
            offset += 12;
        } else if(texture_id0 != texture_id1 || texture_id0 != texture_id2 || texture_id1 != texture_id2 ) {
            std::cerr << "!!! texture id mismatch !!!" << std::endl;
        }
    }
    
    return colors;
}

/*===========================================================================*/
/**
 *  @brief  Returns texture-coord array.
 *  @param  texturedPolygon [in] pointer to the textured polygon object
 *  @param  texture_id [in] texture id
 *  @param  nvertices [in] number of vertices
 */
/*===========================================================================*/
kvs::ValueArray<kvs::Real32> VertexTexture2DCoords(
        const kvs::jaea::TexturedPolygonObject* texturedPolygon,
        const kvs::UInt32 texture_id,
        const size_t nvertices )
{
    const size_t nfaces = texturedPolygon->numberOfConnections();
    const kvs::Real32* vertex_texture_2d_coords = texturedPolygon->texture2DCoords().data();
    const kvs::UInt32* vertex_texture_id = texturedPolygon->textureIds().data();
    const kvs::UInt32* polygon_connections = texturedPolygon->connections().data();
    
    kvs::ValueArray<kvs::Real32> texture_2d_coords( nvertices * 2 );
    size_t offset = 0;
    for ( size_t i = 0; i < nfaces; i++ )
    {
        const kvs::UInt32 id0 = polygon_connections[ i * 3 + 0 ];
        const kvs::UInt32 id1 = polygon_connections[ i * 3 + 1 ];
        const kvs::UInt32 id2 = polygon_connections[ i * 3 + 2 ];
        
        const kvs::UInt32 texture_id0 = vertex_texture_id[ id0 ];
        const kvs::UInt32 texture_id1 = vertex_texture_id[ id1 ];
        const kvs::UInt32 texture_id2 = vertex_texture_id[ id2 ];
        
        if( texture_id == texture_id0 && texture_id == texture_id1 && texture_id == texture_id2 ){
            texture_2d_coords[ offset + 0 ] = vertex_texture_2d_coords[ id0 * 2 + 0 ];
            texture_2d_coords[ offset + 1 ] = vertex_texture_2d_coords[ id0 * 2 + 1 ];
            
            texture_2d_coords[ offset + 2 ] = vertex_texture_2d_coords[ id1 * 2 + 0 ];
            texture_2d_coords[ offset + 3 ] = vertex_texture_2d_coords[ id1 * 2 + 1 ];
            
            texture_2d_coords[ offset + 4 ] = vertex_texture_2d_coords[ id2 * 2 + 0 ];
            texture_2d_coords[ offset + 5 ] = vertex_texture_2d_coords[ id2 * 2 + 1 ];
            offset += 6;
        } else if(texture_id0 != texture_id1 || texture_id0 != texture_id2 || texture_id1 != texture_id2 ) {
            std::cerr << "!!! texture id mismatch !!!" << std::endl;
        }
    }
    
    return texture_2d_coords;
}

}


namespace kvs
{
namespace jaea
{
/*===========================================================================*/
/**
 *  @brief  Constructs a new StochasticPolygonRenderer class.
 */
/*===========================================================================*/
StochasticTexturedPolygonRenderer::StochasticTexturedPolygonRenderer():
        StochasticRendererBase( new Engine() )
{
}

/*===========================================================================*/
/**
 *  @brief  Sets a polygon offset.
 *  @param  offset [in] offset value
 */
/*===========================================================================*/
void StochasticTexturedPolygonRenderer::setPolygonOffset( const float offset )
{
    static_cast<Engine&>( engine() ).setPolygonOffset( offset );
}

/*===========================================================================*/
/**
 *  @brief  Sets enabled linear texture interpolation.
 *  @param  offset [in] offset value
 */
/*===========================================================================*/
void StochasticTexturedPolygonRenderer::setEnabledTextureLinear(const bool isEnabled ){
    static_cast<Engine&>( engine() ).setEnabledTextureLinear( isEnabled );
}

/*===========================================================================*/
/**
 *  @brief  Sets enabled texture mipmap.
 *  @param  offset [in] offset value
 */
/*===========================================================================*/
void StochasticTexturedPolygonRenderer::setEnabledTextureMipmap(const bool isEnabled ){
    static_cast<Engine&>( engine() ).setEnabledTextureMipmap( isEnabled );
}

/*===========================================================================*/
/**
 *  @brief  Sets enabled linear texture mipmap interpolation.
 *  @param  offset [in] offset value
 */
/*===========================================================================*/
void StochasticTexturedPolygonRenderer::setEnabledTextureMipmapLinear(const bool isEnabled ){
    static_cast<Engine&>( engine() ).setEnabledTextureMipmapLinear( isEnabled );
}

/*===========================================================================*/
/**
 *  @brief  Sets enabled anisotropy texture interpolation.
 *  @param  offset [in] offset value
 */
/*===========================================================================*/
void StochasticTexturedPolygonRenderer::setEnabledTextureAnisotropy(const bool isEnabled ){
    static_cast<Engine&>( engine() ).setEnabledTextureAnisotropy( isEnabled );
}

void StochasticTexturedPolygonRenderer::setNearestInterpolation(){
    static_cast<Engine&>( engine() ).setEnabledTextureLinear( false );
    static_cast<Engine&>( engine() ).setEnabledTextureMipmap( false );
    static_cast<Engine&>( engine() ).setEnabledTextureMipmapLinear( false );
    static_cast<Engine&>( engine() ).setEnabledTextureAnisotropy( false );
}
void StochasticTexturedPolygonRenderer::setBilinearInterpolation(){
    static_cast<Engine&>( engine() ).setEnabledTextureLinear( true );
    static_cast<Engine&>( engine() ).setEnabledTextureMipmap( false );
    static_cast<Engine&>( engine() ).setEnabledTextureMipmapLinear( false );
    static_cast<Engine&>( engine() ).setEnabledTextureAnisotropy( false );
}
void StochasticTexturedPolygonRenderer::setTrilinearInterpolation(){
    static_cast<Engine&>( engine() ).setEnabledTextureLinear( true );
    static_cast<Engine&>( engine() ).setEnabledTextureMipmap( true );
    static_cast<Engine&>( engine() ).setEnabledTextureMipmapLinear( true );
    static_cast<Engine&>( engine() ).setEnabledTextureAnisotropy( false );
}
void StochasticTexturedPolygonRenderer::setAnisotropicFiltering(){
    static_cast<Engine&>( engine() ).setEnabledTextureLinear( true );
    static_cast<Engine&>( engine() ).setEnabledTextureMipmap( true );
    static_cast<Engine&>( engine() ).setEnabledTextureMipmapLinear( true );
    static_cast<Engine&>( engine() ).setEnabledTextureAnisotropy( true );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Engine class.
 */
/*===========================================================================*/
StochasticTexturedPolygonRenderer::Engine::Engine():
    m_has_normal( false ),
    m_has_connection( false ),
    m_random_index( 0 ),
    m_polygon_offset( 0.0f ),
    m_enabled_texture_linear( false ),
    m_enabled_texture_mipmap( false ),
    m_enabled_texture_mipmap_linear( false ),
    m_enabled_texture_anisotropy( false )
{
}

StochasticTexturedPolygonRenderer::Engine::~Engine()
{
    for(auto itr = m_map_id_to_texture.begin(); itr != m_map_id_to_texture.end(); itr++){
        kvs::UInt32 texture_id = itr->first;
        kvs::Texture2D* texture2d= itr->second;
        delete texture2d;
    }
}


/*===========================================================================*/
/**
 *  @brief  Releases the GPU resources.
 */
/*===========================================================================*/
void StochasticTexturedPolygonRenderer::Engine::release()
{
#ifdef ENABLE_SHADER_PROGRAM_RELEASE
    m_shader_program.release();
#endif
    m_vbo.release();
}

/*===========================================================================*/
/**
 *  @brief  Create shaders, VBO, and framebuffers.
 *  @param  object [in] pointer to the object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void StochasticTexturedPolygonRenderer::Engine::create( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
{
#ifdef ENABLE_TIME_MEASURE
    const std::string name = object->name();
    std::string label_index = "0";
    if(name == OBJ_NAME_BOUNDING_BOX_SCALE_LABELS){
        label_index = "5";
    } else if(name == OBJ_NAME_CGMODEL){
        label_index = "6";
    }
    const std::string label_11 = "(A_1_2_4_" + label_index + "_6_1)";
    const std::string label_21 = "(A_2_2_4_" + label_index + "_6_1)";
    const std::string label_12 = "(A_1_2_4_" + label_index + "_6_2)";
    const std::string label_22 = "(A_2_2_4_" + label_index + "_6_2)";
    const std::string label_13 = "(A_1_2_4_" + label_index + "_6_3)";
    const std::string label_23 = "(A_2_2_4_" + label_index + "_6_3)";
    MAKE_AND_START_TIMER(A_XXX_1);
#endif
    kvs::jaea::TexturedPolygonObject* texturedPolygon = kvs::jaea::TexturedPolygonObject::DownCast( object );
    m_has_normal = ( texturedPolygon->normals().size() > 0 && 
            texturedPolygon->normalType() == kvs::jaea::TexturedPolygonObject::VertexNormal );
    m_has_connection = ::HasConnections( texturedPolygon );
    if ( !m_has_normal ) setEnabledShading( false );
    
    attachObject( object );
#ifdef DEBUG_TEXTURED_POLYGON
    {
        std::cout << "#randomTexture before " << randomTexture().isValid() << ", " << randomTexture().id() << std::endl;
    }
#endif
    createRandomTexture();

    {
#ifdef DEBUG_TEXTURED_POLYGON
        std::cout << "m_enabled_texture_linear="        << m_enabled_texture_linear << std::endl;
        std::cout << "m_enabled_texture_mipmap="        << m_enabled_texture_mipmap << std::endl;
        std::cout << "m_enabled_texture_mipmap_linear=" << m_enabled_texture_mipmap_linear << std::endl;
        std::cout << "m_enabled_texture_anisotropy="    << m_enabled_texture_anisotropy << std::endl;
#endif // DEBUG_TEXTURED_POLYGON
        
        std::map<kvs::UInt32, kvs::ValueArray<kvs::UInt8>> mapIdToColorArray = texturedPolygon->mapIdToColorArray();
        std::map<kvs::UInt32, kvs::UInt32> mapIdToImageWidth = texturedPolygon->mapIdToImageWidth();
        std::map<kvs::UInt32, kvs::UInt32> mapIdToImageHeight = texturedPolygon->mapIdToImageHeight();
        
        for(auto itr = mapIdToColorArray.begin(); itr != mapIdToColorArray.end(); itr++){
            kvs::UInt32 texture_id = itr->first;

#ifdef SKIP_REDUNDANT_CREATE_IMAGE_TEXTURE
            if (m_map_id_to_texture.find(texture_id) != end(m_map_id_to_texture) ) {
                continue;
            }
#endif

            kvs::ValueArray<kvs::UInt8> color_array = itr->second;
            UInt32 width = mapIdToImageWidth[texture_id];
            UInt32 height = mapIdToImageHeight[texture_id];
            
            kvs::Texture2D *texture = new kvs::Texture2D();
            //std::cout << "#a " << texture->isValid() << ", " << texture->id() << std::endl;
            texture->release();
            texture->setWrapS( GL_REPEAT );
            texture->setWrapT( GL_REPEAT );
            {
                texture->setMagFilter( GL_NEAREST );
                if(m_enabled_texture_mipmap){
                    if(m_enabled_texture_linear){
                        if(m_enabled_texture_mipmap_linear){
                            texture->setMinFilter( GL_LINEAR_MIPMAP_LINEAR );
                        }else{
                            texture->setMinFilter( GL_LINEAR_MIPMAP_NEAREST );
                        }
                    }else{
                        if(m_enabled_texture_mipmap_linear){
                            texture->setMinFilter( GL_NEAREST_MIPMAP_LINEAR );
                        }else{
                            texture->setMinFilter( GL_NEAREST_MIPMAP_NEAREST );
                        }
                    }
                }else{
                    if(m_enabled_texture_linear){
                        texture->setMinFilter( GL_LINEAR );
                    }else{
                        texture->setMinFilter( GL_NEAREST );
                    }
                }
            }
            if( m_enabled_texture_anisotropy ){
                float largest = 0.0;
                KVS_GL_CALL( glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest ));
#ifdef DEBUG_TEXTURED_POLYGON
                std::cout << "largest=" << largest << std::endl;
#endif // DEBUG_TEXTURED_POLYGON
                KVS_GL_CALL( glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest ));
            }
            texture->setPixelFormat(GL_RGBA8,  GL_RGBA, GL_UNSIGNED_BYTE);
            //std::cout << "#b " << texture->isValid() << ", " << texture->id() << std::endl;
            texture->create(width, height, color_array.data());
            if( m_enabled_texture_mipmap ){
                kvs::Texture::GuardedBinder bind( *texture );
                KVS_GL_CALL( glGenerateMipmap( GL_TEXTURE_2D ) );
            }
#ifdef DEBUG_TEXTURED_POLYGON
            std::cout << "## " <<
                    "id="        << texture_id << ", " <<
                    "width="     << texture->width() << ", " <<
                    "height="    << texture->height() << ", " <<
                    "glid="      << texture->id() << ", " <<
                    "isCreated=" << texture->isCreated() << ", " <<
                    "isValid="   << texture->isValid() << ", " <<
                    "isBound="   << texture->isBound() << ", " <<
                    std::endl;
#endif // DEBUG_TEXTURED_POLYGON
            {
                //std::cout << "# " <<  std::endl;
                //for(size_t j=0;j<image.height();j++){
                //    for(size_t i=0;i<image.width();i++){
                //        std::cout << (int)(image.pixels().data()[i+j*image.width()]) << ", ";
                //    }
                //    std::cout << std::endl;
                //}
            }
            //texturedPolygon->addTexture2D(texture_id, *texture);
            m_map_id_to_texture[texture_id] = texture;
        }
    }
    
    {
#ifdef DEBUG_TEXTURED_POLYGON
        std::cout << "#randomTexture after " << randomTexture().isValid() << ", " << randomTexture().id() << std::endl;
#endif // DEBUG_TEXTURED_POLYGON
    }
#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(A_XXX_1, label_11 + "  " + name + " create_texture_data", label_21 + "  " + name + " create_texture_data");
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
    this->create_buffer_object( texturedPolygon );
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
void StochasticTexturedPolygonRenderer::Engine::update( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
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
void StochasticTexturedPolygonRenderer::Engine::setup( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
{
    const kvs::Mat4 M = kvs::OpenGL::ModelViewMatrix();
    const kvs::Mat4 PM = kvs::OpenGL::ProjectionMatrix() * M;
    const kvs::Mat3 N = kvs::Mat3( M[0].xyz(), M[1].xyz(), M[2].xyz() );
    m_shader_program.bind();
    m_shader_program.setUniform( "ModelViewMatrix", M );
    m_shader_program.setUniform( "ModelViewProjectionMatrix", PM );
    m_shader_program.setUniform( "NormalMatrix", N );
    m_shader_program.setUniform( "random_texture_size_inv", 1.0f / randomTextureSize() );
    m_shader_program.setUniform( "random_texture", TEX_INDEX_RANDOM );
    m_shader_program.setUniform( "material_texture", TEX_INDEX_MATERIAL );
    m_shader_program.setUniform( "polygon_offset", m_polygon_offset );
    m_shader_program.unbind();
}

/*===========================================================================*/
/**
 *  @brief  Draw an ensemble.
 *  @param  object [in] pointer to the object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void StochasticTexturedPolygonRenderer::Engine::draw( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
{
    kvs::jaea::TexturedPolygonObject* texturedPolygon = kvs::jaea::TexturedPolygonObject::DownCast( object );
    
    kvs::ProgramObject::Binder bind_shacer_program( m_shader_program );
    KVS_GL_CALL( glActiveTexture( GL_TEXTURE0 ) );
    kvs::Texture::GuardedBinder bind_random_texture( randomTexture() );
    KVS_GL_CALL( glActiveTexture( GL_TEXTURE1 ) );
    
    kvs::VertexBufferObject::Binder bind_vbo( m_vbo );
    
    GLbyte* offset = (GLbyte*)NULL;
    for(auto itr = m_map_id_to_texture.begin(); itr != m_map_id_to_texture.end(); itr++){
        kvs::UInt32 texture_id = itr->first;
        kvs::Texture2D* texture2d= itr->second;
        //std::cout << texture2d->width() << ", " << texture2d->height() << ", " << texture2d->id() << std::endl;
        const size_t nvertices = m_map_num_vertices[texture_id];
        
        kvs::Texture::GuardedBinder bind_texture_2d( *texture2d );
        if( m_enabled_texture_anisotropy ){
            float largest = 0.0;
            KVS_GL_CALL( glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest ));
            //std::cout << "largest=" << largest << std::endl;
            KVS_GL_CALL( glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest ));
        }
        
        {
            kvs::OpenGL::WithEnabled d( GL_DEPTH_TEST );
            
            const size_t size = randomTextureSize();
            const int count = repetitionCount() * ::RandomNumber();
            const float offset_x = static_cast<float>( ( count ) % size );
            const float offset_y = static_cast<float>( ( count / size ) % size );
            const kvs::Vec2 random_offset( offset_x, offset_y );
            m_shader_program.setUniform( "random_offset", random_offset );
            
            const size_t nvertices = m_map_num_vertices[texture_id];
            const size_t npolygons = nvertices / 3;
            const size_t index_size = nvertices * 2 * sizeof( kvs::UInt16 );
            const size_t coord_size = nvertices * 3 * sizeof( kvs::Real32 );
            //const size_t color_size = nvertices * 4 * sizeof( kvs::UInt8 );
            const size_t normal_size = nvertices * 3 * sizeof( kvs::Real32 );
            const size_t texture_2d_coord_size = nvertices * 2 * sizeof( kvs::Real32 );
            
            GLbyte* offset_indices           = offset;
            GLbyte* offset_coords            = offset_indices + index_size;
            //GLbyte* offset_colors            = offset_coords  + coord_size;
            GLbyte* offset_normals           = offset_coords  + coord_size;
            GLbyte* offset_texture_2d_coords = offset_normals + coord_size;
            //offset = offset + index_size + coord_size + color_size + normal_size + texture_2d_coord_size;
            offset = offset + index_size + coord_size + normal_size + texture_2d_coord_size;
            
            KVS_GL_CALL( glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ) );
            
            // Enable coords.
            KVS_GL_CALL( glEnableClientState( GL_VERTEX_ARRAY ) );
            KVS_GL_CALL( glVertexPointer( 3, GL_FLOAT, 0, offset_coords ) );
            
            // Enable colors.
            //KVS_GL_CALL( glEnableClientState( GL_COLOR_ARRAY ) );
            //KVS_GL_CALL( glColorPointer( 4, GL_UNSIGNED_BYTE, 0, offset_colors ) );
            
            // Enable normals.
            if ( m_has_normal )
            {
                KVS_GL_CALL( glEnableClientState( GL_NORMAL_ARRAY ) );
                KVS_GL_CALL( glNormalPointer( GL_FLOAT, 0, offset_normals ) );
            }
            
            // Enable random index.
            KVS_GL_CALL( glEnableVertexAttribArray( m_random_index ) );
            KVS_GL_CALL( glVertexAttribPointer( m_random_index, 2, GL_UNSIGNED_SHORT, GL_FALSE, 0, offset_indices ) );
            
            // Enable texture random index.
            KVS_GL_CALL( glEnableVertexAttribArray( m_texture_2d_coord_index ) );
            KVS_GL_CALL( glVertexAttribPointer( m_texture_2d_coord_index, 2, GL_FLOAT, GL_FALSE, 0, offset_texture_2d_coords ) );
            
            // Draw triangles.
            //if ( m_has_connection )
            //{
            //    kvs::IndexBufferObject::Binder bind4( m_ibo );
            //    KVS_GL_CALL( glDrawElements( GL_TRIANGLES, 3 * npolygons, GL_UNSIGNED_INT, 0 ) );
            //}
            //else
            {
                KVS_GL_CALL( glDrawArrays( GL_TRIANGLES, 0, 3 * npolygons ) );
            }
            
            // Disable coords.
            KVS_GL_CALL( glDisableClientState( GL_VERTEX_ARRAY ) );
            
            // Disable colors.
            //KVS_GL_CALL( glDisableClientState( GL_COLOR_ARRAY ) );
            
            // Disable normals.
            if ( m_has_normal )
            {
                KVS_GL_CALL( glDisableClientState( GL_NORMAL_ARRAY ) );
            }
            
            // Disable random index.
            KVS_GL_CALL( glDisableVertexAttribArray( m_random_index ) );
            
            KVS_GL_CALL( glDisableVertexAttribArray( m_texture_2d_coord_index ) );
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Creates shader program.
 */
/*===========================================================================*/
void StochasticTexturedPolygonRenderer::Engine::create_shader_program()
{
#ifdef ENABLE_TIME_MEASURE
    const std::string name = object()->name();
    std::string label_index = "0";
    if(name == OBJ_NAME_BOUNDING_BOX_SCALE_LABELS){
        label_index = "5";
    } else if(name == OBJ_NAME_CGMODEL){
        label_index = "6";
    }
    const std::string label_11 = "(A_1_2_4_" + label_index + "_6_2_1)";
    const std::string label_21 = "(A_2_2_4_" + label_index + "_6_2_1)";
    const std::string label_12 = "(A_1_2_4_" + label_index + "_6_2_2)";
    const std::string label_22 = "(A_2_2_4_" + label_index + "_6_2_2)";
    const std::string label_13 = "(A_1_2_4_" + label_index + "_6_2_3)";
    const std::string label_23 = "(A_2_2_4_" + label_index + "_6_2_3)";
    const std::string label_14 = "(A_1_2_4_" + label_index + "_6_2_4)";
    const std::string label_24 = "(A_2_2_4_" + label_index + "_6_2_4)";
    const std::string label_15 = "(A_1_2_4_" + label_index + "_6_2_5)";
    const std::string label_25 = "(A_2_2_4_" + label_index + "_6_2_5)";
    const std::string label_16 = "(A_1_2_4_" + label_index + "_6_2_6)";
    const std::string label_26 = "(A_2_2_4_" + label_index + "_6_2_6)";
    MAKE_AND_START_TIMER(A_XXX_1);
#endif
    kvs::ShaderSource::AddSearchPath(std::getenv("CGFORMAT_EXT4KVS_SHADER_DIR"));
#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(A_XXX_1, label_11 + "  " + name + " AddSearchPath", label_21 + "  " + name + " AddSearchPath");
#endif

#ifdef ENABLE_TIME_MEASURE
    MAKE_AND_START_TIMER(A_XXX_2);
#endif
#ifdef ENABLE_LOAD_SHADER_STATIC
    static kvs::ShaderSource vert( "SR_textured_polygon.vert" );
    static kvs::ShaderSource frag( "SR_textured_polygon.frag" );
#else
    kvs::ShaderSource vert( "SR_textured_polygon.vert" );
    kvs::ShaderSource frag( "SR_textured_polygon.frag" );
#endif
#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(A_XXX_2, label_12 + "  " + name + " read_shader_source", label_22 + "  " + name + " read_shader_source");
#endif

#ifdef ENABLE_TIME_MEASURE
    MAKE_AND_START_TIMER(A_XXX_3);
#endif
    if ( isEnabledShading() )
    {
        switch ( shader().type() )
        {
        case kvs::Shader::LambertShading: frag.define("ENABLE_LAMBERT_SHADING"); break;
        case kvs::Shader::PhongShading: frag.define("ENABLE_PHONG_SHADING"); break;
        case kvs::Shader::BlinnPhongShading: frag.define("ENABLE_BLINN_PHONG_SHADING"); break;
        default: break; // NO SHADING
        }
        
        if ( kvs::OpenGL::Boolean( GL_LIGHT_MODEL_TWO_SIDE ) == GL_TRUE )
        {
            frag.define("ENABLE_TWO_SIDE_LIGHTING");
        }
    }
    
#ifdef DEBUG_TEXTURED_POLYGON
    std::cerr << "StochasticTexturedPolygonRenderer::Engine::create_shader_program() :build shaders." << std::endl;
#endif // DEBUG_TEXTURED_POLYGON
#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(A_XXX_3, label_13 + "  " + name + " frag.define()", label_23 + "  " + name + " frag.define()");
#endif

#ifdef ENABLE_TIME_MEASURE
    MAKE_AND_START_TIMER(A_XXX_4);
#endif
#ifdef ENABLE_LOAD_SHADER_STATIC
    bool isLinked = m_shader_program.isLinked();
#else
    bool isLinked = false;
#endif
#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(A_XXX_4, label_14 + "  " + name + " m_shader_program.isLinked()", label_24 + "  " + name + " m_shader_program.isLinked()");
#endif

#ifdef ENABLE_TIME_MEASURE
    MAKE_AND_START_TIMER(A_XXX_5);
#endif
    if(!isLinked){
        //std::cout<< "m_shader_program.build for StochasticTexturedPolygonRendererForMeasure" << std::endl;
        m_shader_program.build( vert, frag );
    }
#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(A_XXX_5, label_15 + "  " + name + " m_shader_program.build()", label_25 + "  " + name + " m_shader_program.build()");
#endif

#ifdef ENABLE_TIME_MEASURE
    MAKE_AND_START_TIMER(A_XXX_6);
#endif
    if(!isLinked){
        m_shader_program.bind();
        m_shader_program.setUniform( "shading.Ka", shader().Ka );
        m_shader_program.setUniform( "shading.Kd", shader().Kd );
        m_shader_program.setUniform( "shading.Ks", shader().Ks );
        m_shader_program.setUniform( "shading.S",  shader().S );
        {
            m_random_index = m_shader_program.attributeLocation("random_index");
            m_texture_2d_coord_index = m_shader_program.attributeLocation("texture_2d_coord");
#ifdef DEBUG_TEXTURED_POLYGON
            std::cout << "# m_random_index=" << m_random_index << ", m_texture_2d_coord_index=" << m_texture_2d_coord_index << std::endl;
#endif // DEBUG_TEXTURED_POLYGON
        }
        m_shader_program.unbind();
    }
#ifdef ENABLE_TIME_MEASURE
    STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(A_XXX_6, label_16 + "  " + name + " m_shader_program.bind()", label_26 + "  " + name + " m_shader_program.bind()");
#endif
}

/*===========================================================================*/
/**
 *  @brief  Create buffer objects.
 *  @param  texturedPolygon [in] pointer to the textured polygon object
 */
/*===========================================================================*/
void StochasticTexturedPolygonRenderer::Engine::create_buffer_object( const kvs::jaea::TexturedPolygonObject* texturedPolygon )
{
    if ( texturedPolygon->polygonType() != kvs::jaea::TexturedPolygonObject::Triangle )
    {
        kvsMessageError("Not supported textured polygon type.");
        return;
    }
    
    std::map<kvs::UInt32, kvs::UInt32> m_map_id_to_image_width = texturedPolygon->mapIdToImageWidth();
    
    size_t byte_size = 0;
    for(auto itr = m_map_id_to_image_width.begin(); itr != m_map_id_to_image_width.end(); itr++){
        kvs::UInt32 texture_id = itr->first;
        const size_t nvertices = ::NumberOfVertices( texturedPolygon, texture_id );
        m_map_num_vertices[texture_id] = nvertices;
#ifdef DEBUG_TEXTURED_POLYGON
        std::cout << "# id=" << texture_id << ", nvertices=" << nvertices << std::endl;
#endif // DEBUG_TEXTURED_POLYGON
        
        const size_t index_size = nvertices * 2 * sizeof( kvs::UInt16 );
        const size_t coord_size = nvertices * 3 * sizeof( kvs::Real32 );
        //const size_t color_size = nvertices * 4 * sizeof( kvs::UInt8 );
        const size_t normal_size = nvertices * 3 * sizeof( kvs::Real32 );
        const size_t texture_2d_coord_size = nvertices * 2 * sizeof( kvs::Real32 );
        
        //byte_size += index_size + coord_size + color_size + normal_size + texture_2d_coord_size;
        byte_size += index_size + coord_size + normal_size + texture_2d_coord_size;
        
        if( nvertices == 0 ){
#ifdef DEBUG_TEXTURED_POLYGON
            std::cerr << "!!! no vertices : texture_id=" << texture_id << " !!!" << std::endl;
#endif // DEBUG_TEXTURED_POLYGON
            continue;
        }
    }
#ifdef DEBUG
    std::cerr << "# byte_size=" << byte_size << std::endl;
#endif
    
    m_vbo.create( byte_size );
    m_vbo.bind();
    size_t offset = 0;
    for(auto itr = m_map_id_to_image_width.begin(); itr != m_map_id_to_image_width.end(); itr++){
        kvs::UInt32 texture_id = itr->first;
        
        const size_t nvertices = m_map_num_vertices[texture_id];
        if( nvertices == 0 ){
            continue;
        }
        
        kvs::ValueArray<kvs::UInt16> indices( nvertices * 2 );
        for ( size_t i = 0; i < nvertices; i++ )
        {
            const unsigned int count = i * 12347;
            indices[ 2 * i + 0 ] = static_cast<kvs::UInt16>( ( count ) % randomTextureSize() );
            indices[ 2 * i + 1 ] = static_cast<kvs::UInt16>( ( count / randomTextureSize() ) % randomTextureSize() );
        }
        kvs::ValueArray<kvs::Real32> coords = ::VertexCoords( texturedPolygon, texture_id, nvertices );
        //kvs::ValueArray<kvs::UInt8> colors = ::VertexColors( texturedPolygon, texture_id, nvertices );
        kvs::ValueArray<kvs::Real32> normals = ::VertexNormals( texturedPolygon, texture_id, nvertices );
        kvs::ValueArray<kvs::Real32> texture_2d_coords = ::VertexTexture2DCoords( texturedPolygon, texture_id, nvertices );
        
        const size_t index_size = indices.byteSize();
        const size_t coord_size = coords.byteSize();
        //const size_t color_size = colors.byteSize();
        const size_t normal_size = normals.byteSize();
        const size_t texture_2d_coord_size = texture_2d_coords.byteSize();
        size_t offset_indices           = offset;
        size_t offset_coords            = offset_indices + index_size;
        //size_t offset_colors            = offset_coords + coord_size;
        size_t offset_normals           = offset_coords + coord_size;
        size_t offset_texture_2d_coords = offset_normals + normal_size;
        //offset += index_size + coord_size + color_size + normal_size + texture_2d_coord_size;
        offset += index_size + coord_size + normal_size + texture_2d_coord_size;
        
        m_vbo.load( index_size, indices.data(), offset_indices );
        m_vbo.load( coord_size, coords.data(), offset_coords );
        //m_vbo.load( color_size, colors.data(), offset_colors );
        if ( normal_size > 0 )
        {
            m_vbo.load( normal_size, normals.data(), offset_normals );
        }
        m_vbo.load( texture_2d_coord_size, texture_2d_coords.data(), offset_texture_2d_coords );
    }
    
    m_vbo.unbind();
}

} // end of namespace jaea
} // end of namespace kvs
