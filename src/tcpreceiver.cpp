#include "tcpreceiver.h"
#include "utils.h"
#include <QThread>


/*
 *
 * The constructor initializes the ZMQ context
 *
 */

TCPReceiver::TCPReceiver(int type)
    : IMessageSource()

    , isRunning_(false)
    , isThreadTerminated_(false)
    , socketType_(type)
    , currentPort_("")
    , currentEndpoint_("")
    , context_(1)
    , socket_(context_, type)
    , currentDataType_(DataType::SensorView)
{
    // Disable buffering
    socket_.setsockopt(ZMQ_CONFLATE, 1);

    int a, b, c;
    zmq::version(&a, &b, &c);
    qDebug() << "ZMQ version: " + QString::number(a) + "."
             + QString::number(b) + "." + QString::number(c);
}

// This function is called when the user presses the connect button
void
TCPReceiver::ConnectRequested(const QString &ipAddress,
                              const QString &port,
                              DataType dataType)
{
    currentDataType_ = dataType;
    currentPort_ = port.toStdString();
    std::string address = ipAddress.toStdString();
    currentEndpoint_ = "tcp://" + address + ":" + currentPort_;

    try
    {
        socket_.connect(currentEndpoint_);
    }
    catch (zmq::error_t& error)
    {
        QString message = "Error connecting to endpoint: " + QString::fromUtf8(error.what());
        emit Disconnected(message);
        return;
    }

    if (socketType_ == ZMQ_SUB)
    {
        socket_.setsockopt(ZMQ_SUBSCRIBE, 0, 0);
    }

    isPaused_ = false;
    isRunning_ = true;
    QtConcurrent::run(this, &TCPReceiver::ReceiveLoop);

    isConnected_ = true;
    emit Connected(currentDataType_);
}

// This function is called when the user presses the disconnect button
void
TCPReceiver::DisconnectRequested()
{
    if (!isConnected_)
    {
        return;
    }

    isThreadTerminated_ = false;
    isRunning_ = false;
    // The status of a QFuture object can neither be monitored, nor manipulated,
    // despite it contains methods that imply it.

    while (!isThreadTerminated_)
    {
        QThread::msleep(50);
    }

    QString message;
    try
    {
        socket_.disconnect(currentEndpoint_);
    }
    catch (zmq::error_t& error)
    {
        message = "Error disconnecting endpoint: " + QString::fromUtf8(error.what());
    }

    isConnected_ = false;
    emit Disconnected(message);
}

/*
 * This function checks if new osi buffers can be received.
 * In this case, the received data will be parsed to SensorData,
 * and a signal with the last received SensorData buffer is emitted.
 */
void
TCPReceiver::ReceiveLoop()
{
    while (isRunning_)
    {
        bool msgReceived = false;

        if (!isPaused_)
        {
            zmq::message_t message;

            try
            {
                socket_.recv(&message, ZMQ_NOBLOCK);
            }
            catch (zmq::error_t& e)
            {
                qDebug() << "ZMQ-Error: " << e.what();
            }
            catch (...)
            {
                qDebug() << "Error while Receive Operation";
            }

            if (message.size() > 0)
            {
                msgReceived = true;

                if(currentDataType_ == DataType::SensorView)
                {
                    osi3::SensorView sv;
                    if(sv.ParseFromArray(message.data(),(int)message.size()))
                    {
                        uint64_t curStamp = ::GetTimeStampInNanoSecond<osi3::SensorView>(sv);
                        emit MessageSVReceived(sv);
                        int sliderValue = curStamp / 1000000;
                        emit UpdateSliderTime(sliderValue);
                    }
                    else
                    {
                        qDebug() << "SensorView receiving error";
                    }
                }
                else //if(currentDataType_ == DataType::SensorData)
                {
                    osi3::SensorData sd;
                    if(sd.ParseFromArray(message.data(),(int)message.size()))
                    {
                        uint64_t curStamp = ::GetTimeStampInNanoSecond<osi3::SensorData>(sd);
                        emit MessageSDReceived(sd);
                        int sliderValue = curStamp / 1000000;
                        emit UpdateSliderTime(sliderValue);
                    }
                    else
                    {
                        qDebug() << "SensorData receiving error";
                    }
                }
            }
        }

        if (!msgReceived)
        {
            QThread::msleep(5);
        }
    }

    isThreadTerminated_ = true;
}


