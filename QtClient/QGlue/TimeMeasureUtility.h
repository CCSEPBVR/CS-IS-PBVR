#ifndef TIME_MEASURE_UTILITY_H
#define TIME_MEASURE_UTILITY_H

#ifdef ENABLE_TIME_MEASURE
#include "timemeasure.h"

#define MAKE_TIMER(name) kvs::Timer timer##name;
#define START_TIMER(name) timer##name.start();
#define MAKE_AND_START_TIMER(name) kvs::Timer timer##name( kvs::Timer::Start );

#define RECORD_TIMER(record_id, value) TimeMeasure::GetInstance()->record(record_id, value);
#define STOP_AND_RECORD_TIMER(name) \
    timer##name.stop();\
    TimeMeasure::GetInstance()->record("("#name")  ", timer##name.msec());
#define STOP_AND_RECORD_TIMER_WITH_ID(name, record_id) \
    timer##name.stop();\
    TimeMeasure::GetInstance()->record(record_id, timer##name.msec());

#define SHOW_TIMER_RESULT() \
    if(TimeMeasure::GetInstance()->isLastIndex()){\
        std::cerr << "[*****TIME_MEASURE_RESULT*****]" << std::endl;\
        for(auto const &key : TimeMeasure::GetInstance()->keys()){\
            std::cerr << "[TIME_MEASURE][" << frame_index << "] " << TimeMeasure::GetInstance()->loggingString(key) << std::endl;\
        }\
    }
#define SET_INDEX_FOR_TIMER(index) TimeMeasure::GetInstance()->setIndex(index);

#define STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(name, record_id_left, record_id_righ) \
    timer##name.stop(); \
    if(TimeMeasure::GetInstance()->eye() == 0) { \
        TimeMeasure::GetInstance()->record(record_id_left, timer##name.msec()); \
    }else if(TimeMeasure::GetInstance()->eye() == 1) { \
        TimeMeasure::GetInstance()->record(record_id_righ, timer##name.msec()); \
    }else{ \
        TimeMeasure::GetInstance()->record("Unknown", timer##name.msec()); \
    }
#define SET_EYE_FOR_TIMER(eye) TimeMeasure::GetInstance()->setEye(eye);
#else
#define MAKE_TIMER(name)
#define START_TIMER(name)
#define MAKE_AND_START_TIMER(name)

#define RECORD_TIMER(record_id, value)
#define STOP_AND_RECORD_TIMER(name)
#define STOP_AND_RECORD_TIMER_WITH_ID(name, record_id)

#define SHOW_TIMER_RESULT()
#define SET_INDEX_FOR_TIMER(index)

#define STOP_AND_RECORD_TIMER_WITH_ID_FOR_EACH_EYE(name, record_id_left, record_id_righ)
#define SET_EYE_FOR_TIMER(eye)
#endif // ENABLE_TIME_MEASURE
#endif // TIME_MEASURE_UTILITY_H
