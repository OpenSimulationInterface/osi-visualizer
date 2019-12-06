#include <QMessageBox>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

#include <iostream>

#include "osireader.h"
#include "utils.h"

#include <string>
#include <iomanip>
#include <utility>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <regex>

int read_bytes(char *message_buf, size_t size, FILE *fd) {
    size_t already_read = 0;
    while (already_read < size) {
        int ret = fread(message_buf + already_read, sizeof(message_buf[0]), size - already_read, fd);
        if (ret < 0) {
            std::cout << "Failed to read" << std::endl;
            return -3;
        }
        if (ret == 0) {
            std::cout << "Unexpected end of file" << std::endl;
            return -4;
        }
        already_read += ret;
    }
}

int realloc_buffer(char **message_buf, size_t new_size) {
    char *new_ptr = *message_buf;
    new_ptr = (char *)realloc(new_ptr, new_size);
    if (new_ptr == nullptr) {
        std::cout << "Failed to allocate buffer memory" << std::endl;
        return -1;
    }
    *message_buf = new_ptr;
    return 0;
}


OsiReader::OsiReader(const int* deltaDelay, const bool& enableSendOut, std::string  pubPortNum, int socketType)
    : IMessageSource(),
      isRunning_(false),
      isReadTerminated_(false),
      osiFileName_(),
      osiHeaderName_(),
      firstTimeStamp_(0),
      stamp2Offset_(),
      iterStamp_(),
      iterMutex_(),
      iterChanged_(false),
      newIterStamp_(),
      deltaDelay_(deltaDelay),
      enableSendOut_(enableSendOut),
      pubPortNumber_(std::move(pubPortNum)),
      currentBuffer_(),
      socketType_(socketType),
      zmqContext_(1),
      zmqPublisher_(zmqContext_, socketType)
{
}

void OsiReader::StartReadFile(const QString& osiFileName, const DataType dataType)
{
    zmqPublisher_.close();
    currentDataType_ = dataType;
    QString errMsg = SetupConnection(true);
    bool success = errMsg.isEmpty();

    if (success)
    {
        QFileInfo fInfo(osiFileName);
        if (fInfo.exists())
        {
            osiFileName_ = osiFileName;
            osiHeaderName_ = osiFileName + "h";  //.txth is header file

            if (QFileInfo::exists(osiHeaderName_))
            {
                ReadHeader();
            }
            else
            {
                QFileInfo fPath(fInfo.path());

                if (fPath.isDir() && fPath.isWritable())
                {
                    success = CreateHeader(errMsg);
                }
                else
                {
                    success = false;
                    errMsg = "No access right to header file: " + osiHeaderName_ + "!\n";
                }
            }

            if (success)
            {
                isPaused_ = false;
                isRunning_ = true;
                iterStamp_ = stamp2Offset_.begin();

                isConnected_ = true;
                emit Connected(currentDataType_);

                // update slider range in millisecond level
                uint64_t sliderRange = (stamp2Offset_.crbegin()->first) / 1000000;
                emit UpdateSliderRange(sliderRange);

                QtConcurrent::run(this, &OsiReader::SendMessageLoop);
            }
        }
        else
        {
            success = false;
            errMsg = "File: " + osiFileName + " doesn't exist! \n";
        }
    }

    if (!success)
        emit Disconnected(errMsg);
}

void OsiReader::StopReadFile()
{
    if (isConnected_)
    {
        isReadTerminated_ = false;
        isRunning_ = false;
        isPaused_ = false;

        FreeZMQConnection();

        while (!isReadTerminated_)
        {
            QThread::msleep(50);
        }

        QString errMsg = SetupConnection(false);

        isConnected_ = false;
        emit Disconnected(errMsg);
    }
}

