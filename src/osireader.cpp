
#include <QtConcurrent/QtConcurrent>
#include <QThread>
#include <QMessageBox>

#include <iostream>

#include "osireader.h"
#include "utils.h"
/**
*  local functions
*/

static void fmuWrapperLogger(jm_callbacks* c, jm_string module, jm_log_level_enu_t log_level, jm_string message)
{
    switch (log_level)
    {
        case jm_log_level_fatal:
            qDebug() << (message);
            break;
        case jm_log_level_error:
        case jm_log_level_warning:
            qDebug() << message;
            break;
        case jm_log_level_nothing:
        case jm_log_level_info:
        case jm_log_level_verbose:
        case jm_log_level_debug:
        case jm_log_level_all:
            qDebug() << message;
            break;
    }
}

static void encode_pointer_to_integer(const void* ptr, fmi2_integer_t& hi, fmi2_integer_t& lo)
{
#if PTRDIFF_MAX == INT64_MAX
    union addrconv
    {
        struct
        {
            int lo;
            int hi;
        } base;
        unsigned long long address;
    } myaddr;
    myaddr.address = reinterpret_cast<unsigned long long>(ptr);
    hi = myaddr.base.hi;
    lo = myaddr.base.lo;
#elif PTRDIFF_MAX == INT32_MAX
    hi = 0;
    lo = reinterpret_cast<int>(ptr);
#else
#error "Cannot determine 32bit or 64bit environment!"
#endif
}


OsiReader::OsiReader(int* deltaDelay,
                     const bool& enableSendOut,
                     const std::string& pubPortNum,
                     const bool& enalbeFMU,
                     int socketType,
                     const std::string& fmuPath)
    : IMessageSource()
    , isRunning_(false)
    , isReadTerminated_(false)

    , osiFileName_()
    , osiHeaderName_()
    , firstTimeStamp_(0)
    , stamp2Offset_()
    , iterStamp_()
    , iterMutex_()
    , iterChanged_(false)
    , newIterStamp_()

    , deltaDelay_(deltaDelay)

    , enableSendOut_(enableSendOut)
    , pubPortNumber_(pubPortNum)

    , socketType_(socketType)
    , zmqContext_(1)
    , zmqPublisher_(zmqContext_, socketType)

    , enableFMU_(enalbeFMU)
    , fmu_(nullptr)
    , callBackFunctions_()
    , callbacks_()
    , fmuContext_(nullptr)
    , jmStatus_()
    , fmiStatus_()
    , tStart_(0)
    , tEnd_(0)
    , tCurrent_(0)
    , hStep_(0)
    , FMUPath_(fmuPath)
    , tmpPath_()
    , logLevel_(LogLevel::Warn)
    , currentBuffer_()
    , vr_()
{
}

void
OsiReader::StartReadFile(const QString& osiFileName, const DataType dataType, const QString& fmuPath)
{
    zmqPublisher_.close();
    currentDataType_ = dataType;
    FMUPath_ = fmuPath.toStdString();
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
                emit Connected(currentDataType_);

                // update slider range in millisecond level
                int sliderRange = (stamp2Offset_.crbegin()->first)/1000000;
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

        FreeFMUConnection();

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

            if(currentDataType_ == DataType::SensorView)
            {
                osi3::SensorView sv;
                if(sv.ParseFromString(str_line))
                {
                    uint64_t curStamp = ::GetTimeStampInNanoSecond<osi3::SensorView>(sv);
                    emit MessageSVSendout(sv);
                    SendOutMessage(str_line);
                    // update slider value in millisecond level
                    int sliderValue = (curStamp - firstTimeStamp_) / 1000000;
                    emit UpdateSliderValue(sliderValue);
                    break;
                }
            }
            else //if(currentDataType_ == DataType::SensorData)
            {
                osi3::SensorData sd;
                if(sd.ParseFromString(str_line))
                {
                    uint64_t curStamp = ::GetTimeStampInNanoSecond<osi3::SensorData>(sd);
                    emit MessageSDSendout(sd);
                    SendOutMessage(str_line);
                    // update slider value in millisecond level
                    int sliderValue = (curStamp - firstTimeStamp_) / 1000000;
                    emit UpdateSliderValue(sliderValue);
                    break;
                }
            }


        }

        inputFile.close();
    }
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

    std::ifstream inputFile (osiFileName_.toStdString().c_str(), std::ios::binary);

    std::string str_line_input;
    std::string str_line;
    std::string str_backup = "";
    size_t size_found;

    std::streampos beginPos = inputFile.tellg();
    std::streamoff offset (0);
    bool isFirstMsg (true);
    double firstTimeStamp (0);

    while(getline (inputFile, str_line_input) && success)
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

        if(currentDataType_ == DataType::SensorView)
            success = BuildUpStamps<osi3::SensorView>(isFirstMsg, firstTimeStamp, str_line, offset);
        else //if(currentDataType_ == DataType::SensorData)
            success = BuildUpStamps<osi3::SensorData>(isFirstMsg, firstTimeStamp, str_line, offset);

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

