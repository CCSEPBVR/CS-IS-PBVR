#ifndef PBVR__TRANSFER_FUNCTION_SYNTHESIZER_H_INCLUDE
#define PBVR__TRANSFER_FUNCTION_SYNTHESIZER_H_INCLUDE

#include <map>
#include <string>
#include <stdexcept>

#include "TransferFunction.h"
#include <kvs/RGBColor>
#include "CellBase.h"

#include "function.h"
#include "function_op.h"
#include "function_parser.h"

#include "VariableRange.h"

// Dictionaries Type-definition
typedef std::map<std::string, pbvr::TransferFunction> TransferFunctionMap;
typedef std::map<std::string, FuncParser::Variable>  VariableMap;
typedef std::map<std::string, std::string>           FunctionStringMap;
typedef std::map<std::string, FuncParser::Function>  FunctionMap;
typedef std::map<std::string, size_t>                QuantityMap;
typedef std::map<std::string, std::string>           TransfuncQuantityBindMap;

typedef std::vector<std::string>                     TagStringVec;

class CharPointerVector : public std::vector<char*>
{
public:
    void pushBackString( const std::string& nm )
    {
        char* tmp = new char[ nm.size() + 1 ];
        strcpy( tmp, nm.c_str() );
        this->push_back( tmp );
    }
    CharPointerVector() {};
    ~CharPointerVector()
    {
        for ( std::vector<char*>::iterator t = this->begin(); t != this->end(); t++ )
        {
            delete[] ( *t );
        }
        this->clear();
    }
};

class TransferFunctionSynthesizer
{

    typedef std::map<std::string, kvs::RGBColor> str_color_map;

public:
    class NumericException : public std::runtime_error
    {
    public:
        NumericException( const std::string& str ):
            std::runtime_error( str )
        {
        }
        virtual ~NumericException() throw()
        {
        }
    };

protected:
    // [ キー, 値 ]
    // qは変量 q = q(X,Y,Z)
    // fは変量の合成式 f = f(X,Y,Z,q1,..,qn)
    // tは１次元伝達関数 t = t(f). tは色関数cと不透明度関数oの組.
    FuncParser::Variables m_variables;
    VariableMap           m_variable_map;// [ 変量名(X,Y,Z,q1,..,qn,f1,..,fn), 変量 ].
    FunctionMap           m_volume_function_map;//[ 変量名(f1,..,fn), 変量の合成式 ]

    TransferFunctionMap      m_transfer_function_map_color; // [ 色関数名(c1,..,c5), 色関数 ]
    TransferFunctionMap      m_transfer_function_map_opacity; // [ 不透明度関数名(o1,..,o5), 不透明度関数 ]
    TransfuncQuantityBindMap m_transfer_function_quantity_map_color; // [ 色関数名(c1,..,c5), 色関数cの引数 ]
    TransfuncQuantityBindMap m_transfer_function_quantity_map_opacity; // [ 不透明度関数名(o1,..,o5), 不透明度関数oの引数 ]
    FunctionStringMap        m_volume_function_string_map;
    QuantityMap              m_quantity_map; // [ 変量名(q1,..,qn), コンポーネントID ]

    TagStringVec             m_time_step_vector;
    CharPointerVector        m_char_pointer_vector;

    // delete by @hira at 2016/12/01 : 1次伝達関数（色、不透明度）= m_color_transfunc_synthesis, m_opacity_transfunc_synthesisに分割
    // std::string          m_synthesozer_function_string;
    // FuncParser::Function m_synthesizer_function;

    float m_max_opacity;
    float m_max_density;
    float m_sampling_volume_inverse;

    VariableRange m_variable_range;

    // add by @hira at 2016/12/01 : 1次伝達関数（色、不透明度）
    std::string m_color_transfunc_synthesis;
    std::string m_opacity_transfunc_synthesis;
    FuncParser::Function m_color_synthesizer_function;
    FuncParser::Function m_opacity_synthesizer_function;

public:

    TransferFunctionSynthesizer();
    explicit TransferFunctionSynthesizer( const TransferFunctionSynthesizer& s );

    virtual ~TransferFunctionSynthesizer();

    const TransferFunctionMap& colorTransferFunctionMap() const;
    const TransferFunctionMap& opacityTransferFunctionMap() const;

