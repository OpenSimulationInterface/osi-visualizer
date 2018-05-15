#include "fmureceiver.h"



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

static void* decode_integer_to_pointer(fmi2_integer_t hi, fmi2_integer_t lo)
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
    myaddr.base.lo = lo;
    myaddr.base.hi = hi;
    return reinterpret_cast<void*>(myaddr.address);
#elif PTRDIFF_MAX == INT32_MAX
    return reinterpret_cast<void*>(lo);
#else
#error "Cannot determine 32bit or 64bit environment!"
#endif
}

FMUReceiver::FMUReceiver()
    : IMessageSource()
    , isRunning_(false)
    , isThreadTerminated_(false)
    , currentDataType_(DataType::Groundtruth)
{
}

void FMUReceiver::ConnectRequested(const QString &ipAddress, const QString &port, const QString& fmuPath, DataType dataType)
{
    if (!fmuPath.isEmpty())
    {
        logLevel_ = LogLevel::Warn;
        ip_ = ipAddress.toStdString();
        port_ = port.toStdString();
        FMUPath_ = fmuPath.toStdString();
        tmpPath_ = FMUPath_.substr(0, FMUPath_.find_last_of("/\\"));

        // Start initialization
        if (!initializeFMUWrapper())
        {
            QString message = "FMU Wrapper initialization failed!";
            emit Disconnected(message);
            return;
        }

        fmu_ = fmi2_import_parse_xml(context_, tmpPath_.c_str(), 0);

        if (!fmu_)
        {
            QString message = "Error parsing modeldescription.xml of fmu ";
            emit Disconnected(message);
            return;
        }

        if (fmi2_import_get_fmu_kind(fmu_) != fmi2_fmu_kind_cs)
        {
            QString message = "Only Co-Simulation 2.0 is supported by this code";
            emit Disconnected(message);
            return;
        }

        if (!importFMU())
        {
            QString message = "Could not create the DLL loading mechanism (error: " + QString(fmi2_import_get_last_error(fmu_)) + ").";
            emit Disconnected(message);
            return;
        }

        if (!initializeFMU())
        {
            QString message = "FMU initialization failed:" + QString(fmi2_import_get_last_error(fmu_));
            emit Disconnected(message);
            return;
        }
    }
    else
    {
        QString message = "fmu_path is empty";
        emit Disconnected(message);
        return;
    }

    isPaused_ = false;
    isRunning_ = true;
    QtConcurrent::run(this, &FMUReceiver::ReceiveLoop);

    isConnected_ = true;
    emit Connected(currentDataType_);
}

void
FMUReceiver::DisconnectRequested()
{
    if (!isConnected_)
        return;

    isThreadTerminated_ = false;
    isRunning_ = false;

    if(fmu_ != nullptr)
    {
        fmi2_import_terminate(fmu_);
        fmi2_import_free(fmu_);
        fmi_import_free_context(context_);
    }

    while (!isThreadTerminated_)
    {
        QThread::msleep(50);
    }

    isConnected_ = false;
    emit Disconnected();
}

void
FMUReceiver::ReceiveLoop()
{
    while (isRunning_)
    {
        bool msgReceived = false;

        if (!isPaused_)
        {
            fmi2_boolean_t newStep = fmi2_true;
            time_t timeC = time(NULL);
            fmiStatus_ = fmi2_import_do_step(fmu_, (fmi2_real_t)timeC, hStep_, newStep);

            osi::SensorData sd;
            if (get_fmi_sensor_data_in(sd))
            {
                msgReceived = true;
                emit MessageReceived(sd, currentDataType_);
            }
            else
            {
//                qDebug() << "FMU SensorData receiving error";
            }
        }

        if (!msgReceived)
        {
            QThread::msleep(5);
        }
    }

    isThreadTerminated_ = true;
}

bool
FMUReceiver::initializeFMUWrapper()
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

    context_ = fmi_import_allocate_context(&callbacks_);
    fmi_version_enu_t version = fmi_import_get_fmi_version(context_, FMUPath_.c_str(), tmpPath_.c_str());

    if (version != fmi_version_2_0_enu)
    {
        qDebug() << "The code only supports version 2.0";
        return false;
    }
    return true;
}

bool
FMUReceiver::importFMU()
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
FMUReceiver::initializeFMU()
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
    booleanVars_[0] = false; // Sender
    booleanVars_[1] = true;  // Receiver
    fmi2_import_set_boolean(fmu_, vr, 2, booleanVars_);

    vr[0] = fmi2_import_get_variable_vr(fmi2_import_get_variable_by_name(fmu_, FMI_ADDRESS_NAME));
    vr[1] = fmi2_import_get_variable_vr(fmi2_import_get_variable_by_name(fmu_, FMI_PORT_NAME));
    fmi2_string_t stringVars_[2];
    stringVars_[0] = ip_.c_str();
    stringVars_[1] = port_.c_str();
    fmi2_import_set_string(fmu_, vr, 2, stringVars_);

    fmiStatus_ = fmi2_import_setup_experiment(fmu_, fmi2_true, relativeTol, tStart_, StopTimeDefined, tEnd_);
    fmiStatus_ = fmi2_import_enter_initialization_mode(fmu_);
    fmiStatus_ = fmi2_import_exit_initialization_mode(fmu_);

    vr_[FMI_INTEGER_SENSORDATA_OUT_BASELO_IDX] = fmi2_import_get_variable_vr(fmi2_import_get_variable_by_name(fmu_, FMI_DATA_OUT_BASELO_NAME));
    vr_[FMI_INTEGER_SENSORDATA_OUT_BASEHI_IDX] = fmi2_import_get_variable_vr(fmi2_import_get_variable_by_name(fmu_, FMI_DATA_OUT_BASEHI_NAME));
    vr_[FMI_INTEGER_SENSORDATA_OUT_SIZE_IDX] = fmi2_import_get_variable_vr(fmi2_import_get_variable_by_name(fmu_, FMI_DATA_OUT_SIZE_NAME));

    return true;
}

bool
FMUReceiver::get_fmi_sensor_data_in(osi::SensorData& data)
{
    fmi2_integer_t integerVars[FMI_INTEGER_OUT_VARS];
    fmiStatus_ = fmi2_import_get_integer(fmu_, vr_, FMI_INTEGER_OUT_VARS, integerVars);
    if (integerVars[FMI_INTEGER_SENSORDATA_OUT_SIZE_IDX] > 0)
    {
        void* buffer = decode_integer_to_pointer(integerVars[FMI_INTEGER_SENSORDATA_OUT_BASEHI_IDX],
                                                 integerVars[FMI_INTEGER_SENSORDATA_OUT_BASELO_IDX]);

        return data.ParseFromArray(buffer, integerVars[FMI_INTEGER_SENSORDATA_OUT_SIZE_IDX]);
    }

    return false;
}



