
#include <QtConcurrent/QtConcurrent>
#include <QThread>
#include <QMessageBox>

#include <unistd.h>
#include <iostream>

#include "osireader.h"

OsiReader::OsiReader(int* deltaDelay,
                     const bool& enableSendOut,
                     const std::string& zmqPubPortNum)
    : IMessageSource()
    , isRunning_(false)
    , isReadTerminated_(false)

    , osiFileName_()
    , osiHeaderName_()
    , stamp2Offset_()
    , iterStamp_()
    , iterMutex_()
    , iterChanged_(false)
    , newIterStamp_()

    , deltaDelay_(deltaDelay)

    , enableSendOut_(enableSendOut)
    , zmqPubPortNumber_(zmqPubPortNum)
    , zmqContext_(1)
    , zmqPublisher_(zmqContext_, ZMQ_PUB)
{
}

void
OsiReader::StartReadFile(const QString& osiFileName, const DataType dataType)
{
    zmqPublisher_.close();
    QString errMsg = SetupConnection(true);
    bool success = errMsg.isEmpty();

    if(success)
    {
        QFileInfo fInfo(osiFileName);
        if(fInfo.exists())
        {
            osiFileName_ = osiFileName;
            osiHeaderName_ = osiFileName + "h"; // .txth is header file

            if(QFileInfo::exists(osiHeaderName_))
            {
                ReadHeader();
            }
            else
            {
                QFileInfo fPath(fInfo.path());

                if(fPath.isDir() && fPath.isWritable())
                {
                    success = CreateHeader(errMsg);
                }
                else
                {
                    success = false;
                    errMsg = "No access right to header file: " + osiHeaderName_ + "!\n";
                }
            }

            if(success)
            {
                isPaused_ = false;
                isRunning_ = true;
                iterStamp_ = stamp2Offset_.begin();

                isConnected_ = true;
                emit Connected(defaultDatatype_);

                // update slider range in millisecond level
                int sliderRange = (stamp2Offset_.crbegin()->first)/1000000;
                emit UpdateSliderRange(sliderRange);

                defaultDatatype_ = dataType;
                QtConcurrent::run(this, &OsiReader::SendMessageLoop);
            }
        }
        else
        {
            success = false;
            errMsg = "File: " + osiFileName + " doesn't exist! \n";
        }
    }

    if(!success)
        emit Disconnected(errMsg);
}

void
OsiReader::StopReadFile()
{
    if(isConnected_)
    {
        isReadTerminated_ = false;
        isRunning_ = false;
        isPaused_ = false;

        while(!isReadTerminated_)
        {
            QThread::msleep(50);
        }

        QString errMsg = SetupConnection(false);

        isConnected_ = false;
        emit Disconnected(errMsg);
    }
}

void
OsiReader::SliderValueChanged(int newValue)
{
    iterMutex_.lock();
    iterChanged_ = true;
    iterMutex_.unlock();

    uint64_t nanoTimeStamp = (int64_t)newValue * 1000000;
    for(newIterStamp_ = stamp2Offset_.begin();
        newIterStamp_ != stamp2Offset_.end(); ++newIterStamp_)
    {
        if(newIterStamp_->first > nanoTimeStamp)
        {
            break;
        }
    }

    if(newIterStamp_ == stamp2Offset_.end())
        newIterStamp_ = stamp2Offset_.begin();

    if(isPaused_)
    {
        std::ifstream inputFile (osiFileName_.toStdString().c_str());
        inputFile.seekg(newIterStamp_->second, std::ios_base::beg);

        std::string str_line_input;
        std::string str_line;
        std::string str_backup = "";
        size_t size_found;

        while(getline (inputFile, str_line_input))
        {
            if(str_backup != "")
            {
                str_line = str_backup;
                str_backup = "";
            }
            if((size_found = str_line_input.find("$$__$$")) != std::string::npos)
            {
                str_line += str_line_input.substr(0,size_found);
                str_backup = str_line_input.substr(size_found + 6) +"\n";
            }
            else
            {
                str_line += str_line_input + "\n";
                continue;
            }

            osi::SensorData osiSD;
            if(osiSD.ParseFromString(str_line))
            {
                uint64_t curStamp = GetTimeStampInNanoSecond(osiSD);
                MessageSendout(osiSD, defaultDatatype_);
                ZMQSendOutMessage(str_line);
                // update slider value in millisecond level
                int sliderValue = curStamp / 1000000;
                UpdateSliderValue(sliderValue);
                break;
            }
        }

        inputFile.close();
    }
}

