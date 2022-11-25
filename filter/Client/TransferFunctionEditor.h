#ifndef PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_H_INCLUDE
#define PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_H_INCLUDE

#include "kvs/glut/TransferFunctionEditor"
#include "Command.h"
#include <kvs/TransferFunction>

namespace kvs
{
namespace visclient
{
class Command;

class TransferFunctionEditor : public kvs::glut::TransferFunctionEditor
{
public:

    TransferFunctionEditor( kvs::glut::Screen* screen,
                            kvs::TransferFunction* transfer_function = NULL ) :
        kvs::glut::TransferFunctionEditor( screen ),
        m_transfer_function( transfer_function )
    {}

    void apply();
    void bindTransferFunction( kvs::TransferFunction* transfer_function );

private:
    kvs::TransferFunction* m_transfer_function;
};

}
}

#endif    // PBVR__KVS__VISCLIENT__TRANSFER_FUNCTION_EDITOR_H_INCLUDE