    const TransfuncQuantityBindMap& colorTransferFunctionBindMap() const;
    const TransfuncQuantityBindMap& opacityTransferFunctionBindMap() const;

    const FunctionStringMap& volumeFunctionStringMap() const;

    const QuantityMap& quantityMap() const;

    const FunctionMap& functionMap() const;

    const std::string& synthesizerFunctionString() const;

    //kawamura
    void multivariateMinMaxValues( pbvr::CellBase<kvs::Real32>* cell,
                                   const size_t cell_index,
                                   const size_t nnodes );

    void multivariateMinMaxAveragedValues( pbvr::CellBase<kvs::Real32>* cell,
                                           const size_t cell_index,
                                           const size_t nnodes );

    void setMaxOpacity( const float opacity );
    void setMaxDensity( const float density );
    void setSamplingVolumeInverse( const float sampling_volume_inverse );

    float getMaxOpacity() const;
    float getMaxDensity() const;
    float getSamplingVolumeInverse() const;

    VariableRange& variableRange();
    const VariableRange& variableRange() const;
    void  mergeVarRange( const TransferFunctionSynthesizer& tfs );

    void copy( const TransferFunctionSynthesizer& s );
    TransferFunctionSynthesizer& operator= ( const TransferFunctionSynthesizer& s );

    void setValue( const std::string& nm, const float value );
    float getValue( const std::string& nm );

    void setTransferFunctionMap( const TransferFunctionMap& ptf_map );
    void setColorTransferFunctionMap( const TransferFunctionMap& ptf_map );
    void setOpacityTransferFunctionMap( const TransferFunctionMap& ptf_map );

    void setTransfuncQuantityBindMap( const TransfuncQuantityBindMap& tfq_map );
    void setColorTransfuncQuantityBindMap( const TransfuncQuantityBindMap& tfq_map );
    void setOpacityTransfuncQuantityBindMap( const TransfuncQuantityBindMap& tfq_map );

    void setVolumeFunctionStringMap( const FunctionStringMap& volumeFunctionStringMap );
    void setQuantityMap( const QuantityMap& qmap );
    void setSynthFunctionString( const std::string& synth_func_str );
    // add by @hira at 2016/12/01
    void setColorSynthFunctionString( const std::string& synth_func_str );
    void setOpacitySynthFunctionString( const std::string& synth_func_str );
    const std::string& colorTransfuncSynthesis() const;
    const std::string& opacityTransfuncSynthesis() const;

    void optimize();
    void optimize2();
    void create();
    void clear();

    float calculateOpacity( pbvr::CellBase<kvs::Real32>* cell,
                            const size_t n,
                            const kvs::Vector3f& coord );
    float calculateAveragedOpacity( pbvr::CellBase<kvs::Real32>* cell,
                                    const size_t n,
                                    const kvs::Vector3f& coord );
    float calculateAveragedOpacityBatch( pbvr::CellBase<kvs::Real32>* cell,
                                         const size_t n,
                                         const kvs::Vector3f& coord );
    float calculateGravityPointOpacity( pbvr::CellBase<kvs::Real32>* cell,
                                        const size_t n,
                                        const kvs::Vector3f& coord );

    kvs::RGBColor calculateColor( pbvr::CellBase<kvs::Real32>* cell,
                                  const size_t n,
                                  const kvs::Vector3f& coord );
    kvs::RGBColor calculateAveragedColor( pbvr::CellBase<kvs::Real32>* cell,
                                          const size_t n,
                                          const kvs::Vector3f& coord );
    kvs::RGBColor calculateGravityPointColor( pbvr::CellBase<kvs::Real32>* cell,
                                              const size_t n,
                                              const kvs::Vector3f& coord );
    void calculateOpacityOfNode( pbvr::CellBase<kvs::Real32>* cell,
                                 pbvr::CellBase<kvs::Real32>* cell_opacity,
                                 const size_t n );
private:
    void add_variant( const std::string& nm );
    void initialize_volume_function();
    void register_volume_function();

public:
    static void AssertValid( const float& v, const char* file, const int line );
};

#endif // PBVR__TRANSFER_FUNCTION_SYNTHESIZER_H_INCLUDE

