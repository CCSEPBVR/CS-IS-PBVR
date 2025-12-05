#include <vismodule/Argument>
#include <vismodule/VariableRange>
#include <vismodule/ExtendedTransferFunction>
#include <vismodule/ParticleProperty>

// void setDefalutTransferFunction( jpv::ParticleTransferServerMessage* servMes, const int tf_number );
void setDefalutTransferFunctionToArgument( ParticleProperty& param, const VariableRange vr, const int tf_number );
// void setClientTransferFunctionToArgument( Argument *param, const jpv::ParticleTransferClientMessage& clntMes );
// void setParamTransferFunctionToServer( jpv::ParticleTransferServerMessage* servMes, Argument *param );