void OsiReader::SliderValueChanged(int newValue)
{
    iterMutex_.lock();
    iterChanged_ = true;
    iterMutex_.unlock();
    const std::regex osi_regex("\\.osi");

    uint64_t nanoTimeStamp = (uint64_t)newValue * 1000000;
    for (newIterStamp_ = stamp2Offset_.begin(); newIterStamp_ != stamp2Offset_.end(); ++newIterStamp_)
    {
        if (newIterStamp_->first > nanoTimeStamp)
        {
            break;
        }
    }

    if (newIterStamp_ == stamp2Offset_.end())
        newIterStamp_ = stamp2Offset_.begin();

    if (isPaused_)
    {
        if (std::regex_search(osiFileName_.toStdString().c_str(), osi_regex)){

            typedef unsigned int message_size_t;
            FILE *fd = fopen(osiFileName_.toStdString().c_str(), "r");
            if (fd == nullptr) {
                perror("Open failed");
            }

            fseek(fd, newIterStamp_->second, SEEK_SET);

            int is_ok = 1;
            char *message_buf = nullptr;
            size_t buf_size = 0;
            while (is_ok) {
                message_size_t size = 0;
                int ret = fread(&size, sizeof(message_size_t), 1, fd);
                if (ret == 0) {
                    std::cout << "End of trace" << std::endl;
                    break;
                } else if (ret != 1) {
                    std::cout << "Failed to read the size of the message" << std::endl;
                    is_ok = 0;
                }
                if (is_ok && size > buf_size) {
                    size_t new_size = size * 2;
                    if (realloc_buffer(&message_buf, new_size) < 0) {
                        is_ok = 0;
                        std::cout << "Failed to allocate memory" << std::endl;
                    } else {
                        buf_size = new_size;
                    }
                }
                if (is_ok) {
                    ret = read_bytes(message_buf, size, fd);
                    if (ret < 0) {
                        is_ok = 0;
                        std::cout << "Failed to read the message" << std::endl;
                    }
                }
                if (is_ok) {
                    std::string message_str(message_buf, message_buf + size);

                    if (currentDataType_ == DataType::SensorView) {
                        osi3::SensorView sv;
                        if (sv.ParseFromString(message_str)) {
                            uint64_t curStamp = ::GetTimeStampInNanoSecond<osi3::SensorView>(sv);
                            emit MessageSVSendout(sv);
                            SendOutMessage(message_str);
                            // update slider value in millisecond level
                            uint64_t sliderValue = (curStamp - firstTimeStamp_) / 1000000;
                            emit UpdateSliderValue(sliderValue);
                            break;
                        }
                    } else  // if(currentDataType_ == DataType::SensorData)
                    {
                        osi3::SensorData sd;
                        if (sd.ParseFromString(message_str)) {
                            uint64_t curStamp = ::GetTimeStampInNanoSecond<osi3::SensorData>(sd);
                            emit MessageSDSendout(sd);
                            SendOutMessage(message_str);
                            // update slider value in millisecond level
                            uint64_t sliderValue = (curStamp - firstTimeStamp_) / 1000000;
                            emit UpdateSliderValue(sliderValue);
                            break;
                        }
                    }
                }
            }

        free(message_buf);
        fclose(fd);

        } else {
            std::ifstream inputFile(osiFileName_.toStdString().c_str());
            inputFile.seekg(newIterStamp_->second, std::ios_base::beg);
            int size_found = 0;

            std::string str_line_input;
            std::string str_line;
            std::string str_backup;

            while (getline(inputFile, str_line_input))
            {
                if (!str_backup.empty())
                {
                    str_line = str_backup;
                    str_backup = "";
                }
                if ((size_found = str_line_input.find("$$__$$")) != std::string::npos)
                {
                    str_line += str_line_input.substr(0, size_found);
                    str_backup = str_line_input.substr(size_found + 6) + "\n";
                }
                else
                {
                    str_line += str_line_input + "\n";
                    continue;
                }

                if (currentDataType_ == DataType::SensorView)
                {
                    osi3::SensorView sv;
                    if (sv.ParseFromString(str_line))
                    {
                        uint64_t curStamp = ::GetTimeStampInNanoSecond<osi3::SensorView>(sv);
                        emit MessageSVSendout(sv);
                        SendOutMessage(str_line);
                        // update slider value in millisecond level
                        uint64_t sliderValue = (curStamp - firstTimeStamp_) / 1000000;
                        emit UpdateSliderValue(sliderValue);
                        break;
                    }
                }
                else  // if(currentDataType_ == DataType::SensorData)
                {
                    osi3::SensorData sd;
                    if (sd.ParseFromString(str_line))
                    {
                        uint64_t curStamp = ::GetTimeStampInNanoSecond<osi3::SensorData>(sd);
                        emit MessageSDSendout(sd);
                        SendOutMessage(str_line);
                        // update slider value in millisecond level
                        uint64_t sliderValue = (curStamp - firstTimeStamp_) / 1000000;
                        emit UpdateSliderValue(sliderValue);
                        break;
                    }
                }
            }

            inputFile.close();
        }
    }
}

void OsiReader::ReadHeader()
{
    std::ifstream inputHeader(osiHeaderName_.toStdString().c_str());

    std::string line;
    std::getline(inputHeader, line);

    size_t size = std::strtoll(line.c_str(), nullptr, 0);
    stamp2Offset_.reserve(size);

    char* pEnd;
    while (getline(inputHeader, line))
    {
        uint64_t timeStamp = std::strtoull(line.c_str(), &pEnd, 10);
        std::streamoff offset = std::strtol(pEnd, &pEnd, 10);
        stamp2Offset_.emplace_back(timeStamp, offset);
    }
    inputHeader.close();
}