uint64_t
OsiReader::GetTimeStampInNanoSecond(osi::SensorData& osiSD)
{
    uint64_t second = osiSD.mutable_ground_truth()->mutable_global_ground_truth()->timestamp().seconds();
    int32_t nano = osiSD.mutable_ground_truth()->mutable_global_ground_truth()->timestamp().nanos();
    uint64_t timeStamp = second * 1000000000 + nano;

    return timeStamp;
}

void
OsiReader::ReadHeader()
{
    std::ifstream inputHeader (osiHeaderName_.toStdString().c_str());

    std::string line;
    std::getline(inputHeader, line);

    size_t size = std::strtoll(line.c_str(), nullptr, 0);
    stamp2Offset_.reserve(size);

    char* pEnd;
    while(getline (inputHeader, line))
    {
        uint64_t timeStamp = std::strtoull(line.c_str(), &pEnd, 10);
        std::streamoff offset = std::strtol(pEnd, &pEnd, 10);
        stamp2Offset_.push_back(std::make_pair(timeStamp, offset));
    }

    inputHeader.close();
}

bool
OsiReader::CreateHeader(QString& errorMsg)
{
    bool success (true);

    std::ifstream inputFile (osiFileName_.toStdString().c_str());

    std::string str_line_input;
    std::string str_line;
    std::string str_backup = "";
    size_t size_found;

    std::streampos beginPos = inputFile.tellg();
    std::streamoff offset (0);
    bool isFirstMsg (true);
    double_t firstTimeStamp (0);

    while(getline (inputFile, str_line_input))
    {
        if(str_backup != "")
        {
            str_line = str_backup;
            str_backup = "";
        }
        if((size_found = str_line_input.find("$$__$$")) != std::string::npos)
        {
            str_line += str_line_input.substr(0,size_found);
            str_backup = str_line_input.substr(size_found + 6) +"\n";
        }
        else
        {
            str_line += str_line_input + "\n";
            continue;
        }

        osi::SensorData osiSD;
        if(osiSD.ParseFromString(str_line))
        {
            if(isFirstMsg)
            {
                isFirstMsg = false;
                firstTimeStamp = GetTimeStampInNanoSecond(osiSD);
            }

            uint64_t timeStamp = GetTimeStampInNanoSecond(osiSD);
            stamp2Offset_.push_back(std::make_pair(timeStamp - firstTimeStamp, offset));
        }
        else
        {
            success = false;
        }

        offset = inputFile.tellg() - beginPos - str_backup.size();
        str_line = "";
    }

    if(stamp2Offset_.empty() || success == false)
    {
        success = false;
        errorMsg = "Can not parse OSI::SensorData message!\n Wrong file format!\n";
    }
    else
    {
        SaveHeader();
    }

    inputFile.close();

    return success;
}

void
OsiReader::SaveHeader()
{
    std::ofstream outputFile (osiHeaderName_.toStdString().c_str());

    outputFile << stamp2Offset_.size() << std::endl;

    for(const auto& iter: stamp2Offset_)
    {
        outputFile << iter.first << "\t" << iter.second << std::endl;
    }

    outputFile.close();
}

