#include "ParticleTransferProtocol.h"
#include "Serializer.h"

#include <kvs/Camera>
#include <kvs/TransferFunction>
//#include <list> 
#include <cstring>
//4 debug
#include <typeinfo>

// リトルエンディアン環境では JKVS
// ビッグエンディアン環境では SVKJ
const int32_t jpv::ParticleTransferUtils::magicNumber = 0x53564b4a; // 1398164298;

bool jpv::ParticleTransferUtils::isLittleEndian( void )
{
    char buf[4];
    std::memcpy( buf, reinterpret_cast<const char*>( &jpv::ParticleTransferUtils::magicNumber ), sizeof( int32_t ) );
    if ( buf[0] == 'J' )
    {
        return true;
    }
    else
    {
        return false;
    }
}

jpv::ParticleTransferClientMessage::ParticleTransferClientMessage( void )
    : camera( NULL ), transfunc( NULL )
{
    std::fill( header, header + sizeof( header ), '\0' );
/*
    std::fill( opacity_func.exp_token, opacity_func.exp_token+sizeof(opacity_func.exp_token),0 );
    std::fill( opacity_func.var_name, opacity_func.var_name+sizeof(opacity_func.var_name),0 );
    std::fill( opacity_func.value_array, opacity_func.value_array+sizeof(opacity_func.value_array),0);

    std::fill( color_func.exp_token, color_func.exp_token+sizeof(color_func.exp_token),0 );
    std::fill( color_func.var_name, color_func.var_name+sizeof(color_func.var_name),0 );
    std::fill( color_func.value_array, color_func.value_array+sizeof(color_func.value_array),0);
*/
}

int32_t jpv::ParticleTransferClientMessage::byteSize( void ) const
{
    int32_t s = 0;
    s += sizeof( header );
    s += sizeof( messageSize );
    s += sizeof( initParam );
    if ( initParam == -3 )
    {
        s += sizeof( int64_t );
        s += sizeof( char ) * ( inputDir.size() + 1 );
    }
    if ( initParam == 1 )
    {
        s += sizeof( samplingMethod );
        s += sizeof( subPixelLevel );
        s += sizeof( repeatLevel );
        s += sizeof( shuffleMethod );
        s += sizeof( nodeType );
        s += sizeof( renderingId );
        s += sizeof( samplingStep );
        s += sizeof( enable_crop_region );
        s += sizeof( crop_region[0] ) * 6;
        s += sizeof( particle_limit );
        s += sizeof( particle_density );
        s += sizeof( particle_data_size_limit ); //add by shimomura 0308
        s += sizeof( int64_t );
        s += sizeof( char ) * ( inputDir.size() + 1 );
        s += sizeof( size_t );
        s += sizeof( char ) * ( m_x_synthesis.size() + 1 );
        s += sizeof( size_t );
        s += sizeof( char ) * ( m_y_synthesis.size() + 1 );
        s += sizeof( size_t );
        s += sizeof( char ) * ( m_z_synthesis.size() + 1 );
        s += jpv::Serializer::byteSize<kvs::Camera>( *camera );
    }
    if ( initParam == 1 || initParam == -3 )
    {
        s += jpv::Serializer::byteSize( transfunc.size() );
        for ( size_t i = 0; i < transfunc.size(); i++ )
        {
            s += jpv::Serializer::byteSize( transfunc[i].Name );
            s += jpv::Serializer::byteSize( transfunc[i].ColorVar );
            s += jpv::Serializer::byteSize( transfunc[i].ColorVarMin );
            s += jpv::Serializer::byteSize( transfunc[i].ColorVarMax );
            s += jpv::Serializer::byteSize( transfunc[i].OpacityVar );
            s += jpv::Serializer::byteSize( transfunc[i].OpacityVarMin );
            s += jpv::Serializer::byteSize( transfunc[i].OpacityVarMax );
            s += jpv::Serializer::byteSize( transfunc[i].selection );
            if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
            {
                s += jpv::Serializer::byteSize( transfunc[i].Resolution );
                s += jpv::Serializer::byteSize( transfunc[i].EquationR );
                s += jpv::Serializer::byteSize( transfunc[i].EquationG );
                s += jpv::Serializer::byteSize( transfunc[i].EquationB );
                s += jpv::Serializer::byteSize( transfunc[i].EquationA );
            }
            else if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectTransferFunction )
            {
                //デフォルトでこちらの分岐に入る
                //Serializer内部で伝達関数のテーブルサイズと最大最小値のサイズを計算
                s += jpv::Serializer::byteSize<kvs::TransferFunction>( transfunc[i] );
            }
        }
        s += jpv::Serializer::byteSize( voleqn.size() );
        for ( size_t i = 0; i < voleqn.size(); i++ )
        {
            s += jpv::Serializer::byteSize( voleqn[i].Name );
            s += jpv::Serializer::byteSize( voleqn[i].Equation );
        }
        //s += jpv::Serializer::byteSize( transferFunctionSynthesis );
        s += jpv::Serializer::byteSize( color_tf_synthesis );
        s += jpv::Serializer::byteSize( opacity_tf_synthesis );

        //2019 kawamura
        s +=   sizeof(int)*128; //opacity_func
        s +=   sizeof(int)*128; //opacity_func
        s +=   sizeof(float)*128; //opacity_func

        s +=   sizeof(int)*128; //color_func
        s +=   sizeof(int)*128; //color_func
        s +=   sizeof(float)*128; //color_func


        //2019 kawamura
        s += jpv::Serializer::byteSize( opacity_var.size() );
        for( size_t i = 0; i < opacity_var.size(); i++)
        {
            s += sizeof(int)*128;//exp_token
            s += sizeof(int)*128;//var_name
            s += sizeof(float)*128;//value_array
        }

        //2019 kawamura
        s += jpv::Serializer::byteSize( color_var.size() );
        for( size_t i = 0; i < color_var.size(); i++ )
        {
            s += sizeof(int)*128;//exp_token
            s += sizeof(int)*128;//var_name
            s += sizeof(float)*128;//value_array
        }

        //2023 shimomura
        s +=   sizeof(int)*128; //x_synthesis
        s +=   sizeof(int)*128; //x_synthesis
        s +=   sizeof(float)*128; //x_synthesis

        s +=   sizeof(int)*128; //y_synthesis
        s +=   sizeof(int)*128; //y_synthesis
        s +=   sizeof(float)*128; //y_synthesis

        s +=   sizeof(int)*128; //z_synthesis
        s +=   sizeof(int)*128; //z_synthesis
        s +=   sizeof(float)*128; //z_synthesis
    }
    if ( initParam >= 0 )
    {
        s += sizeof( timeParam );
        if ( timeParam == 0 )
        {
            s += sizeof( memorySize );
        }
        else if ( timeParam == 1 )
        {
            s += sizeof( beginTime );
            s += sizeof( endTime );
            s += sizeof( memorySize );
        }
        else if ( timeParam == 2 )
        {
            s += sizeof( step );
        }
        s += sizeof( transParam );
        if ( transParam == 1 )
        {
            s += sizeof( levelIndex );
        }
    }
    return s;
}