bool OsiReader::CreateHeader(QString& errorMsg)
{
    bool success(true);
    bool isFirstMsg(true);
    double firstTimeStamp(0);

    const std::regex osi_regex("\\.osi");

    if (std::regex_search(osiFileName_.toStdString().c_str(), osi_regex)){
        FILE *fd = fopen(osiFileName_.toStdString().c_str(), "r");
        if (fd == nullptr) {
            perror("Open failed");
            return -1;
        }
        std::streamoff offset(0);
        std::streampos beginPos = ftell (fd);

        typedef unsigned int message_size_t;

        int is_ok = 1;
        char *message_buf = nullptr;
        size_t buf_size = 0;

        while (is_ok && success) {
            message_size_t size = 0;
            int ret = fread(&size, sizeof(message_size_t), 1, fd);
            if (ret == 0) {
                std::cout << "End of trace" << std::endl;
                break;
            }
            else if (ret != 1) {
                std::cout << "Failed to read the size of the message" << std::endl;
                is_ok = 0;
            }
            if (is_ok && size > buf_size) {
                size_t new_size = size*2;
                if (realloc_buffer(&message_buf, new_size) < 0) {
                    is_ok = 0;
                    std::cout << "Failed to allocate memory" << std::endl;
                } else {
                    buf_size = new_size;
                }
            }
            if (is_ok) {
                ret = read_bytes(message_buf, size, fd);
                if (ret < 0) {
                    is_ok = 0;
                    std::cout << "Failed to read the message" << std::endl;
                }
            }
            if (is_ok) {
                std::string message_str(message_buf, message_buf+size);

                if (currentDataType_ == DataType::SensorView)
                    success = BuildUpStamps<osi3::SensorView>(isFirstMsg, firstTimeStamp, message_str, offset);
                else  // if(currentDataType_ == DataType::SensorData)
                    success = BuildUpStamps<osi3::SensorData>(isFirstMsg, firstTimeStamp, message_str, offset);

                offset = ftell (fd) - beginPos - size - sizeof(message_size_t);
            }
        }

    free(message_buf);
    fclose(fd);
    } else {
        std::ifstream inputFile(osiFileName_.toStdString().c_str(), std::ios::binary);

        std::string str_line_input;
        std::string str_line;
        std::string str_backup;
        size_t size_found;

        std::streampos beginPos = inputFile.tellg();
        std::streamoff offset(0);


        while (getline(inputFile, str_line_input) && success)
        {
            if (!str_backup.empty())
            {
                str_line = str_backup;
                str_backup = "";
            }
            if ((size_found = str_line_input.find("$$__$$")) != std::string::npos)
            {
                str_line += str_line_input.substr(0, size_found);
                str_backup = str_line_input.substr(size_found + 6) + "\n";
            }
            else
            {
                str_line += str_line_input + "\n";
                continue;
            }

            if (currentDataType_ == DataType::SensorView)
                success = BuildUpStamps<osi3::SensorView>(isFirstMsg, firstTimeStamp, str_line, offset);
            else  // if(currentDataType_ == DataType::SensorData)
                success = BuildUpStamps<osi3::SensorData>(isFirstMsg, firstTimeStamp, str_line, offset);

            offset = inputFile.tellg() - beginPos - str_backup.size();
            str_line = "";
        }
    }

    if (stamp2Offset_.empty() || !success)
    {
        success = false;
        errorMsg = "Can not parse OSI::SensorData message!\n Wrong file format!\n";
    }
    else
    {
        SaveHeader();
    }


    return success;
}

template <typename T>
bool OsiReader::BuildUpStamps(bool& isFirstMsg,
                              double& firstTimeStamp,
                              const std::string& message,
                              const std::streamoff& offset)
{
    bool success(true);

    T data;
    if (data.ParseFromString(message))
    {
        if (isFirstMsg)
        {
            isFirstMsg = false;
            firstTimeStamp = GetTimeStampInNanoSecond<T>(data);
        }

        uint64_t timeStamp = GetTimeStampInNanoSecond<T>(data);
        stamp2Offset_.emplace_back(timeStamp - (uint64_t)firstTimeStamp, offset);
    }
    else
    {
        success = false;
    }

    return success;
}

