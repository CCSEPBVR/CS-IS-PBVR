#ifndef PBVR__RANGE_ESTIMATER_H_INCLUDE
#define PBVR__RANGE_ESTIMATER_H_INCLUDE

#include "FilterInformation.h"
#include "TransferFunctionSynthesizer.h"
//#include "TransferFunctionSynthesizer_IS.h"
#include "VariableRange.h"
#include <string>
#include "ParticleTransferProtocol.h"
#include "ReversePolishNotation.h"

class RangeEstimater
{
protected:
//    FuncParser::ReversePolishNotation m_rpn;
public:
    static VariableRange Estimation( const size_t step,
                                     const FilterInformationFile& fi,
                                     const TransferFunctionSynthesizer& tfs );

    static VariableRange EstimationList( const size_t step,
                                         const FilterInformationList& fil,
                                         const TransferFunctionSynthesizer& tfs,
                                         const jpv::ParticleTransferClientMessage clntMes  );

};

#endif // PBVR__RANGE_ESTIMATER_H_INCLUDE