size_t jpv::ParticleTransferClientMessage::pack( char* buf ) const
{
    size_t index = 0;
    index += jpv::Serializer::writeArray( buf + index, header );
    index += jpv::Serializer::write( buf + index, messageSize );
    index += jpv::Serializer::write( buf + index, initParam );
    if ( initParam == -3 )
    {
        index += jpv::Serializer::write( buf + index, inputDir.size() );
        index += jpv::Serializer::writeArray( buf + index, inputDir.c_str(), inputDir.size() + 1 );
    }
    if ( initParam == 1 )
    {
        index += jpv::Serializer::write( buf + index, samplingMethod );
        index += jpv::Serializer::write( buf + index, subPixelLevel );
        index += jpv::Serializer::write( buf + index, repeatLevel );
        index += jpv::Serializer::write( buf + index, nodeType );
        index += jpv::Serializer::write( buf + index, renderingId );
        index += jpv::Serializer::write( buf + index, samplingStep );
        index += jpv::Serializer::write( buf + index, shuffleMethod );
        index += jpv::Serializer::write( buf + index, enable_crop_region );
        for ( int i = 0; i < 6; i++ )
        {
            index += jpv::Serializer::write( buf + index, crop_region[i] );
        }
        index += jpv::Serializer::write( buf + index, particle_limit );
        index += jpv::Serializer::write( buf + index, particle_density );
        index += jpv::Serializer::write( buf + index, particle_data_size_limit );
        index += jpv::Serializer::write( buf + index, inputDir.size() );
        index += jpv::Serializer::writeArray( buf + index, inputDir.c_str(), inputDir.size() + 1 );
        index += jpv::Serializer::write( buf + index, m_x_synthesis.size() );
        index += jpv::Serializer::writeArray( buf + index, m_x_synthesis.c_str(), m_x_synthesis.size() + 1 );
        index += jpv::Serializer::write( buf + index, m_y_synthesis.size() );
        index += jpv::Serializer::writeArray( buf + index, m_y_synthesis.c_str(), m_y_synthesis.size() + 1 );
        index += jpv::Serializer::write( buf + index, m_z_synthesis.size() );
        index += jpv::Serializer::writeArray( buf + index, m_z_synthesis.c_str(), m_z_synthesis.size() + 1 );
        index += jpv::Serializer::pack( buf + index, *camera );
    }
    if ( initParam == 1 || initParam == -3 )
    {
        index += jpv::Serializer::write( buf + index, transfunc.size() );
        for ( size_t i = 0; i < transfunc.size(); i++ )
        {
            index += jpv::Serializer::write( buf + index, transfunc[i].Name );
            index += jpv::Serializer::write( buf + index, transfunc[i].ColorVar );
            index += jpv::Serializer::write( buf + index, transfunc[i].ColorVarMin );
            index += jpv::Serializer::write( buf + index, transfunc[i].ColorVarMax );
            index += jpv::Serializer::write( buf + index, transfunc[i].OpacityVar );
            index += jpv::Serializer::write( buf + index, transfunc[i].OpacityVarMin );
            index += jpv::Serializer::write( buf + index, transfunc[i].OpacityVarMax );
            index += jpv::Serializer::write( buf + index, transfunc[i].selection );
            if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
            {
                index += jpv::Serializer::write( buf + index, transfunc[i].Resolution );
                index += jpv::Serializer::write( buf + index, transfunc[i].EquationR );
                index += jpv::Serializer::write( buf + index, transfunc[i].EquationG );
                index += jpv::Serializer::write( buf + index, transfunc[i].EquationB );
                index += jpv::Serializer::write( buf + index, transfunc[i].EquationA );
            }
            else if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectTransferFunction )
            {
                //デフォルトでこちらの分岐に入る
                //Serializer内部で伝達関数のテーブルサイズと最大最小値をパック
                index += jpv::Serializer::pack<kvs::TransferFunction>( buf + index, transfunc[i] );
            }
        }
        index += jpv::Serializer::write( buf + index, voleqn.size() );
        for ( size_t i = 0; i < voleqn.size(); i++ )
        {
            index += jpv::Serializer::write( buf + index, voleqn[i].Name );
            index += jpv::Serializer::write( buf + index, voleqn[i].Equation );
        }
        //index += jpv::Serializer::write( buf + index, transferFunctionSynthesis );
        index += jpv::Serializer::write( buf + index, color_tf_synthesis );
        index += jpv::Serializer::write( buf + index, opacity_tf_synthesis );

        //2019 kawamura
        index +=  jpv::Serializer::writeArray( buf + index, opacity_func.exp_token );
        index +=  jpv::Serializer::writeArray( buf + index, opacity_func.var_name );
        index +=  jpv::Serializer::writeArray( buf + index, opacity_func.value_array );

        index +=  jpv::Serializer::writeArray( buf + index, color_func.exp_token );
        index +=  jpv::Serializer::writeArray( buf + index, color_func.var_name );
        index +=  jpv::Serializer::writeArray( buf + index, color_func.value_array );


        //2019 kawamura
        index +=  jpv::Serializer::write( buf + index, opacity_var.size() );
        for( size_t i = 0; i < opacity_var.size(); i++ )
        {
            index +=  jpv::Serializer::writeArray( buf + index, opacity_var[i].exp_token );
            index +=  jpv::Serializer::writeArray( buf + index, opacity_var[i].var_name );
            index +=  jpv::Serializer::writeArray( buf + index, opacity_var[i].value_array );
        }

        //2019 kawamura
        index +=  jpv::Serializer::write( buf + index, color_var.size() );
        for( size_t i = 0; i < color_var.size(); i++ )
        {
            index +=  jpv::Serializer::writeArray( buf + index, color_var[i].exp_token );
            index +=  jpv::Serializer::writeArray( buf + index, color_var[i].var_name );
            index +=  jpv::Serializer::writeArray( buf + index, color_var[i].value_array );
        }
        //2023 shimomura
        index +=  jpv::Serializer::writeArray( buf + index, x_synthesis_token.exp_token );
        index +=  jpv::Serializer::writeArray( buf + index, x_synthesis_token.var_name );
        index +=  jpv::Serializer::writeArray( buf + index, x_synthesis_token.value_array );

        index +=  jpv::Serializer::writeArray( buf + index, y_synthesis_token.exp_token );
        index +=  jpv::Serializer::writeArray( buf + index, y_synthesis_token.var_name );
        index +=  jpv::Serializer::writeArray( buf + index, y_synthesis_token.value_array );

        index +=  jpv::Serializer::writeArray( buf + index, z_synthesis_token.exp_token );
        index +=  jpv::Serializer::writeArray( buf + index, z_synthesis_token.var_name );
        index +=  jpv::Serializer::writeArray( buf + index, z_synthesis_token.value_array );

    }
    if ( initParam >= 0 )
    {
        index += jpv::Serializer::write( buf + index, timeParam );
        if ( timeParam == 0 )
        {
            index += jpv::Serializer::write( buf + index, memorySize );
        }
        else if ( timeParam == 1 )
        {
            index += jpv::Serializer::write( buf + index, beginTime );
            index += jpv::Serializer::write( buf + index, endTime );
            index += jpv::Serializer::write( buf + index, memorySize );
        }
        else if ( timeParam == 2 )
        {
            index += jpv::Serializer::write( buf + index, step );
        }
        index += jpv::Serializer::write( buf + index, transParam );
        if ( transParam == 1 )
        {
            index += jpv::Serializer::write( buf + index, levelIndex );
        }
    }
    return index;
}