void OsiReader::SaveHeader()
{
    std::ofstream outputFile(osiHeaderName_.toStdString().c_str());

    outputFile << stamp2Offset_.size() << std::endl;

    for (const auto& iter : stamp2Offset_)
    {
        outputFile << iter.first << "\t" << iter.second << std::endl;
    }

    outputFile.close();
}

void OsiReader::SendMessageLoop()
{
    bool isFirstMessage(true);
    std::regex osi_regex("\\.osi");

    if (std::regex_search(osiFileName_.toStdString().c_str(), osi_regex)){
        FILE *fd = fopen(osiFileName_.toStdString().c_str(), "r");
        if (fd == nullptr) {
            perror("Open failed");
        }

        while (isRunning_)
        {
            if (!isPaused_) {
                    fseek(fd, iterStamp_->second, SEEK_SET);
                    uint64_t preTimeStamp(0);
                    bool isRefreshMessage(true);
                    typedef unsigned int message_size_t;

                    std::string str_backup;

                    int is_ok = 1;
                    char *message_buf = nullptr;
                    size_t buf_size = 0;

                    while (is_ok && !isPaused_ && isRunning_) {

                        if (iterChanged_)
                            break;

                        message_size_t size = 0;
                        int ret = fread(&size, sizeof(message_size_t), 1, fd);
                        if (ret == 0) {
                            std::cout << "End of trace" << std::endl;
                            break;
                        } else if (ret != 1) {
                            std::cout << "Failed to read the size of the message" << std::endl;
                            is_ok = 0;
                        }
                        if (is_ok && size > buf_size) {
                            size_t new_size = size * 2;
                            if (realloc_buffer(&message_buf, new_size) < 0) {
                                is_ok = 0;
                                std::cout << "Failed to allocate memory" << std::endl;
                            } else {
                                buf_size = new_size;
                            }
                        }
                        if (is_ok) {
                            ret = read_bytes(message_buf, size, fd);
                            if (ret < 0) {
                                is_ok = 0;
                                std::cout << "Failed to read the message" << std::endl;
                            }
                        }
                        if (is_ok) {
                            std::string message_str(message_buf, message_buf + size);

                            if (!str_backup.empty())
                            {
                                message_str = str_backup;
                                str_backup = "";
                            }

                            bool sendSuccess(true);
                            if (currentDataType_ == DataType::SensorView) {
                                osi3::SensorView sv;
                                sendSuccess = SendMessage<osi3::SensorView>(sv, isFirstMessage, isRefreshMessage,
                                                                            preTimeStamp, message_str);
                                if (sendSuccess)
                                    MessageSVSendout(sv);
                            } else  // if(currentDataType_ == DataType::SensorData)
                            {
                                osi3::SensorData sd;
                                sendSuccess = SendMessage<osi3::SensorData>(sd, isFirstMessage, isRefreshMessage,
                                                                            preTimeStamp, message_str);
                                if (sendSuccess)
                                    MessageSDSendout(sd);
                            }

                            if (!sendSuccess) {
                                // should never come here: error - osi parse wrong
                                isRunning_ = false;
                                isConnected_ = false;
                                emit Disconnected(
                                        tr("Run time error: Can not parse OSI::SensorData message!\n Wrong file format!\n"));
                                break;
                            }
                        }

                    }
                    free(message_buf);

                    if (iterChanged_) {
                        iterStamp_ = newIterStamp_;
                        iterMutex_.lock();
                        iterChanged_ = false;
                        iterMutex_.unlock();
                    }
                } else {
                    QThread::msleep(5);
                }
        }
        fclose(fd);
    }
    else {
        std::cout << "[WARNING]: The $$__$$ separated trace files will be completely removed in the near future and be replaced with the length separated OSI trace files. Please convert them to *.osi files with the converter in the main OSI repository or record the current trace file into the OSI format." << std::endl;
        std::ifstream inputFile(osiFileName_.toStdString().c_str(), std::ios::binary);
        while (isRunning_)
        {
            if (!isPaused_)
            {
                inputFile.clear();
                inputFile.seekg(iterStamp_->second, std::ios_base::beg);

                std::string str_line_input;
                std::string str_line;
                std::string str_backup;
                size_t size_found;

                uint64_t preTimeStamp(0);
                bool isRefreshMessage(true);

                while (getline(inputFile, str_line_input) && !isPaused_ && isRunning_)
                {
                    if (iterChanged_)
                        break;

                    if (!str_backup.empty())
                    {
                        str_line = str_backup;
                        str_backup = "";
                    }
                    if ((size_found = str_line_input.find("$$__$$")) != std::string::npos)
                    {
                        str_line += str_line_input.substr(0, size_found);
                        str_backup = str_line_input.substr(size_found + 6) + "\n";
                    }
                    else
                    {
                        str_line += str_line_input + "\n";
                        continue;
                    }

                    bool sendSuccess(true);
                    if (currentDataType_ == DataType::SensorView)
                    {
                        osi3::SensorView sv;
                        sendSuccess =
                            SendMessage<osi3::SensorView>(sv, isFirstMessage, isRefreshMessage, preTimeStamp, str_line);
                        if (sendSuccess)
                            MessageSVSendout(sv);
                    }
                    else  // if(currentDataType_ == DataType::SensorData)
                    {
                        osi3::SensorData sd;
                        sendSuccess =
                            SendMessage<osi3::SensorData>(sd, isFirstMessage, isRefreshMessage, preTimeStamp, str_line);
                        if (sendSuccess)
                            MessageSDSendout(sd);
                    }

                    if (!sendSuccess)
                    {
                        // should never come here: error - osi parse wrong
                        isRunning_ = false;
                        isConnected_ = false;
                        emit Disconnected(
                            tr("Run time error: Can not parse OSI::SensorData message!\n Wrong file format!\n"));
                        break;
                    }

                    str_line = "";
                }

                str_backup = "";
                str_line = "";

                if (iterChanged_)
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
    
    }
    isReadTerminated_ = true;
}

template <typename T>
bool OsiReader::SendMessage(T& data,
                            bool& isFirstMessage,
                            bool& isRefreshMessage,
                            uint64_t& preTimeStamp,
                            const std::string& message)
{
    bool success(false);

    if (data.ParseFromString(message))
    {
        uint64_t curStamp = GetTimeStampInNanoSecond<T>(data);

        if (isFirstMessage)
        {
            firstTimeStamp_ = curStamp;
            isFirstMessage = false;
        }

        if (isRefreshMessage)
        {
            preTimeStamp = curStamp;
            isRefreshMessage = false;
        }

        int64_t sleep = curStamp - preTimeStamp;
        if (sleep < 0)
            sleep = 0;

        sleep /= 1000;
        sleep += *deltaDelay_ * 1000;

        QThread::usleep(sleep);
        preTimeStamp = curStamp;

        SendOutMessage(message);
        // update slider value in millisecond level
        uint64_t sliderValue = (curStamp - firstTimeStamp_) / 1000000;
        emit UpdateSliderValue(sliderValue);

        ++iterStamp_;
        if (iterStamp_ == stamp2Offset_.end())
            iterStamp_ = stamp2Offset_.begin();

        success = true;
    }

    return success;
}

void OsiReader::SendOutMessage(const std::string& message)
{
    currentBuffer_ = message;
    if (enableSendOut_ && zmqPublisher_.connected())
    {
        zmq::message_t zmqMessage(currentBuffer_.size());
        memcpy(zmqMessage.data(), currentBuffer_.data(), currentBuffer_.size());
        zmqPublisher_.send(zmqMessage);
    }
}

QString OsiReader::SetupConnection(bool enable)
{
    QString errMsg;
    if (enable && enableSendOut_)
    {
        errMsg = SetZMQConnection();
    }

    return errMsg;
}

QString OsiReader::SetZMQConnection()
{
    QString errMsg;
    if (!zmqPublisher_.connected())
    {
        zmqPublisher_ = zmq::socket_t(zmqContext_, socketType_);
        zmqPublisher_.setsockopt(ZMQ_LINGER, 100);
        try
        {
            zmqPublisher_.bind("tcp://127.0.0.1:" + pubPortNumber_);
        }
        catch (zmq::error_t& error)
        {
            zmqPublisher_.close();
            errMsg = "Error connecting to endpoint: " + QString::fromUtf8(error.what());
        }
    }

    return errMsg;
}

QString OsiReader::FreeZMQConnection()
{
    QString errMsg;
    std::unique_ptr<zmq::socket_t> tmp_socket;
    std::unique_ptr<zmq::message_t> tmp_message;
    if (socketType_ == ZMQ_PUSH)
    {
        tmp_socket = std::make_unique<zmq::socket_t>(zmqContext_, ZMQ_PULL);
        tmp_socket->connect("tcp://127.0.0.1:" + pubPortNumber_);
        QThread::msleep(50);
        if (!tmp_socket->connected())
        {
            tmp_message = std::make_unique<zmq::message_t>();
            tmp_socket->recv(tmp_message.get());
        }
    }
    try
    {
        zmqPublisher_.close();
    }
    catch (zmq::error_t& error)
    {
        errMsg = "Error close connecting to endpoint: " + QString::fromUtf8(error.what());
    }

    return errMsg;
}
