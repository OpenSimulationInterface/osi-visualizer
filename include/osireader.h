///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///


#pragma once

#include <osi_version.pb.h>
#include <osi_sensordata.pb.h>

#include <zmq.hpp>

#include <fstream>
#include <iostream>
#include <unistd.h>

#include <QObject>
#include <QMutex>

#include "imessagesource.h"
#include "types.h"


class OsiReader: public QObject, public IMessageSource
{
    Q_OBJECT

    public:
        OsiReader(int* deltaDelay,
                  const bool& enableSendOut,
                  const std::string& zmqPubPortNum);

        QString SetupConnection(bool enable);

        void SetSendOutPortNum(const std::string& port) { zmqPubPortNumber_ = port; }

        signals:
            void Connected(DataType dataType);
            void UpdateSliderRange(int sliderRange);
            void UpdateSliderValue(int sliderValue);
            void Disconnected(const QString& message = "");
            void MessageSendout(const osi::SensorData& SensorData,
                                const DataType datatype);

    public slots:
            void StartReadFile(const QString& osiFileName, const DataType dataType);
            void StopReadFile();
            void SliderValueChanged(int newValue);

    private:

        uint64_t GetTimeStampInNanoSecond(osi::SensorData &osiSD);

        void ReadHeader();
        bool CreateHeader(QString& errorMsg);
        void SaveHeader();

        void SendMessageLoop();

        void ZMQSendOutMessage(const std::string& message);



        bool isRunning_;
        bool isReadTerminated_;

        QString osiFileName_;
        QString osiHeaderName_;

        // std::map<time stamp in nanosecond, input file stream position>
        std::vector<std::pair<uint64_t, std::streamoff> > stamp2Offset_;
        std::vector<std::pair<uint64_t, std::streamoff> >::iterator iterStamp_;
        QMutex iterMutex_;
        bool iterChanged_;
        std::vector<std::pair<uint64_t, std::streamoff> >::iterator newIterStamp_;

        const int* const deltaDelay_;

        const bool&     enableSendOut_;
        std::string     zmqPubPortNumber_;
        zmq::context_t  zmqContext_;
        zmq::socket_t   zmqPublisher_;

        // read from input file: data type is always SensorData
        DataType defaultDatatype_ = DataType::Groundtruth;
        const QString defaultHeaderPrifix_ = "Header_";
};

