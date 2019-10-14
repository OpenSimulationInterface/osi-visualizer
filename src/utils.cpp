
#include "utils.h"

#include "osi_sensordata.pb.h"
#include "osi_sensorview.pb.h"

template <>
uint64_t GetTimeStampInNanoSecond<osi3::SensorData>(const osi3::SensorData& data)
{
    uint64_t second = data.timestamp().seconds();
    int32_t nano = data.timestamp().nanos();
    uint64_t timeStamp = second * 1000000000 + nano;

    return timeStamp;
}

template <>
uint64_t GetTimeStampInNanoSecond<osi3::SensorView>(const osi3::SensorView& sv)
{
    uint64_t second = sv.global_ground_truth().timestamp().seconds();
    int32_t nano = sv.global_ground_truth().timestamp().nanos();
    uint64_t timeStamp = second * 1000000000 + nano;

    return timeStamp;
}