size_t jpv::ParticleTransferClientMessage::unpack( const char* buf )
{
    size_t index = 0;
    int64_t tmp_char_size;
    char*  tmp_char = NULL;
    int64_t tmp_size[6];
    char*  tmp_char_array[6];

    index += jpv::Serializer::readArray( buf + index, header );
    if ( strncmp( "JPTP /1.0", header, 9 ) != 0 )
    {
        return 0;
    }
    index += jpv::Serializer::read( buf + index, messageSize );
    index += jpv::Serializer::read( buf + index, initParam );
    if ( initParam == -3 )
    {
        index += jpv::Serializer::read( buf + index, tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        inputDir = std::string( tmp_char );
        delete[] tmp_char;
    }
    if ( initParam == 1 )
    {
        index += jpv::Serializer::read( buf + index, samplingMethod );
        index += jpv::Serializer::read( buf + index, subPixelLevel );
        index += jpv::Serializer::read( buf + index, repeatLevel );
        index += jpv::Serializer::read( buf + index, nodeType );
        index += jpv::Serializer::read( buf + index, renderingId );
        index += jpv::Serializer::read( buf + index, samplingStep );
        index += jpv::Serializer::read( buf + index, shuffleMethod );
        index += jpv::Serializer::read( buf + index, enable_crop_region );
        for ( int i = 0; i < 6; i++ )
        {
            index += jpv::Serializer::read( buf + index, crop_region[i] );
        }
        index += jpv::Serializer::read( buf + index, particle_limit );
        index += jpv::Serializer::read( buf + index, particle_density );
        index += jpv::Serializer::read( buf + index, particle_data_size_limit );
        index += jpv::Serializer::read( buf + index, tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        inputDir = std::string( tmp_char );
        delete[] tmp_char;

        // X-Z coodrinate Synthesis
        index += jpv::Serializer::read( buf + index, tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_x_synthesis = std::string( tmp_char );
        delete[] tmp_char;
        index += jpv::Serializer::read( buf + index, tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_y_synthesis = std::string( tmp_char );
        delete[] tmp_char;
        index += jpv::Serializer::read( buf + index, tmp_char_size );
        tmp_char = new char[tmp_char_size + 1];
        index += jpv::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
        m_z_synthesis = std::string( tmp_char );
        delete[] tmp_char;
//        std::cout << "Receive X Syth = " << m_x_synthesis << std::endl;
//        std::cout << "Receive Y Syth = " << m_y_synthesis << std::endl;
//        std::cout << "Receive Z Syth = " << m_z_synthesis << std::endl;

        index += jpv::Serializer::unpack( buf + index, *camera );
    }
    if ( initParam == 1 || initParam == -3 )
    {
        size_t s;
        index += jpv::Serializer::read( buf + index, s );
        transfunc.clear();
        for ( size_t i = 0; i < s; i++ )
        {
            transfunc.push_back( NamedTransferFunctionParameter() );
            index += jpv::Serializer::read( buf + index, transfunc[i].Name );
            index += jpv::Serializer::read( buf + index, transfunc[i].ColorVar );
            index += jpv::Serializer::read( buf + index, transfunc[i].ColorVarMin );
            index += jpv::Serializer::read( buf + index, transfunc[i].ColorVarMax );
            index += jpv::Serializer::read( buf + index, transfunc[i].OpacityVar );
            index += jpv::Serializer::read( buf + index, transfunc[i].OpacityVarMin );
            index += jpv::Serializer::read( buf + index, transfunc[i].OpacityVarMax );
            index += jpv::Serializer::read( buf + index, transfunc[i].selection );
            if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
            {
                index += jpv::Serializer::read( buf + index, transfunc[i].Resolution );
                index += jpv::Serializer::read( buf + index, transfunc[i].EquationR );
                index += jpv::Serializer::read( buf + index, transfunc[i].EquationG );
                index += jpv::Serializer::read( buf + index, transfunc[i].EquationB );
                index += jpv::Serializer::read( buf + index, transfunc[i].EquationA );
            }
            else if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectTransferFunction )
            {
                //デフォルトでこちらの分岐に入る
                //Serializer内部で伝達関数のテーブルサイズと最大最小値をアンパック
                index += jpv::Serializer::unpack<kvs::TransferFunction>( buf + index, transfunc[i] );
            }
        }
        index += jpv::Serializer::read( buf + index, s );
        voleqn.clear();
        for ( size_t i = 0; i < s; i++ )
        {
            voleqn.push_back( VolumeEquation() );
            index += jpv::Serializer::read( buf + index, voleqn[i].Name );
            index += jpv::Serializer::read( buf + index, voleqn[i].Equation );
        }
        //index += jpv::Serializer::read( buf + index, transferFunctionSynthesis );
        index += jpv::Serializer::read( buf + index, color_tf_synthesis );
        index += jpv::Serializer::read( buf + index, opacity_tf_synthesis );

        //2019 kawamura
        index +=  jpv::Serializer::readArray( buf + index, opacity_func.exp_token, 128 );
        index +=  jpv::Serializer::readArray( buf + index, opacity_func.var_name, 128 );
        index +=  jpv::Serializer::readArray( buf + index, opacity_func.value_array, 128 );

        index +=  jpv::Serializer::readArray( buf + index, color_func.exp_token, 128 );
        index +=  jpv::Serializer::readArray( buf + index, color_func.var_name, 128 );
        index +=  jpv::Serializer::readArray( buf + index, color_func.value_array, 128 );

        //2019 kawamura
        EquationToken tmp_token;
        size_t size;
        index +=  jpv::Serializer::read( buf + index, size );
        opacity_var.clear();
        for( size_t i = 0; i < size; i++ )
        {
            index +=  jpv::Serializer::readArray( buf + index, tmp_token.exp_token, 128 );
            index +=  jpv::Serializer::readArray( buf + index, tmp_token.var_name, 128 );
            index +=  jpv::Serializer::readArray( buf + index, tmp_token.value_array, 128 );
            opacity_var.push_back( tmp_token );
        }

        //2019 kawamura
        index +=  jpv::Serializer::read( buf + index, size );
        color_var.clear();
        for( size_t i = 0; i < size; i++ )
        {
            index +=  jpv::Serializer::readArray( buf + index, tmp_token.exp_token, 128 );
            index +=  jpv::Serializer::readArray( buf + index, tmp_token.var_name, 128 );
            index +=  jpv::Serializer::readArray( buf + index, tmp_token.value_array, 128 );
            color_var.push_back( tmp_token );
        }
    
        //2023 shimomura
        index +=  jpv::Serializer::readArray( buf + index, x_synthesis_token.exp_token, 128 );
        index +=  jpv::Serializer::readArray( buf + index, x_synthesis_token.var_name, 128 );
        index +=  jpv::Serializer::readArray( buf + index, x_synthesis_token.value_array, 128 );

        index +=  jpv::Serializer::readArray( buf + index, y_synthesis_token.exp_token, 128 );
        index +=  jpv::Serializer::readArray( buf + index, y_synthesis_token.var_name, 128 );
        index +=  jpv::Serializer::readArray( buf + index, y_synthesis_token.value_array, 128 );

        index +=  jpv::Serializer::readArray( buf + index, z_synthesis_token.exp_token, 128 );
        index +=  jpv::Serializer::readArray( buf + index, z_synthesis_token.var_name, 128 );
        index +=  jpv::Serializer::readArray( buf + index, z_synthesis_token.value_array, 128 );    
    
    }
    if ( initParam >= 0 )
    {
        index += jpv::Serializer::read( buf + index, timeParam );
        if ( timeParam == 0 )
        {
            index += jpv::Serializer::read( buf + index, memorySize );
        }
        else if ( timeParam == 1 )
        {
            index += jpv::Serializer::read( buf + index, beginTime );
            index += jpv::Serializer::read( buf + index, endTime );
            index += jpv::Serializer::read( buf + index, memorySize );
        }
        else if ( timeParam == 2 )
        {
            index += jpv::Serializer::read( buf + index, step );
        }
        index += jpv::Serializer::read( buf + index, transParam );
        if ( transParam == 1 )
        {
            index += jpv::Serializer::read( buf + index, levelIndex );
        }
    }

    return index;
}

//2019 kawamura
void jpv::ParticleTransferClientMessage::show( void ) const
{
    std::cout<<"SHOW CLIENT MESSAGE"<<std::endl;

    std::cout<<"header="<<header<<std::endl;
    std::cout<<"messageSize="<<messageSize<<std::endl;
    std::cout<<"initParam="<<initParam<<std::endl;
    std::cout<<"timeParam="<<timeParam<<std::endl;
    std::cout<<"transParam="<<transParam<<std::endl;

    std::cout<<"transfunc.size="<<transfunc.size()<<std::endl;
    std::cout<<"transfunc.Name,ColorVar,OpacityVar,ColorVarMin,ColorVarMax"<<std::endl;
    for(int i=0; i<transfunc.size(); i++)
    {
        std::cout<<transfunc[i].Name<<","<<transfunc[i].ColorVar<<","<<transfunc[i].OpacityVar<<","<<transfunc[i].ColorVarMin<<","<<transfunc[i].ColorVarMax<<std::endl;
    }

    //for(int i=0; i<transfunc.size(); i++) std::cout << "m_transfunc["<< i <<"].colorMap().maxValue() = " << transfunc[i].colorMap().maxValue() << std::endl;
    //for(int i=0; i<256; i++) std::cout << "m_transfunc.OpacityMap().at("<< i <<") = " << transfunc[0].opacityMap()[i] <<std::endl; 
    //for(int i=0; i<256; i++) std::cout << "m_transfunc.OpacityMap().at("<< i <<") = " << transfunc[1].opacityMap()[i] <<std::endl; 
    //for(auto it = transfunc[0].colorMap().points().begin(); it != transfunc[0].colorMap().points().end(); ++it)
    //{ 
    //    std::cout <<  *it.str() << " "; 
    //}
    std::cout<<"voleqn.size="<<voleqn.size()<<std::endl;
    std::cout<<"voleqn.Name,Equation"<<std::endl;
    for(int i=0; i<voleqn.size(); i++)
    {
        std::cout<<voleqn[i].Name<<","<<voleqn[i].Equation<<std::endl;
    }

    //std::cout<<"transferFunctionSynthesis="<<transferFunctionSynthesis<<std::endl;
    std::cout << "color_tf_synthesis=" << color_tf_synthesis << std::endl;
    std::cout << "opacity_tf_synthesis=" << opacity_tf_synthesis << std::endl;

    std::cout << "m_x_synthesis = " << m_x_synthesis << std::endl;
    std::cout << "m_y_synthesis = " << m_y_synthesis << std::endl;
    std::cout << "m_z_synthesis = " << m_z_synthesis << std::endl;

    // show token
    std::cout<<"opacity_func"<<std::endl;
    std::cout<<"exp_token";
    for(int i=0; i<20; i++)
    {
        std::cout<<opacity_func.exp_token[i]<<",";
    }std::cout<<std::endl;
    std::cout<<"var_name";
    for(int i=0; i<20; i++)
    {
        std::cout<<opacity_func.var_name[i]<<",";
    }std::cout<<std::endl;
    std::cout<<"value_array";
    for(int i=0; i<20; i++)
    {
        std::cout<<opacity_func.value_array[i]<<",";
    }std::cout<<std::endl;

    std::cout<<"color_func"<<std::endl;
    std::cout<<"exp_token";
    for(int i=0; i<20; i++)
    {
        std::cout<<color_func.exp_token[i]<<",";
    }std::cout<<std::endl;
    std::cout<<"var_name";
    for(int i=0; i<20; i++)
    {
        std::cout<<color_func.var_name[i]<<",";
    }std::cout<<std::endl;
    std::cout<<"value_array";
    for(int i=0; i<20; i++)
    {
        std::cout<<color_func.value_array[i]<<",";
    }std::cout<<std::endl;

    std::cout<<"opacity_var.size="<<opacity_var.size()<<std::endl;
    for(int j=0; j<opacity_var.size(); j++)
    {
        std::cout<<"exp_token";
        for(int i=0; i<20; i++)
        {
            std::cout<<opacity_var[j].exp_token[i]<<",";
        }std::cout<<std::endl;
        std::cout<<"var_name";
        for(int i=0; i<20; i++)
        {
            std::cout<<opacity_var[j].var_name[i]<<",";
        }std::cout<<std::endl;
        std::cout<<"value_array";
        for(int i=0; i<20; i++)
        {
            std::cout<<opacity_var[j].value_array[i]<<",";
        }std::cout<<std::endl;
    }

    std::cout<<"color_var.size="<<color_var.size()<<std::endl;
    for(int j=0; j<color_var.size(); j++)
    {
        std::cout<<"exp_token";
        for(int i=0; i<20; i++)
        {
            std::cout<<color_var[j].exp_token[i]<<",";
        }std::cout<<std::endl;
        std::cout<<"var_name";
        for(int i=0; i<20; i++)
        {
            std::cout<<color_var[j].var_name[i]<<",";
        }std::cout<<std::endl;
        std::cout<<"value_array";
        for(int i=0; i<20; i++)
        {
            std::cout<<color_var[j].value_array[i]<<",";
        }std::cout<<std::endl;
    }

    std::cout<<std::endl;
}

jpv::ParticleTransferServerMessage::ParticleTransferServerMessage( void ):
    camera( NULL )
{
    std::fill( header, header + sizeof( header ), '\0' );
    m_server_status = 0;
}

int32_t jpv::ParticleTransferServerMessage::byteSize( void ) const
{
    int32_t s = 0;
    s += sizeof( header );
    s += sizeof( messageSize );
    // MODIFIED START FEAST 2015.12.23
    s += sizeof( m_server_status );
    if ( m_server_status == 1 )  
    {
        return s;
    }
    // MODIFIED START FEAST 2015.12.23
    s += sizeof( timeStep );
    s += sizeof( subPixelLevel );
    s += sizeof( repeatLevel );
    s += sizeof( levelIndex );
    s += sizeof( numParticle );
    s += sizeof( numVolDiv );
    s += sizeof( staStep );
    s += sizeof( endStep );
    s += sizeof( numStep );
    s += sizeof( minObjectCoord[0] ) * 3;
    s += sizeof( maxObjectCoord[0] ) * 3;
    s += sizeof( minValue );
    s += sizeof( maxValue );
    s += sizeof( numNodes );
    s += sizeof( numElements );
    s += sizeof( elemType );
    s += sizeof( fileType );
    s += sizeof( numIngredients );
    s += varRange.byteSize();
    s += sizeof( flag_send_bins );
    s += sizeof( particle_limit );
    s += sizeof( particle_density );
    s += sizeof( particle_data_size_limit );
    s += jpv::Serializer::byteSize<kvs::Camera>( *camera );
    if ( flag_send_bins == 1 )
    {   
    s += sizeof( tf_count );
    for ( int i = 0; i < tf_count; i++ )
    {
        s += sizeof( c_nbins[i] );
        s += sizeof( c_bins[i] ) * c_nbins[i];
        s += sizeof( o_nbins[i] );
        s += sizeof( o_bins[i] ) * o_nbins[i];
    }

    s += jpv::Serializer::byteSize( transfunc.size() );
    for ( size_t i = 0; i < transfunc.size(); i++ )
    {
        s += jpv::Serializer::byteSize( transfunc[i].Name );
        s += jpv::Serializer::byteSize( transfunc[i].ColorVar );
        s += jpv::Serializer::byteSize( transfunc[i].ColorVarMin );
        s += jpv::Serializer::byteSize( transfunc[i].ColorVarMax );
        s += jpv::Serializer::byteSize( transfunc[i].OpacityVar );
        s += jpv::Serializer::byteSize( transfunc[i].OpacityVarMin );
        s += jpv::Serializer::byteSize( transfunc[i].OpacityVarMax );
        s += jpv::Serializer::byteSize( transfunc[i].selection );
        if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
        {
            s += jpv::Serializer::byteSize( transfunc[i].Resolution );
            s += jpv::Serializer::byteSize( transfunc[i].EquationR );
            s += jpv::Serializer::byteSize( transfunc[i].EquationG );
            s += jpv::Serializer::byteSize( transfunc[i].EquationB );
            s += jpv::Serializer::byteSize( transfunc[i].EquationA );
        }
        else if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectTransferFunction )
        {
            s += jpv::Serializer::byteSize<kvs::TransferFunction>( transfunc[i] );
        }
    }
    s += jpv::Serializer::byteSize( voleqn.size() );
    for ( size_t i = 0; i < voleqn.size(); i++ )
    {
        s += jpv::Serializer::byteSize( voleqn[i].Name );
        s += jpv::Serializer::byteSize( voleqn[i].Equation );
    }
    //s += jpv::Serializer::byteSize( transferFunctionSynthesis );
    s += jpv::Serializer::byteSize( color_tf_synthesis );
    s += jpv::Serializer::byteSize( opacity_tf_synthesis );
    }

    return s;
}

size_t jpv::ParticleTransferServerMessage::pack( char* buf ) const
{
    size_t index = 0;
    index += jpv::Serializer::writeArray( buf + index, header );
    index += jpv::Serializer::write( buf + index, messageSize );
    // MODIFIED START FEAST 2015.12.23
    index += jpv::Serializer::write( buf + index, m_server_status );
    if ( m_server_status == 1 )
    { 
        return 0;
    }
    // MODIFIED START FEAST 2015.12.23
    index += jpv::Serializer::write( buf + index, timeStep );
    index += jpv::Serializer::write( buf + index, subPixelLevel );
    index += jpv::Serializer::write( buf + index, repeatLevel );
    index += jpv::Serializer::write( buf + index, levelIndex );
    index += jpv::Serializer::write( buf + index, numParticle );
    index += jpv::Serializer::write( buf + index, numVolDiv );
    index += jpv::Serializer::write( buf + index, staStep );
    index += jpv::Serializer::write( buf + index, endStep );
    index += jpv::Serializer::write( buf + index, numStep );
    index += jpv::Serializer::write( buf + index, minObjectCoord[0] );
    index += jpv::Serializer::write( buf + index, minObjectCoord[1] );
    index += jpv::Serializer::write( buf + index, minObjectCoord[2] );
    index += jpv::Serializer::write( buf + index, maxObjectCoord[0] );
    index += jpv::Serializer::write( buf + index, maxObjectCoord[1] );
    index += jpv::Serializer::write( buf + index, maxObjectCoord[2] );
    index += jpv::Serializer::write( buf + index, minValue );
    index += jpv::Serializer::write( buf + index, maxValue );
    index += jpv::Serializer::write( buf + index, numNodes );
    index += jpv::Serializer::write( buf + index, numElements );
    index += jpv::Serializer::write( buf + index, elemType );
    index += jpv::Serializer::write( buf + index, fileType );
    index += jpv::Serializer::write( buf + index, numIngredients );
    index += varRange.pack( buf + index );
    index += jpv::Serializer::write( buf + index, flag_send_bins );
    index += jpv::Serializer::write( buf + index, particle_limit );
    index += jpv::Serializer::write( buf + index, particle_density );
    index += jpv::Serializer::write( buf + index, particle_data_size_limit );
    index += jpv::Serializer::pack( buf + index, *camera );
    if ( flag_send_bins == 1 )
    {   
    index += jpv::Serializer::write( buf + index, tf_count );
    for ( int i = 0; i < tf_count; i++ )
    {
        index += jpv::Serializer::write( buf + index, c_nbins[i] );
        index += jpv::Serializer::writeArray<kvs::UInt64>( buf + index, c_bins[i], c_nbins[i] );
        index += jpv::Serializer::write( buf + index, o_nbins[i] );
        index += jpv::Serializer::writeArray<kvs::UInt64>( buf + index, o_bins[i], o_nbins[i] );
    }

    index += jpv::Serializer::write( buf + index, transfunc.size() );
    for ( size_t i = 0; i < transfunc.size(); i++ )
    {
        index += jpv::Serializer::write( buf + index, transfunc[i].Name );
        index += jpv::Serializer::write( buf + index, transfunc[i].ColorVar );
        index += jpv::Serializer::write( buf + index, transfunc[i].ColorVarMin );
        index += jpv::Serializer::write( buf + index, transfunc[i].ColorVarMax );
        index += jpv::Serializer::write( buf + index, transfunc[i].OpacityVar );
        index += jpv::Serializer::write( buf + index, transfunc[i].OpacityVarMin );
        index += jpv::Serializer::write( buf + index, transfunc[i].OpacityVarMax );
        index += jpv::Serializer::write( buf + index, transfunc[i].selection );
        if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
        {
            index += jpv::Serializer::write( buf + index, transfunc[i].Resolution );
            index += jpv::Serializer::write( buf + index, transfunc[i].EquationR );
            index += jpv::Serializer::write( buf + index, transfunc[i].EquationG );
            index += jpv::Serializer::write( buf + index, transfunc[i].EquationB );
            index += jpv::Serializer::write( buf + index, transfunc[i].EquationA );
        }
        else if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectTransferFunction )
        {
            index += jpv::Serializer::pack<kvs::TransferFunction>( buf + index, transfunc[i] );
        }
    }
    index += jpv::Serializer::write( buf + index, voleqn.size() );
    for ( size_t i = 0; i < voleqn.size(); i++ )
    {
        index += jpv::Serializer::write( buf + index, voleqn[i].Name );
        index += jpv::Serializer::write( buf + index, voleqn[i].Equation );
    }
    //index += jpv::Serializer::write( buf + index, transferFunctionSynthesis );
    index += jpv::Serializer::write( buf + index, color_tf_synthesis );
    index += jpv::Serializer::write( buf + index, opacity_tf_synthesis );
    }
    else// if ( flag_send_bins == 2 )
    {
        index += jpv::Serializer::writeArray<float>( buf + index, positions, 3 * numParticle );
        index += jpv::Serializer::writeArray<float>( buf + index, normals, 3 * numParticle );
        index += jpv::Serializer::writeArray<unsigned char>( buf + index, colors, 3 * numParticle );
    }
    return index;
}

size_t jpv::ParticleTransferServerMessage::unpack_message( const char* buf )
{
    size_t index = 0;
    index += jpv::Serializer::readArray( buf + index, header );
    if ( strncmp( "JPTP /1.0", header, 9 ) != 0 )
    {
        return 0;
    }
    index += jpv::Serializer::read( buf + index, messageSize );
    // MODIFIED START FEAST 2015.12.23
    index += jpv::Serializer::read( buf + index, m_server_status );
    if ( m_server_status == 1 )
    { 
        return 0;
    }
    // MODIFIED START FEAST 2015.12.23
    index += jpv::Serializer::read( buf + index, timeStep );
    index += jpv::Serializer::read( buf + index, subPixelLevel );
    index += jpv::Serializer::read( buf + index, repeatLevel );
    index += jpv::Serializer::read( buf + index, levelIndex );
    index += jpv::Serializer::read( buf + index, numParticle );
    index += jpv::Serializer::read( buf + index, numVolDiv );
    index += jpv::Serializer::read( buf + index, staStep );
    index += jpv::Serializer::read( buf + index, endStep );
    index += jpv::Serializer::read( buf + index, numStep );
    index += jpv::Serializer::read( buf + index, minObjectCoord[0] );
    index += jpv::Serializer::read( buf + index, minObjectCoord[1] );
    index += jpv::Serializer::read( buf + index, minObjectCoord[2] );
    index += jpv::Serializer::read( buf + index, maxObjectCoord[0] );
    index += jpv::Serializer::read( buf + index, maxObjectCoord[1] );
    index += jpv::Serializer::read( buf + index, maxObjectCoord[2] );
    index += jpv::Serializer::read( buf + index, minValue );
    index += jpv::Serializer::read( buf + index, maxValue );
    index += jpv::Serializer::read( buf + index, numNodes );
    index += jpv::Serializer::read( buf + index, numElements );
    index += jpv::Serializer::read( buf + index, elemType );
    index += jpv::Serializer::read( buf + index, fileType );
    index += jpv::Serializer::read( buf + index, numIngredients );
    index += varRange.unpack( buf + index );
    index += jpv::Serializer::read( buf + index, flag_send_bins );
    index += jpv::Serializer::read( buf + index, particle_limit );
    index += jpv::Serializer::read( buf + index, particle_density );
    index += jpv::Serializer::read( buf + index, particle_data_size_limit );
    index += jpv::Serializer::unpack( buf + index, *camera );
    if ( flag_send_bins == 1 )
    {
    index += jpv::Serializer::read( buf + index, tf_count );
    c_nbins = new kvs::UInt64[ tf_count ];
    o_nbins = new kvs::UInt64[ tf_count ];

    c_bins.resize( tf_count );
    o_bins.resize( tf_count );
    for ( int i = 0; i < tf_count; i++ )
    {
        index += jpv::Serializer::read( buf + index, c_nbins[i] );
        c_bins[i] =  new kvs::UInt64[ c_nbins[i] ];
        index += jpv::Serializer::readArray<kvs::UInt64>( buf + index, c_bins[i], c_nbins[i] );
        index += jpv::Serializer::read( buf + index, o_nbins[i] );
        o_bins[i] =  new kvs::UInt64[ o_nbins[i] ];
        index += jpv::Serializer::readArray<kvs::UInt64>( buf + index, o_bins[i], o_nbins[i] );
    }

    size_t s;
    index += jpv::Serializer::read( buf + index, s );
    transfunc.clear();
    for ( size_t i = 0; i < s; i++ )
    {
        transfunc.push_back( NamedTransferFunctionParameter() );
        index += jpv::Serializer::read( buf + index, transfunc[i].Name );
        index += jpv::Serializer::read( buf + index, transfunc[i].ColorVar );
        index += jpv::Serializer::read( buf + index, transfunc[i].ColorVarMin );
        index += jpv::Serializer::read( buf + index, transfunc[i].ColorVarMax );
        index += jpv::Serializer::read( buf + index, transfunc[i].OpacityVar );
        index += jpv::Serializer::read( buf + index, transfunc[i].OpacityVarMin );
        index += jpv::Serializer::read( buf + index, transfunc[i].OpacityVarMax );
        index += jpv::Serializer::read( buf + index, transfunc[i].selection );
        if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectExtendTransferFunction )
        {
            index += jpv::Serializer::read( buf + index, transfunc[i].Resolution );
            index += jpv::Serializer::read( buf + index, transfunc[i].EquationR );
            index += jpv::Serializer::read( buf + index, transfunc[i].EquationG );
            index += jpv::Serializer::read( buf + index, transfunc[i].EquationB );
            index += jpv::Serializer::read( buf + index, transfunc[i].EquationA );
        }
        else if ( transfunc[i].selection == NamedTransferFunctionParameter::SelectTransferFunction )
        {
            index += jpv::Serializer::unpack<kvs::TransferFunction>( buf + index, transfunc[i] );
        }
    }
    index += jpv::Serializer::read( buf + index, s );
    voleqn.clear();
    for ( size_t i = 0; i < s; i++ )
    {
        voleqn.push_back( VolumeEquation() );
        index += jpv::Serializer::read( buf + index, voleqn[i].Name );
        index += jpv::Serializer::read( buf + index, voleqn[i].Equation );
    }
    //index += jpv::Serializer::read( buf + index, transferFunctionSynthesis );
    index += jpv::Serializer::read( buf + index, color_tf_synthesis );
    index += jpv::Serializer::read( buf + index, opacity_tf_synthesis );
    }

    return index;
}

