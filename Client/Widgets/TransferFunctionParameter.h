#ifndef TRANSFERFUNCTIONPARAMETER_H
#define TRANSFERFUNCTIONPARAMETER_H
#include <kvs/TransferFunction>
#include "FrequencyTable.h"
class TransferFunctions : public kvs::TransferFunction
{
public:
    enum Selection //おそらく必要ない。
    {
        SelectExtendTransferFunction = 0,
        SelectTransferFunction = 1,
    };

    enum SelectedRange
    {
        UserDefinedRange = 0,
        ServerSideRange = 1
    };

public:
    Selection m_selection;

    std::string m_color_function_name;
    std::string m_color_variable;
    SelectedRange m_color_confirmed_select_range;
    SelectedRange m_color_stored_select_range;
    float m_color_user_defined_min;
    float m_color_user_defined_max;
    float m_color_server_side_min;
    float m_color_server_side_max;
    kvs::visclient::FrequencyTable m_color_histogram;

    std::string m_opacity_function_name;
    std::string m_opacity_variable;
    SelectedRange m_opacity_confirmed_select_range;
    SelectedRange m_opacity_stored_select_range;
    float m_opacity_user_defined_min;
    float m_opacity_user_defined_max;
    float m_opacity_server_side_min;
    float m_opacity_server_side_max;
    kvs::visclient::FrequencyTable m_opacity_histogram;
};

class TransferFunctionParameter
{
public:
    // int32_t m_resolution;
    // std::string m_volume_synthesis;

    int m_number_of_transfer_functions;

    std::string m_color_synthesizer;
    std::string m_opacity_synthesizer;

    std::vector<TransferFunctions> m_transfer_function;

public:
    TransferFunctions* getTransferFunction( const std::string &colorName, const std::string &opacityName )
    {
        std::vector<TransferFunctions>::iterator itr;
        for( itr = this->m_transfer_function.begin(); itr != this->m_transfer_function.end(); itr++ )
        {
            TransferFunctions func = (*itr);
            if( func.m_color_function_name == colorName && func.m_opacity_function_name == opacityName )
            {
                return &(*itr);
            }
        }
        return nullptr;
    }

    TransferFunctions* getTransferFunction( const std::string &functionName )
    {
        std::vector<TransferFunctions>::iterator itr;
        for( itr = this->m_transfer_function.begin(); itr != this->m_transfer_function.end(); itr++ )
        {
            TransferFunctions func = (*itr);
            if( func.m_color_function_name == functionName || func.m_opacity_function_name == functionName )
            {
                return &(*itr);
            }
        }
        return nullptr;
    }

    TransferFunctions* getTransferFunction( const int n )
    {
        if( n >= 0 && n < static_cast<int>(this->m_transfer_function.size()) )
        {
            return &this->m_transfer_function[n];
        }
        return nullptr;
    }

    void addTransferFunction( const std::string &colorName, const std::string &opacityName, const std::string &variable )
    {
        TransferFunctions *func = getTransferFunction( colorName, opacityName );
        if( func != nullptr )
        {
            return;
        }

        TransferFunctions transferFunction;

        transferFunction.m_color_function_name = colorName;
        transferFunction.m_color_variable = variable;
        transferFunction.m_color_stored_select_range = TransferFunctions::UserDefinedRange;
        transferFunction.m_color_user_defined_min = 0.0;
        transferFunction.m_color_user_defined_max = 1.0;
        transferFunction.m_color_server_side_min = std::numeric_limits<float>::quiet_NaN(); //Nan 追加された段階でサーバからの値を取得できないため。
        transferFunction.m_color_server_side_max = std::numeric_limits<float>::quiet_NaN(); //Nan 追加された段階でサーバからの値を取得できないため。

        transferFunction.m_opacity_function_name = opacityName;
        transferFunction.m_opacity_variable = variable;
        transferFunction.m_opacity_stored_select_range = TransferFunctions::UserDefinedRange;
        transferFunction.m_opacity_user_defined_min = 0.0;
        transferFunction.m_opacity_user_defined_max = 1.0;
        transferFunction.m_opacity_server_side_min = std::numeric_limits<float>::quiet_NaN(); //Nan 追加された段階でサーバからの値を取得できないため。
        transferFunction.m_opacity_server_side_max = std::numeric_limits<float>::quiet_NaN(); //Nan 追加された段階でサーバからの値を取得できないため。
        m_transfer_function.push_back( transferFunction );
    }

    void removeTransferFunction(  const std::string &colorName, const std::string &opacityName )
    {
        std::vector<TransferFunctions>::iterator itr;
        for( itr = this->m_transfer_function.begin(); itr != this->m_transfer_function.end(); itr++ )
        {
            if( itr->m_color_function_name == colorName && itr->m_opacity_function_name == opacityName )
            {
                this->m_transfer_function.erase( itr );
                break;
            }
        }
        return;
    }

    void setColorFunctionVariable( const std::string &colorName, const std::string &variable )
    {
        TransferFunctions *func = this->getTransferFunction( colorName );
        if( func != nullptr )
        {
            func->m_color_function_name = colorName;
            func->m_color_variable = variable;
            return;
        }
    }

    void setOpacityFunctionVariable( const std::string &opacityName, const std::string &variable )
    {
        TransferFunctions *func = this->getTransferFunction( opacityName );
        if( func != nullptr )
        {
            func->m_opacity_function_name = opacityName;
            func->m_opacity_variable = variable;
            return;
        }
    }

    void setColorFunctionRange( const std::string &colorName, float min, float max )
    {
        std::cout << colorName << ":" << min << "," << max << std::endl;
        TransferFunctions *func = this->getTransferFunction( colorName );
        if( func != nullptr )
        {
            func->m_color_user_defined_min = min;
            func->m_color_user_defined_max = max;
        }
    }

    void setOpacityFunctionRange( const std::string &opacityName, float min, float max )
    {
        TransferFunctions *func = this->getTransferFunction( opacityName );
        if( func != nullptr )
        {
            func->m_opacity_user_defined_min = min;
            func->m_opacity_user_defined_max = max;
        }
    }
};
#endif // TRANSFERFUNCTIONPARAMETER_H
