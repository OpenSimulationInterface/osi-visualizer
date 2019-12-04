///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_OSIREADER_H
#define OSI_VISUALIZER_OSIREADER_H

#include "osi_sensordata.pb.h"
#include "osi_version.pb.h"

#include <zmq.hpp>

#include <fstream>
#include <iostream>

#include <QMutex>
#include <QObject>

#include "imessagesource.h"
#include "types.h"

class OsiReader : public QObject, public IMessageSource
{
    Q_OBJECT

  public:
    OsiReader(const int* deltaDelay, const bool& enableSendOut, std::string  pubPortNum, int socketType);

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
    void StartReadFile(const QString& osiFileName, const DataType dataType);
    void StopReadFile();
    void SliderValueChanged(int newValue);

  private:
    void ReadHeader();
    bool CreateHeader(QString& errorMsg);
    template <typename T>
    bool BuildUpStamps(bool& isFirstMsg,
                       double& firstTimeStamp,
                       const std::string& message,
                       const std::streamoff& offset);

    void SaveHeader();

    void SendMessageLoop();
    template <typename T>
    bool SendMessage(T& data,
                     bool& isFirstMessage,
                     bool& isRefreshMessage,
                     uint64_t& preTimeStamp,
                     const std::string& message);

    void SendOutMessage(const std::string& message);

    QString SetZMQConnection();
    QString FreeZMQConnection();

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

    const bool& enableSendOut_;
    std::string pubPortNumber_;
    std::string currentBuffer_;

    // ZMQ
    int socketType_;
    zmq::context_t zmqContext_;
    zmq::socket_t zmqPublisher_;

    // read from input file: data type is always SensorData
    DataType currentDataType_ = DataType::SensorView;
    const QString defaultHeaderPrifix_ = "Header_";
};
#endif  // OSI_VISUALIZER_OSIREADER_H