size_t jpv::ParticleTransferServerMessage::unpack_particles( const char* buf )
{
    size_t index = messageSize;
    positions = new float[3 * numParticle];
    normals = new float[3 * numParticle];
    colors = new unsigned char[3 * numParticle];
    index += jpv::Serializer::readArray<float>( buf + index, positions, 3 * numParticle );
    index += jpv::Serializer::readArray<float>( buf + index, normals, 3 * numParticle );
    index += jpv::Serializer::readArray<unsigned char>( buf + index, colors, 3 * numParticle );
    return index;
}

//add by shimomura 2022/12/23 
/**
 * Color Histogram用のデータを設定する。
 * @param histogram_size        Histogram数
 * @param nbins                 Histogramデータ数
 * @param c_bins            Color Histogram用のデータ = histogram_size*nbins
 * @param transfer_function_names   すべての1次伝達関数名
 * @param transfunc_synthesizer_names   Histogram用のデータの1次伝達関数名
 */
void jpv::ParticleTransferServerMessage::setColorHistogramBins(
        int histogram_size,
        int nbins,
        const kvs::UInt64* arg_c_bins)
        //const kvs::UInt64* arg_c_bins,
        //const std::vector<std::string> &transfer_function_names,
        //const std::vector<std::string> &transfunc_synthesizer_names)
{
    for ( int synth_tf = 0; synth_tf < histogram_size; synth_tf++ ) { 
        this->c_nbins[synth_tf] = nbins;
        for ( int res = 0; res < nbins; res++ ) { 
            int n = synth_tf*nbins + res;
            this->c_bins[synth_tf][res] = arg_c_bins[n];
        }                                                                                                                                                   
    }   

    return;
}

