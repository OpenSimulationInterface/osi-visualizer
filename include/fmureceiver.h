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

#define FMI_INTEGER_SENSORDATA_IN_BASELO_IDX 0
#define FMI_INTEGER_SENSORDATA_IN_BASEHI_IDX 1
#define FMI_INTEGER_SENSORDATA_IN_SIZE_IDX 2
#define FMI_INTEGER_SENSORDATA_OUT_BASELO_IDX 3
#define FMI_INTEGER_SENSORDATA_OUT_BASEHI_IDX 4
#define FMI_INTEGER_SENSORDATA_OUT_SIZE_IDX 5
#define FMI_INTEGER_LAST_IDX FMI_INTEGER_SENSORDATA_OUT_SIZE_IDX
#define FMI_INTEGER_VARS (FMI_INTEGER_LAST_IDX + 1)




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

        fmi2_value_reference_t vr_[FMI_INTEGER_VARS];
        fmi2_integer_t integerVars_[FMI_INTEGER_VARS];

        // initialize fmu wrapper specific logger, create fmi import context and check fmi version
        bool initializeFMUWrapper();
        // import fmu binary file
        bool importFMU();
        // setup and initialize FMU
        bool initializeFMU();
        // protobuf accessors
        bool get_fmi_sensor_data_in(osi3::SensorData& data);
};


