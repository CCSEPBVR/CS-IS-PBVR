#ifndef PBVR__RANGE_ESTIMATER_H_INCLUDE
#define PBVR__RANGE_ESTIMATER_H_INCLUDE

#include "FilterInformation.h"
#include "TransferFunctionSynthesizer.h"
#include "VariableRange.h"
#include <string>

class RangeEstimater
{

public:
    static VariableRange Estimation( const size_t step,
                                     const FilterInformationFile& fi,
                                     const TransferFunctionSynthesizer& tfs );

    static VariableRange EstimationList( const size_t step,
                                         const FilterInformationList& fil,
                                         const TransferFunctionSynthesizer& tfs );
};

#endif // PBVR__RANGE_ESTIMATER_H_INCLUDE