/**
 * Opacity Histogram用のデータを設定する。
 * @param histogram_size        Histogram数
 * @param nbins                 Histogramデータ数
 * @param o_bins            Opacity Histogram用のデータ = histogram_size*nbins
 * @param transfer_function_names   すべての1次伝達関数名
 * @param transfunc_synthesizer_names   Histogram用のデータの1次伝達関数名
 */
void jpv::ParticleTransferServerMessage::setOpacityHistogramBins(
        int histogram_size,
        int nbins,
        const kvs::UInt64* arg_o_bins)
        //const kvs::UInt64* arg_o_bins,
        //const std::vector<std::string> &transfer_function_names,
        //const std::vector<std::string> &transfunc_synthesizer_names)
{
    for ( int synth_tf = 0; synth_tf < histogram_size; synth_tf++ ) { 
        this->o_nbins[synth_tf] = nbins;
        for ( int res = 0; res < nbins; res++ ) { 
            int n = synth_tf*nbins + res;
            this->o_bins[synth_tf][res] = arg_o_bins[n];
        }   
    }  
   
    //std::stringstream debug11; 
    //debug11 << "o_bins[1] = {";
    //for(int i =0; i< nbins; i++) debug11 << o_bins[1][i] << "," ;
    //std::cout << debug11.str() << std::endl;

    return;
}

