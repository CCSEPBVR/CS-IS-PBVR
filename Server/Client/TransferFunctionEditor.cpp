#include "TransferFunctionEditor.h"
#include "Command.h"

using namespace kvs::visclient;

void TransferFunctionEditor::apply()
{
    kvs::glut::TransferFunctionEditor::apply();
    if ( m_transfer_function )
    {
        *m_transfer_function = transferFunction();
    }
    screen()->redraw();
}

void TransferFunctionEditor::bindTransferFunction( kvs::TransferFunction* transfer_function )
{
    m_transfer_function = transfer_function;
    setTransferFunction( *m_transfer_function );
    this->redraw();
}
