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
            void ExportOsiMessage();
            void MessageReceived(const osi::SensorData& sensorData,
                                 const DataType datatype);

    public slots:
        void DisconnectRequested();
        void ConnectRequested(QString ipAddress, QString port, DataType dataType);

    private:

        void ReceiveLoop();

        // socket.Connected returns always true after it has been connected once
        // (even after disconnect)
        bool isRunning_;
        bool isThreadTerminated_;

        std::string currentPort_;
        std::string currentEndpoint_;
        zmq::context_t context_;
        zmq::socket_t socket_;
        DataType currentDataType_;
};

