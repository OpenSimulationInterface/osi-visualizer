///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once

#include "types.h"
#include <QtConcurrent/QtConcurrent>
#include <zmq.hpp>

#include "osi_sensordata.pb.h"
#include "imessagesource.h"


class TCPReceiver : public QObject, public IMessageSource
{
    Q_OBJECT

    public:

        TCPReceiver();

        signals:
            void Connected(DataType dataType);
            void Disconnected(const QString& message = "");
            void UpdateSliderTime(int sliderValue);
            void MessageSDReceived(const osi3::SensorData& sd);
            void MessageSVReceived(const osi3::SensorView& sv);

    public slots:
        void DisconnectRequested();
        void ConnectRequested(const QString& ipAddress,
                              const QString& port,
                              DataType dataType);

    private:

        void ReceiveLoop();

        bool isRunning_;
        bool isThreadTerminated_;

        std::string currentPort_;
        std::string currentEndpoint_;
        zmq::context_t context_;
        zmq::socket_t socket_;
        DataType currentDataType_;
};



