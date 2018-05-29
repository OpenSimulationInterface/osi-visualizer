///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///


#pragma once

#include "osi_version.pb.h"
#include "osi_sensordata.pb.h"

#include <zmq.hpp>

#include <fstream>
#include <iostream>
#include <unistd.h>

#include <QObject>
#include <QMutex>

#include "imessagesource.h"
#include "types.h"


extern "C" {
#include <JM/jm_portability.h>
#include <fmilib.h>
}


#define FMI_SENDER_NAME   "sender"
#define FMI_RECEIVER_NAME "receiver"
#define FMI_ADDRESS_NAME  "address"
#define FMI_PORT_NAME     "port"

#define FMI_DATA_IN_BASELO_NAME "OSMPSensorViewIn.base.lo"
#define FMI_DATA_IN_BASEHI_NAME "OSMPSensorViewIn.base.hi"
#define FMI_DATA_IN_SIZE_NAME   "OSMPSensorViewIn.size"

/* local index defines */
#define FMI_INTEGER_SENSORDATA_IN_BASELO_IDX 0 // correspond to FMI_DATA_IN_BASELO_NAME
#define FMI_INTEGER_SENSORDATA_IN_BASEHI_IDX 1 // correspond to FMI_DATA_IN_BASEHI_NAME
#define FMI_INTEGER_SENSORDATA_IN_SIZE_IDX   2 // correspond to FMI_DATA_IN_SIZE_NAME
#define FMI_INTEGER_LAST_IN_IDX FMI_INTEGER_SENSORDATA_IN_SIZE_IDX
#define FMI_INTEGER_IN_VARS (FMI_INTEGER_LAST_IN_IDX + 1)


class OsiReader: public QObject, public IMessageSource
{
    Q_OBJECT

    public:
        OsiReader(int* deltaDelay,
                  const bool& enableSendOut,
                  const std::string& pubPortNum,
                  const bool &enalbeFMU,
                  const std::string &fmuPath = "");

        QString SetupConnection(bool enable);

        void SetSendOutPortNum(const std::string& port) { pubPortNumber_ = port; }

        signals:
            void Connected(DataType dataType);
            void UpdateSliderRange(int sliderRange);
            void UpdateSliderValue(int sliderValue);
            void Disconnected(const QString& message = "");
            void MessageSDSendout(const osi3::SensorData& SensorData);
            void MessageSVSendout(const osi3::SensorView& SensorView);

    public slots:
            void StartReadFile(const QString& osiFileName,
                               const DataType dataType,
                               const QString& fmuPath);
            void StopReadFile();
            void SliderValueChanged(int newValue);

    private:

        void ReadHeader();
        bool CreateHeader(QString& errorMsg);
        template <typename T> bool BuildUpStamps(bool& isFirstMsg,
                                                 double& firstTimeStamp,
                                                 const std::string& message,
                                                 const std::streamoff& offset);

        void SaveHeader();

        void SendMessageLoop();
        template <typename T> bool SendMessage(T& data,
                                               bool& isFirstMessage,
                                               bool& isRefreshMessage,
                                               uint64_t& preTimeStamp,
                                               const std::string& message);

        void SendOutMessage(const std::string& message);

        QString SetZMQConnection();
        QString FreeZMQConnection();
        QString SetFMUConnection();
        void FreeFMUConnection();



        bool isRunning_;
        bool isReadTerminated_;

        QString osiFileName_;
        QString osiHeaderName_;

        uint64_t firstTimeStamp_;
        // std::map<time stamp in nanosecond, input file stream position>
        std::vector<std::pair<uint64_t, std::streamoff> > stamp2Offset_;
        std::vector<std::pair<uint64_t, std::streamoff> >::iterator iterStamp_;
        QMutex iterMutex_;
        bool iterChanged_;
        std::vector<std::pair<uint64_t, std::streamoff> >::iterator newIterStamp_;

        const int* const deltaDelay_;

        const bool&     enableSendOut_;
        std::string     pubPortNumber_;
        std::string     currentBuffer_;

        // ZMQ
        zmq::context_t  zmqContext_;
        zmq::socket_t   zmqPublisher_;

        // FMU
        const bool&    enableFMU_;
        fmi2_import_t* fmu_;
        fmi2_callback_functions_t callBackFunctions_;
        jm_callbacks callbacks_;
        fmi_import_context_t* fmuContext_;
        jm_status_enu_t jmStatus_;
        fmi2_status_t fmiStatus_;
        fmi2_real_t tStart_;
        fmi2_real_t tEnd_;
        fmi2_real_t tCurrent_;
        fmi2_real_t hStep_;
        std::string FMUPath_;
        std::string tmpPath_;
        enum class LogLevel
        {
            Warn,
            Debug
        };
        LogLevel logLevel_;
        fmi2_value_reference_t vr_[FMI_INTEGER_IN_VARS];

        // Initialize fmu wrapper specific logger, create fmi import context and check fmi version
        bool initializeFMUWrapper();
        // import fmu binary file
        bool importFMU();
        // setup and Initialize FMU
        bool initializeFMU();
        // protobuf accessors
        void set_fmi_sensor_data_out();


        // read from input file: data type is always SensorData
        DataType currentDataType_ = DataType::SensorView;
        const QString defaultHeaderPrifix_ = "Header_";
};