template <typename T>
bool
OsiReader::BuildUpStamps(bool& isFirstMsg,
                         double& firstTimeStamp,
                         const std::string& message,
                         const std::streamoff& offset)
{
    bool success (true);

    T data;
    if(data.ParseFromString(message))
    {
        if(isFirstMsg)
        {
            isFirstMsg = false;
            firstTimeStamp = GetTimeStampInNanoSecond<T>(data);
        }

        uint64_t timeStamp = GetTimeStampInNanoSecond<T>(data);
        stamp2Offset_.push_back(std::make_pair(timeStamp - (uint64_t)firstTimeStamp, offset));
    }
    else
    {
        success = false;
    }

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
    std::ifstream inputFile (osiFileName_.toStdString().c_str(), std::ios::binary);
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

                bool sendSuccess(true);
                if(currentDataType_ == DataType::SensorView)
                {
                    osi3::SensorView sv;
                    sendSuccess = SendMessage<osi3::SensorView>(sv, isFirstMessage, isRefreshMessage, preTimeStamp, str_line);
                    if(sendSuccess)
                        MessageSVSendout(sv);
                }
                else //if(currentDataType_ == DataType::SensorData)
                {
                    osi3::SensorData sd;
                    sendSuccess = SendMessage<osi3::SensorData>(sd, isFirstMessage, isRefreshMessage, preTimeStamp, str_line);
                    if(sendSuccess)
                        MessageSDSendout(sd);
                }

                if(!sendSuccess)
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

template <typename T>
bool
OsiReader::SendMessage(T& data,
                       bool& isFirstMessage,
                       bool& isRefreshMessage,
                       uint64_t& preTimeStamp,
                       const std::string& message)
{
    bool success (false);

    if(data.ParseFromString(message))
    {
        uint64_t curStamp = GetTimeStampInNanoSecond<T>(data);

        if(isFirstMessage)
        {
            firstTimeStamp_ = curStamp;
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

		QThread::usleep(sleep);
        preTimeStamp = curStamp;

        SendOutMessage(message);
        // update slider value in millisecond level
        int sliderValue = (curStamp - firstTimeStamp_) / 1000000;
        emit UpdateSliderValue(sliderValue);

        ++iterStamp_;
        if(iterStamp_ == stamp2Offset_.end())
            iterStamp_ = stamp2Offset_.begin();

        success = true;
    }

    return success;
}

void
OsiReader::SendOutMessage(const std::string& message)
{
    currentBuffer_ = message;
    if(enableSendOut_ && enableFMU_ && fmu_ != nullptr)
    {
        set_fmi_sensor_data_out();
        fmi2_boolean_t newStep = fmi2_true;
        time_t timeC = time(NULL);
        fmiStatus_ = fmi2_import_do_step(fmu_, (fmi2_real_t)timeC, hStep_, newStep);
    }
    if(enableSendOut_ && zmqPublisher_.connected())
    {
        zmq::message_t zmqMessage(currentBuffer_.size());
        memcpy(zmqMessage.data(), currentBuffer_.data(), currentBuffer_.size());
        zmqPublisher_.send(zmqMessage);
    }
}

QString
OsiReader::SetupConnection(bool enable)
{
    QString errMsg;
    if(enable && enableSendOut_ && enableFMU_)
    {
        errMsg = SetFMUConnection();
    }
    else if(enable && enableSendOut_)
    {
        errMsg = SetZMQConnection();
    }
    else
    {
        if(zmqPublisher_.connected())
        {
            errMsg = FreeZMQConnection();
        }
    }

    return errMsg;
}

QString
OsiReader::SetZMQConnection()
{
    QString errMsg;
    if(!zmqPublisher_.connected())
    {
        zmqPublisher_ = zmq::socket_t(zmqContext_, socketType_);
        zmqPublisher_.setsockopt(ZMQ_LINGER, 100);
        try
        {
            zmqPublisher_.bind("tcp://*:" + pubPortNumber_);
        }
        catch (zmq::error_t& error)
        {
            zmqPublisher_.close();
            errMsg = "Error connecting to endpoint: " + QString::fromUtf8(error.what());
        }
    }

    return errMsg;
}

QString
OsiReader::FreeZMQConnection()
{
    QString errMsg;
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

QString
OsiReader::SetFMUConnection()
{
    QString errMsg;
    if(fmu_ == nullptr)
    {
        if (!FMUPath_.empty())
        {
            tmpPath_ = FMUPath_.substr(0, FMUPath_.find_last_of("/\\"));

            if (!initializeFMUWrapper())
                errMsg = "FMU Wrapper initialization failed!";

            fmu_ = fmi2_import_parse_xml(fmuContext_, tmpPath_.c_str(), 0);

            if (!fmu_ && errMsg.isEmpty())
                errMsg = "Error parsing modeldescription.xml of fmu ";

            if (fmi2_import_get_fmu_kind(fmu_) != fmi2_fmu_kind_cs  && errMsg.isEmpty())
                errMsg = "Only Co-Simulation 2.0 is supported by this code";

            if (!importFMU() && errMsg.isEmpty())
                errMsg = "Could not create the DLL loading mechanism (error: " + QString(fmi2_import_get_last_error(fmu_)) + ").";

            if (!initializeFMU() && errMsg.isEmpty())
                errMsg = "FMU initialization failed:" + QString(fmi2_import_get_last_error(fmu_));
        }
        else
        {
            errMsg = "fmu_path is empty";
        }
    }

    return errMsg;
}

void
OsiReader::FreeFMUConnection()
{
    if(fmu_ != nullptr)
    {
        fmi2_import_terminate(fmu_);
        fmi2_import_free(fmu_);
        fmi_import_free_context(fmuContext_);
        fmu_ = nullptr;
    }
}

bool
OsiReader::initializeFMUWrapper()
{
    // TODO: config or use default values?
    callbacks_.malloc = malloc;
    callbacks_.calloc = calloc;
    callbacks_.realloc = realloc;
    callbacks_.free = free;
    callbacks_.logger = fmuWrapperLogger;
    if (logLevel_ == LogLevel::Warn)
    {
        callbacks_.log_level = jm_log_level_warning;
    }
    else
    {
        callbacks_.log_level = jm_log_level_debug;
    }
    callbacks_.context = 0;

    fmuContext_ = fmi_import_allocate_context(&callbacks_);
    fmi_version_enu_t version = fmi_import_get_fmi_version(fmuContext_, FMUPath_.c_str(), tmpPath_.c_str());
    if (version != fmi_version_2_0_enu)
    {
        qDebug() << "The code only supports version 2.0";
        return false;
    }
    return true;
}

bool
OsiReader::importFMU()
{
    // TODO: config or use default values?
    callBackFunctions_.logger = fmi2_log_forwarding;
    callBackFunctions_.allocateMemory = calloc;
    callBackFunctions_.freeMemory = free;
    callBackFunctions_.componentEnvironment = fmu_;

    jmStatus_ = fmi2_import_create_dllfmu(fmu_, fmi2_fmu_kind_cs, &callBackFunctions_);
    if (jmStatus_ == jm_status_error)
    {
        return false;
    }
    return true;
}

bool
OsiReader::initializeFMU()
{
    // TODO: config or use default values?
    // TODO: alternative solution for start values & verify parameters for setup and initiliazation functions
    // start values
    fmi2_string_t instanceName = "Test CS model instance";
    fmi2_boolean_t visible = fmi2_false;
    fmi2_real_t relativeTol = 1e-4;

    tStart_ = 0;
    tCurrent_ = tStart_;
    fmi2_boolean_t StopTimeDefined = fmi2_false;
    // Do we need it?
    fmi2_string_t fmuGUID;
    fmuGUID = fmi2_import_get_GUID(fmu_);

    jmStatus_ = fmi2_import_instantiate(fmu_, instanceName, fmi2_cosimulation, FMUPath_.c_str(), visible);

    fmi2_value_reference_t vr[2];
    vr[0] = fmi2_import_get_variable_vr(fmi2_import_get_variable_by_name(fmu_, FMI_SENDER_NAME));
    vr[1] = fmi2_import_get_variable_vr(fmi2_import_get_variable_by_name(fmu_, FMI_RECEIVER_NAME));
    fmi2_boolean_t booleanVars_[2];
    booleanVars_[0] = true;  // Sender
    booleanVars_[1] = false; // Receiver
    fmi2_import_set_boolean(fmu_, vr, 2, booleanVars_);

    vr[0] = fmi2_import_get_variable_vr(fmi2_import_get_variable_by_name(fmu_, FMI_ADDRESS_NAME));
    vr[1] = fmi2_import_get_variable_vr(fmi2_import_get_variable_by_name(fmu_, FMI_PORT_NAME));
    fmi2_string_t stringVars_[2];
    stringVars_[0] = "127.0.0.1";
    stringVars_[1] = pubPortNumber_.c_str();
    fmi2_import_set_string(fmu_, vr, 2, stringVars_);

    fmiStatus_ = fmi2_import_setup_experiment(fmu_, fmi2_true, relativeTol, tStart_, StopTimeDefined, tEnd_);
    fmiStatus_ = fmi2_import_enter_initialization_mode(fmu_);
    fmiStatus_ = fmi2_import_exit_initialization_mode(fmu_);

    vr_[FMI_INTEGER_SENSORDATA_IN_BASELO_IDX] = fmi2_import_get_variable_vr(fmi2_import_get_variable_by_name(fmu_, FMI_DATA_IN_BASELO_NAME));
    vr_[FMI_INTEGER_SENSORDATA_IN_BASEHI_IDX] = fmi2_import_get_variable_vr(fmi2_import_get_variable_by_name(fmu_, FMI_DATA_IN_BASEHI_NAME));
    vr_[FMI_INTEGER_SENSORDATA_IN_SIZE_IDX] = fmi2_import_get_variable_vr(fmi2_import_get_variable_by_name(fmu_, FMI_DATA_IN_SIZE_NAME));

    return true;
}

void
OsiReader::set_fmi_sensor_data_out()
{
    fmi2_integer_t integerVars[FMI_INTEGER_IN_VARS];
    encode_pointer_to_integer(currentBuffer_.data(),
                              integerVars[FMI_INTEGER_SENSORDATA_IN_BASEHI_IDX],
                              integerVars[FMI_INTEGER_SENSORDATA_IN_BASELO_IDX]);
    integerVars[FMI_INTEGER_SENSORDATA_IN_SIZE_IDX] = (fmi2_integer_t)currentBuffer_.length();

    fmiStatus_ = fmi2_import_set_integer(fmu_, vr_, FMI_INTEGER_IN_VARS, integerVars);
}