void
OsiReader::SendMessageLoop()
{
    std::ifstream inputFile (osiFileName_.toStdString().c_str());
    bool isFirstMessage (true);

    while(isRunning_)
    {
        if(!isPaused_)
        {
            inputFile.clear();
            inputFile.seekg(iterStamp_->second, std::ios_base::beg);

            std::string str_line_input;
            std::string str_line;
            std::string str_backup = "";
            size_t size_found;

            uint64_t firstTimeStamp (0);
            uint64_t preTimeStamp (0);
            bool isRefreshMessage (true);

            while(getline(inputFile, str_line_input) && !isPaused_ && isRunning_)
            {
                if(iterChanged_)
                    break;

                if(str_backup != "")
                {
                    str_line = str_backup;
                    str_backup = "";
                }
                if((size_found = str_line_input.find("$$__$$")) != std::string::npos)
                {
                    str_line += str_line_input.substr(0,size_found);
                    str_backup = str_line_input.substr(size_found + 6) +"\n";
                }
                else
                {
                    str_line += str_line_input + "\n";
                    continue;
                }

                osi::SensorData osiSD;
                if(osiSD.ParseFromString(str_line))
                {
                    uint64_t curStamp = GetTimeStampInNanoSecond(osiSD);

                    if(isFirstMessage)
                    {
                        firstTimeStamp = curStamp;
                        isFirstMessage = false;
                    }

                    if(isRefreshMessage)
                    {
                        preTimeStamp = curStamp;
                        isRefreshMessage = false;
                    }

                    int64_t sleep = curStamp - preTimeStamp;
                    if(sleep < 0)
                        sleep = 0;

                    sleep /= 1000;
                    sleep += *deltaDelay_ * 1000;

                    usleep(sleep);
                    preTimeStamp = curStamp;

                    MessageSendout(osiSD, defaultDatatype_);
                    ZMQSendOutMessage(str_line);
                    // update slider value in millisecond level
                    int sliderValue = (curStamp - firstTimeStamp) / 1000000;
                    UpdateSliderValue(sliderValue);

                    ++iterStamp_;
                    if(iterStamp_ == stamp2Offset_.end())
                        iterStamp_ = stamp2Offset_.begin();
                }
                else
                {
                    // should never come here: error - osi parse wrong
                    isRunning_ = false;
                    isConnected_ = false;
                    emit Disconnected(tr("Run time error: Can not parse OSI::SensorData message!\n Wrong file format!\n"));
                    break;
                }

                str_line = "";
            }

            str_backup = "";
            str_line = "";

            if(iterChanged_)
            {
                iterStamp_ = newIterStamp_;
                iterMutex_.lock();
                iterChanged_ = false;
                iterMutex_.unlock();
            }
        }
        else
        {
            QThread::msleep(5);
        }
    }

    inputFile.close();

    isReadTerminated_ = true;
}

void
OsiReader::ZMQSendOutMessage(const std::string& message)
{
    if(enableSendOut_ && zmqPublisher_.connected())
    {
        zmq::message_t zmqMessage(message.size());
        memcpy(zmqMessage.data(), message.data(), message.size());
        zmqPublisher_.send(zmqMessage);
    }
}

QString
OsiReader::SetupConnection(bool enable)
{
    QString errMsg;
    if(enable && enableSendOut_)
    {
        if(!zmqPublisher_.connected())
        {
            zmqPublisher_ = zmq::socket_t(zmqContext_, ZMQ_PUB);
            zmqPublisher_.setsockopt(ZMQ_LINGER, 100);
            try
            {
                zmqPublisher_.bind("tcp://*:" + zmqPubPortNumber_);
            }
            catch (zmq::error_t& error)
            {
                zmqPublisher_.close();
                errMsg = "Error connecting to endpoint: " + QString::fromUtf8(error.what());
            }
        }
    }
    else
    {
        if(zmqPublisher_.connected())
        {
            try
            {
                zmqPublisher_.close();
            }
            catch (zmq::error_t& error)
            {
                errMsg = "Error close connecting to endpoint: " + QString::fromUtf8(error.what());
            }
        }
    }

    return errMsg;
}