/**
 * 関数の領域確保、初期化を行う
 * @param transfer_function_count       関数数
 * @param nbins     解像度
 */
void jpv::ParticleTransferServerMessage::initializeTransferFunction(
        const int32_t transfer_function_count,
        const int nbins)
{

    this->tf_count = transfer_function_count;

    this->c_nbins = new kvs::UInt64[transfer_function_count];
    this->o_nbins = new kvs::UInt64[transfer_function_count];

    this->c_bins.resize( transfer_function_count );
    this->o_bins.resize( transfer_function_count );
    //this->m_color_bin_names.resize( transfer_function_count );
    //this->m_opacity_bin_names.resize( transfer_function_count );

    for ( int tf = 0; tf < this->tf_count; tf++ )
    {   
        this->c_nbins[tf] = nbins;
        this->o_nbins[tf] = nbins;
        this->c_bins[tf] =  new kvs::UInt64[ this->c_nbins[tf] ];
        this->o_bins[tf] =  new kvs::UInt64[ this->o_nbins[tf] ];
        for ( kvs::UInt64 res = 0; res < this->c_nbins[tf]; res++ )
        {   
            this->c_bins[tf][res] = 0;
        }   
        for ( kvs::UInt64 res = 0; res < this->o_nbins[tf]; res++ )
        {   
            this->o_bins[tf][res] = 0;
        }   
        //this->m_color_bin_names[tf].clear();
        //this->m_opacity_bin_names[tf].clear();
    }   

    return;
}


