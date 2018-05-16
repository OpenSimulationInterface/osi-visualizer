///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///



#pragma once

#include <QtConcurrent/QtConcurrent>

#include <string>

#include "osi_sensordata.pb.h"
#include "imessagesource.h"
#include "types.h"


extern "C" {
#include <JM/jm_portability.h>
#include <fmilib.h>
}

/* The following names come from FMU modelDescription.in.xml */
#define FMI_SENDER_NAME   "sender"
#define FMI_RECEIVER_NAME "receiver"
#define FMI_ADDRESS_NAME  "address"
#define FMI_PORT_NAME     "port"

#define FMI_DATA_OUT_BASELO_NAME "OSMPSensorDataOut.base.lo"
#define FMI_DATA_OUT_BASEHI_NAME "OSMPSensorDataOut.base.hi"
#define FMI_DATA_OUT_SIZE_NAME   "OSMPSensorDataOut.size"

/* local index defines */
#define FMI_INTEGER_SENSORDATA_OUT_BASELO_IDX 0 // correspond to FMI_DATA_OUT_BASELO_NAME
#define FMI_INTEGER_SENSORDATA_OUT_BASEHI_IDX 1 // correspond to FMI_DATA_OUT_BASEHI_NAME
#define FMI_INTEGER_SENSORDATA_OUT_SIZE_IDX   2 // correspond to FMI_DATA_OUT_SIZE_NAME
#define FMI_INTEGER_LAST_OUT_IDX FMI_INTEGER_SENSORDATA_OUT_SIZE_IDX
#define FMI_INTEGER_OUT_VARS (FMI_INTEGER_LAST_OUT_IDX + 1)

class FMUReceiver: public QObject, public IMessageSource
{
    Q_OBJECT

    public:
        FMUReceiver();

        signals:
            void Connected(DataType dataType);
            void Disconnected(const QString& message = "");
            void MessageReceived(const osi3::SensorData& sensorData,
                                 const DataType datatype);

    public slots:
            void DisconnectRequested();
            void ConnectRequested(const QString& ipAddress,
                                  const QString& port,
                                  const QString& fmuPath,
                                  DataType dataType);

    private:

        void ReceiveLoop();


        bool isRunning_;
        bool isThreadTerminated_;
        DataType currentDataType_;


        // FMU interface
        fmi2_import_t* fmu_;
        fmi2_callback_functions_t callBackFunctions_;
        jm_callbacks callbacks_;
        fmi_import_context_t* context_;

        jm_status_enu_t jmStatus_;
        fmi2_status_t fmiStatus_;

        fmi2_real_t tStart_;
        fmi2_real_t tEnd_;
        fmi2_real_t tCurrent_;
        fmi2_real_t hStep_;

        std::string ip_;
        std::string port_;
        std::string FMUPath_;
        std::string tmpPath_;
        enum class LogLevel
        {
            Warn,
            Debug
        };
        LogLevel logLevel_;
        std::string currentBuffer_;
        fmi2_value_reference_t vr_[FMI_INTEGER_OUT_VARS];

        // initialize fmu wrapper specific logger, create fmi import context and check fmi version
        bool initializeFMUWrapper();
        // import fmu binary file
        bool importFMU();
        // setup and initialize FMU
        bool initializeFMU();
        // protobuf accessors
        bool get_fmi_sensor_data_in(osi3::SensorData& data);
};