//2019 kawamura
void jpv::ParticleTransferServerMessage::show( void ) const
{
    std::cout<<"SHOW SERVER MESSAGE"<<std::endl;
    std::cout<<"header="<<header<<std::endl;
    std::cout<<"messageSize="<<messageSize<<std::endl;
    std::cout<<"timeStep="<<timeStep<<std::endl;
    std::cout<<"numParticle="<<numParticle<<std::endl;

    std::cout<<"transfunc.size="<<transfunc.size()<<std::endl;
    //std::cout<<"transfunc.Name,ColorVar,OpacityVar,ColorVarMin,ColorVarMax"<<std::endl;
    //for(int i=0; i<transfunc.size(); i++)
    //{
    //    std::cout<<transfunc[i].Name<<","<<transfunc[i].ColorVar<<","<<transfunc[i].OpacityVar<<","<<transfunc[i].ColorVarMin<<","<<transfunc[i].ColorVarMax<<std::endl;

    //}

    //for(int i=0; i<transfunc.size(); i++) std::cout << "m_transfunc["<< i <<"].colorMap().maxValue() = " << transfunc[i].colorMap().maxValue() << std::endl;
   
    //for(int i=0; i<256; i++) std::cout << "m_transfunc.colorMap().table().at("<< i <<") = " << transfunc[i].colorMap().table().at(i) <<std::endl; 
   
//    std::cout << "c_nbins = " << *c_nbins << std::endl;
//    std::cout << "o_nbins = " << *o_nbins << std::endl;
//
//    //{ 
//    std::cout << "c_bins = {" << std::endl;
//    for(int i=0; i < 256; i++) std::cout << c_bins[0][i] << " " ; 
//    //}
//
//    std::string ss = "t1_var_o"; 
//    std::cout << "varRange.max(t1_var_o) = " << varRange.max(ss) <<std::endl;
//    std::cout << "varRange.min(t1_var_o) = " << varRange.min(ss) <<std::endl;
//    ss = "t1_var_c";
//    std::cout << "varRange.max(t1_var_c) = " << varRange.max(ss) <<std::endl;
//    std::cout << "varRange.min(t1_var_c) = " << varRange.min(ss) <<std::endl;
//
//    std::cout<<"voleqn.size="<<voleqn.size()<<std::endl;
//    std::cout<<"voleqn.Name,Equation"<<std::endl;
//    for(int i=0; i<voleqn.size(); i++)
//    {
//        std::cout<<voleqn[i].Name<<","<<voleqn[i].Equation<<std::endl;
//    }
//
//    //std::cout<<"transferFunctionSynthesis="<<transferFunctionSynthesis<<std::endl;
//    std::cout << "color_tf_synthesis=" << color_tf_synthesis << std::endl;
//    std::cout << "opacity_tf_synthesis=" << opacity_tf_synthesis << std::endl;
//
//    std::cout<<std::endl;
}
