#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "customtreewidgetitem.h"
#include "tcpreceiver.h"
#include "fmureceiver.h"
#include "glwidget.h"
#include "global.h"
#include "osiparser.h"
#include "osireader.h"

#include "pythoncomparedialog.h"
#include "displayobjectdialog.h"


#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QColorDialog>
#include <QRadioButton>
#include <QFileDialog>
#include <QLineEdit>
#include <QHostAddress>

#include <sstream>
#include <iomanip>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , config_("appconfig.xml")

    , isSrcConnection_(true)
    , isSrcConnection2_(true)
    , isPlaying_(false)
    , isPlaying2_(false)
    , isConnected_(false)
    , isConnected2_(false)
    , isPlayed_(false)
    , isPlayed2_(false)
    , isSavingOSI_(false)
    , isSavingOSI2_(false)
    , isOverflow_(false)
    , isOverflow2_(false)

    , ui_(new Ui::MainWindow)

    , glWidget_(nullptr)
    , tcpReceiver_(new TCPReceiver())
    , fmuReceiver_(new FMUReceiver())
    , reader_(new OsiReader(&config_.ch1DeltaDelay_, config_.ch1EnableSendOut_, config_.ch1SendOutPortNum_.toStdString(), config_.ch1FMUTxCheck_, config_.ch1LoadFMUTx_.toStdString()))
    , osiparser_(new OsiParser(config_.osiMsgSaveThreshold_))

    , glWidget2_(nullptr)
    , tcpReceiver2_(new TCPReceiver())
    , fmuReceiver2_(new FMUReceiver())
    , reader2_(new OsiReader(&config_.ch2DeltaDelay_, config_.ch2EnableSendOut_, config_.ch2SendOutPortNum_.toStdString(), config_.ch2FMUTxCheck_, config_.ch2LoadFMUTx_.toStdString()))
    , osiparser2_(new OsiParser(config_.osiMsgSaveThreshold_))

    , colorWidgets_()
    , treeNodes_()
    , treeNodes2_()
    , playIcon_(QPixmap(config_.srcPath_ + "Resources/Images/Play.png"))
    , pauseIcon_(QPixmap(config_.srcPath_ + "Resources/Images/Pause.png"))

    , displayObjectDlg_(nullptr)
    , displayObjectDlg2_(nullptr)
{
    ui_->setupUi(this);

    QPixmap logo(config_.srcPath_ + "Resources/Images/BMW_Logo.png");
    ui_->logoLabel->setPixmap(logo);

    InitObjectTree();
    InitObjectTree2();
    ui_->objectTree->header()->setSortIndicator(0, Qt::AscendingOrder);
    ui_->objectTree_2->header()->setSortIndicator(0, Qt::AscendingOrder);

    glWidget_ = new GLWidget(this, tcpReceiver_, treeNodes_, config_);
    ui_->glLayout->addWidget(glWidget_);
    glWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    glWidget2_ = new GLWidget(this, tcpReceiver2_, treeNodes2_, config_);
    ui_->glLayout_2->addWidget(glWidget2_);
    glWidget2_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
    glWidget_->setFormat(format);
    glWidget2_->setFormat(format);

    ui_->saveOSIMessage->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui_->saveOSIMessage->setIcon(playIcon_);
    ui_->saveOSIMessage_2->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui_->saveOSIMessage_2->setIcon(playIcon_);

    ConnectSignalsToSlots();
    InitComboBoxes();
    InitLaneTypeMenu();
    InitLoadConfigure();
    InitLegendGroupBox();
    ToggleSrcGroups();
    ToggleSrcGroups2();
    TogglePlayPauseButton();
    TogglePlayPauseButton2();
    EnableSlider(false);
    EnableSlider2(false);

    QTimer::singleShot(1000, this, SLOT(showMaximized()));
}

void
MainWindow::LocalUpdate()
{
    ShowFOV();
    ShowFOV2();
    ToggleShowFOV();
    ToggleShowFOV2();
}

MainWindow::~MainWindow()
{
    // Call all deletes in reversed order!
    delete glWidget_;
    delete glWidget2_;
    delete osiparser_;
    delete osiparser2_;
    delete tcpReceiver_;
    delete fmuReceiver_;
    delete tcpReceiver2_;
    delete fmuReceiver2_;
    delete reader_;
    delete reader2_;
    delete ui_;
}

void
MainWindow::RBConnection()
{
    isSrcConnection_ = true;
    UpdateGLWidgetMessageSource();
    ToggleSrcGroups();
    ToggleShowFOV();
}

void
MainWindow::RBConnection2()
{
    isSrcConnection2_ = true;
    UpdateGLWidgetMessageSource2();
    ToggleSrcGroups2();
    ToggleShowFOV2();
}

void
MainWindow::RBPlayback()
{
    isSrcConnection_ = false;
    UpdateGLWidgetMessageSource();
    ToggleSrcGroups();
    ToggleShowFOV();
}

void
MainWindow::RBPlayback2()
{
    isSrcConnection2_ = false;
    UpdateGLWidgetMessageSource2();
    ToggleSrcGroups2();
    ToggleShowFOV2();
}

void
MainWindow::CBDataTypeCon(int index)
{
    if(index == 1)
    {
        ui_->showFOV->setEnabled(true);
        glWidget_->UpdateFOVPaint(ui_->showFOV->isChecked());
        EnableShowFOV(!ui_->showFOV->isChecked());
    }
    else
    {
        ui_->showFOV->setEnabled(false);
        glWidget_->UpdateFOVPaint(false);
        EnableShowFOV(false);
    }
}

void
MainWindow::CBDataTypePlay(int index)
{
    if(index == 1)
    {
        ui_->showFOV->setEnabled(true);
        glWidget_->UpdateFOVPaint(ui_->showFOV->isChecked());
        EnableShowFOV(!ui_->showFOV->isChecked());
    }
    else
    {
        ui_->showFOV->setEnabled(false);
        glWidget_->UpdateFOVPaint(false);
        EnableShowFOV(false);
    }
}

void
MainWindow::CBDataTypeCon2(int index)
{
    if(index == 1)
    {
        ui_->showFOV_2->setEnabled(true);
        glWidget2_->UpdateFOVPaint(ui_->showFOV_2->isChecked());
        EnableShowFOV2(!ui_->showFOV_2->isChecked());
    }
    else
    {
        ui_->showFOV_2->setEnabled(false);
        glWidget2_->UpdateFOVPaint(false);
        EnableShowFOV2(false);
    }
}

void
MainWindow::CBDataTypePlay2(int index)
{
    if(index == 1)
    {
        ui_->showFOV_2->setEnabled(true);
        glWidget2_->UpdateFOVPaint(ui_->showFOV_2->isChecked());
        EnableShowFOV2(!ui_->showFOV_2->isChecked());
    }
    else
    {
        ui_->showFOV_2->setEnabled(false);
        glWidget2_->UpdateFOVPaint(false);
        EnableShowFOV2(false);
    }
}

void
MainWindow::ToggleShowFOV()
{
    if(isSrcConnection_)
        CBDataTypeCon(ui_->dataType->currentIndex());
    else
        CBDataTypePlay(ui_->playbackDataType->currentIndex());
}

void
MainWindow::ToggleShowFOV2()
{
    if(isSrcConnection2_)
        CBDataTypeCon2(ui_->dataType_2->currentIndex());
    else
        CBDataTypePlay2(ui_->playbackDataType_2->currentIndex());
}

void
MainWindow::CheckBoxFMURx()
{
    bool enableFmu = ui_->checkBoxFMURx->isChecked();
    ui_->loadFMURx->setEnabled(enableFmu);
    ui_->loadFMURxBrowse->setEnabled(enableFmu);
}

void
MainWindow::CheckBoxFMURx2()
{
    bool enableFmu = ui_->checkBoxFMURx_2->isChecked();
    ui_->loadFMURx_2->setEnabled(enableFmu);
    ui_->loadFMURxBrowse_2->setEnabled(enableFmu);
}

void
MainWindow::LoadFMURxEdited(const QString& text)
{
    ui_->loadFMURx->setText(text);
    ui_->loadFMURx->setToolTip(text);
}

void
MainWindow::LoadFMURxBrowse()
{
    QString fileName = ui_->loadFMURx->text();

    fileName = QFileDialog::getOpenFileName(this,
        tr("Load file"), fileName, tr("FMU Files (*.fmu)"));

    if(!fileName.isEmpty())
    {
        ui_->loadFMURx->setText(fileName);
        ui_->loadFMURx->setToolTip(fileName);
    }
}

void
MainWindow::LoadFMURxEdited2(const QString& text)
{
    ui_->loadFMURx_2->setText(text);
    ui_->loadFMURx_2->setToolTip(text);
}

void
MainWindow::LoadFMURxBrowse2()
{
    QString fileName = ui_->loadFMURx_2->text();

    fileName = QFileDialog::getOpenFileName(this,
        tr("Load file"), fileName, tr("FMU Files (*.fmu)"));

    if(!fileName.isEmpty())
    {
        ui_->loadFMURx_2->setText(fileName);
        ui_->loadFMURx_2->setToolTip(fileName);
    }
}

void
MainWindow::CheckBoxFMUTx()
{
    bool enableFmu = ui_->checkBoxFMUTx->isChecked();
    ui_->loadFMUTx->setEnabled(enableFmu);
    ui_->loadFMUTxBrowse->setEnabled(enableFmu);
}

void
MainWindow::CheckBoxFMUTx2()
{
    bool enableFmu = ui_->checkBoxFMUTx_2->isChecked();
    ui_->loadFMUTx_2->setEnabled(enableFmu);
    ui_->loadFMUTxBrowse_2->setEnabled(enableFmu);
}

void
MainWindow::LoadFMUTxEdited(const QString& text)
{
    ui_->loadFMUTx->setText(text);
    ui_->loadFMUTx->setToolTip(text);
}

void
MainWindow::LoadFMUTxBrowse()
{
    QString fileName = ui_->loadFMUTx->text();

    fileName = QFileDialog::getOpenFileName(this,
        tr("Load file"), fileName, tr("FMU Files (*.fmu)"));

    if(!fileName.isEmpty())
    {
        ui_->loadFMUTx->setText(fileName);
        ui_->loadFMUTx->setToolTip(fileName);
    }
}

void
MainWindow::LoadFMUTxEdited2(const QString& text)
{
    ui_->loadFMUTx_2->setText(text);
    ui_->loadFMUTx_2->setToolTip(text);
}

void
MainWindow::LoadFMUTxBrowse2()
{
    QString fileName = ui_->loadFMUTx_2->text();

    fileName = QFileDialog::getOpenFileName(this,
        tr("Load file"), fileName, tr("FMU Files (*.fmu)"));

    if(!fileName.isEmpty())
    {
        ui_->loadFMUTx_2->setText(fileName);
        ui_->loadFMUTx_2->setToolTip(fileName);
    }
}

void
MainWindow::LoadFileEdited(const QString& text)
{
    ui_->loadFile->setText(text);
    ui_->loadFile->setToolTip(text);
}

void
MainWindow::LoadFileBrowse()
{
    QString fileName = ui_->loadFile->text();

    fileName = QFileDialog::getOpenFileName(this,
        tr("Load file"), fileName, tr("Text Files (*.txt)"));

    if(!fileName.isEmpty())
    {
        ui_->loadFile->setText(fileName);
        ui_->loadFile->setToolTip(fileName);
    }
}

void
MainWindow::LoadFileEdited2(const QString& text)
{
    ui_->loadFile_2->setText(text);
    ui_->loadFile_2->setToolTip(text);
}

void
MainWindow::LoadFileBrowse2()
{
    QString fileName = ui_->loadFile_2->text();

    fileName = QFileDialog::getOpenFileName(this,
        tr("Load file"), fileName, tr("Text Files (*.txt)"));

    if(!fileName.isEmpty())
    {
        ui_->loadFile_2->setText(fileName);
        ui_->loadFile_2->setToolTip(fileName);
    }
}

void
MainWindow::EnableSendToNetwork()
{
    bool enableSend = ui_->enableSendToNetwork->checkState() == Qt::Checked;
    ui_->sendOutPortNum->setEnabled(enableSend);
    bool enableFMU = ui_->checkBoxFMUTx->checkState() == Qt::Checked;
    ui_->checkBoxFMUTx->setEnabled(enableSend);
    ui_->loadFMUTx->setEnabled(enableSend && enableFMU);
    ui_->loadFMUTxBrowse->setEnabled(enableSend && enableFMU);
}

void
MainWindow::EnableSendToNetwork2()
{
    bool enableSend = ui_->enableSendToNetwork_2->checkState() == Qt::Checked;
    ui_->sendOutPortNum_2->setEnabled(enableSend);
    bool enableFMU = ui_->checkBoxFMUTx_2->checkState() == Qt::Checked;
    ui_->checkBoxFMUTx_2->setEnabled(enableSend);
    ui_->loadFMUTx_2->setEnabled(enableSend && enableFMU);
    ui_->loadFMUTxBrowse_2->setEnabled(enableSend && enableFMU);
}

void
MainWindow::UpdateCombineChannelMenu()
{
    ui_->actionCombiCh->setEnabled(isPlaying_ == isPlaying2_);
}

void
MainWindow::PlayPauseButtonClicked()
{
    LocalPlayPause();
    if(config_.combineChannel_)
        LocalPlayPause2();

    UpdateCombineChannelMenu();
}

void
MainWindow::LocalPlayPause()
{
    if(CheckFieldsValidity() == false)
        return;

    bool configUpdated = UpdateConfigure();

    if(isConnected_ == true && isSrcConnection_ == false)
        configUpdated = true;
    else if(isPlayed_ == true && isSrcConnection_ == true)
        configUpdated = true;

    // Initial state
    if( isPlaying_ == false &&
        isConnected_ == false &&
        isPlayed_ == false )
    {
        Play();
    }
    else
    {
        if(isPlaying_)
        {
            TogglePause();
        }
        else
        {
            if(configUpdated)
            {
                Stop();
                Play();

                ui_->hSlider->blockSignals(true);
                ui_->hSlider->setValue(0);
                ui_->hSlider->blockSignals(false);
            }
            else
            {
                TogglePause();
            }
        }
    }

    UpdateFields();
}

void
MainWindow::PlayPauseButtonClicked2()
{
    LocalPlayPause2();
    if(config_.combineChannel_)
        LocalPlayPause();

    UpdateCombineChannelMenu();
}

void
MainWindow::LocalPlayPause2()
{
    if(CheckFieldsValidity2() == false)
        return;

    bool configUpdated = UpdateConfigure2();

    if(isConnected2_ == true && isSrcConnection2_ == false)
        configUpdated = true;
    else if(isPlayed2_ == true && isSrcConnection2_ == true)
        configUpdated = true;

    // Initial state
    if( isPlaying2_ == false &&
        isConnected2_ == false &&
        isPlayed2_ == false )
    {
        Play2();
    }
    else
    {
        if(isPlaying2_)
        {
            TogglePause2();
        }
        else
        {
            if(configUpdated)
            {
                Stop2();
                Play2();

                ui_->hSlider_2->blockSignals(true);
                ui_->hSlider_2->setValue(0);
                ui_->hSlider_2->blockSignals(false);
            }
            else
            {
                TogglePause2();
            }
        }
    }

    UpdateFields2();
}

bool
MainWindow::CheckFieldsValidity()
{
    bool success (true);
    QString errMsg;

    if(isSrcConnection_)
    {
        // IP address
        QHostAddress ipAddress;
        if(ipAddress.setAddress(ui_->ipAddress->text()) == false)
        {
            errMsg += "  Invalid IP address!\n";
        }

        // Port number
        ui_->portNumber->text().toUInt(&success);
        if(success == false)
            errMsg += "  Port number should be valid integer!\n";

        // Rx FMU file
        if(ui_->checkBoxFMURx->isChecked() && QFileInfo::exists(ui_->loadFMURx->text()) == false)
        {
            success = false;
            errMsg += "  Connection FMU file doesn't exist!\n";
        }
    }
    else
    {
        // Load file
        if(QFileInfo::exists(ui_->loadFile->text()) == false)
        {
            success = false;
            errMsg += "  Load file doesn't exist!\n";
        }

        // Delta delay
        if(ui_->deltaDelay->text().isEmpty() == false)
        {
            ui_->deltaDelay->text().toUInt(&success);
            if(success == false)
                errMsg += "  Port number should be valid positive integer!\n";
        }

        // Tx FMU file
        if( ui_->enableSendToNetwork->isChecked() &&
            ui_->checkBoxFMUTx->isChecked() &&
            QFileInfo::exists(ui_->loadFMUTx->text()) == false )
        {
            success = false;
            errMsg += "  Send FMU file doesn't exist!\n";
        }
    }

    if(errMsg.isEmpty() == false)
    {
        success = false;
        ShowErrorMessage("Channel 1: \n" + errMsg);
    }

    return success;
}

bool
MainWindow::CheckFieldsValidity2()
{
    bool success (true);
    QString errMsg;

    if(isSrcConnection2_)
    {
        // IP address
        QHostAddress ipAddress;
        if(ipAddress.setAddress(ui_->ipAddress_2->text()) == false)
        {
            errMsg += "  Invalid IP address!\n";
        }

        // Port number
        ui_->portNumber_2->text().toUInt(&success);
        if(success == false)
            errMsg += "  Port number should be valid integer!\n";

        // Rx FMU file
        if(ui_->checkBoxFMURx_2->isChecked() && QFileInfo::exists(ui_->loadFMURx_2->text()) == false)
        {
            success = false;
            errMsg += "  Connection FMU file doesn't exist!\n";
        }
    }
    else
    {
        // Load file
        if(QFileInfo::exists(ui_->loadFile_2->text()) == false)
        {
            success = false;
            errMsg += "  Load file doesn't exist!\n";
        }

        // Delta delay
        if(ui_->deltaDelay_2->text().isEmpty() == false)
        {
            ui_->deltaDelay_2->text().toUInt(&success);
            if(success == false)
                errMsg += "  Port number should be valid positive integer!\n";
        }

        // Tx FMU file
        if( ui_->enableSendToNetwork_2->isChecked() &&
            ui_->checkBoxFMUTx_2->isChecked() &&
            QFileInfo::exists(ui_->loadFMUTx_2->text()) == false )
        {
            success = false;
            errMsg += "  Send FMU file doesn't exist!\n";
        }
    }

    if(errMsg.isEmpty() == false)
    {
        success = false;
        ShowErrorMessage("Channel 2: \n" + errMsg);
    }

    return success;
}

void
MainWindow::ShowErrorMessage(const QString& errMsg)
{
    QMessageBox messageBox;
    messageBox.critical(this, "Error", errMsg);
    messageBox.setFixedSize(200, 100);
    qDebug() << errMsg;
}

void
MainWindow::ConnectSignalsToSlots()
{
    // connection -- playback selection
    connect(ui_->rbConnection,   &QRadioButton::clicked, this, &MainWindow::RBConnection);
    connect(ui_->rbPlayback,     &QRadioButton::clicked, this, &MainWindow::RBPlayback);
    connect(ui_->rbConnection_2, &QRadioButton::clicked, this, &MainWindow::RBConnection2);
    connect(ui_->rbPlayback_2,   &QRadioButton::clicked, this, &MainWindow::RBPlayback2);

    // fmu rx
    connect(ui_->checkBoxFMURx,     &QCheckBox::clicked,    this, &MainWindow::CheckBoxFMURx);
    connect(ui_->checkBoxFMURx_2,   &QCheckBox::clicked,    this, &MainWindow::CheckBoxFMURx2);
    connect(ui_->loadFMURx,         &QLineEdit::textEdited, this, &MainWindow::LoadFMURxEdited);
    connect(ui_->loadFMURxBrowse,   &QPushButton::clicked,  this, &MainWindow::LoadFMURxBrowse);
    connect(ui_->loadFMURx_2,       &QLineEdit::textEdited, this, &MainWindow::LoadFMURxEdited2);
    connect(ui_->loadFMURxBrowse_2, &QPushButton::clicked,  this, &MainWindow::LoadFMURxBrowse2);


    // fmu rx
    connect(ui_->checkBoxFMUTx,     &QCheckBox::clicked,    this, &MainWindow::CheckBoxFMUTx);
    connect(ui_->checkBoxFMUTx_2,   &QCheckBox::clicked,    this, &MainWindow::CheckBoxFMUTx2);
    connect(ui_->loadFMUTx,         &QLineEdit::textEdited, this, &MainWindow::LoadFMUTxEdited);
    connect(ui_->loadFMUTxBrowse,   &QPushButton::clicked,  this, &MainWindow::LoadFMUTxBrowse);
    connect(ui_->loadFMUTx_2,       &QLineEdit::textEdited, this, &MainWindow::LoadFMUTxEdited2);
    connect(ui_->loadFMUTxBrowse_2, &QPushButton::clicked,  this, &MainWindow::LoadFMUTxBrowse2);

    // load/save playback file
    connect(ui_->loadFile,         &QLineEdit::textEdited, this, &MainWindow::LoadFileEdited);
    connect(ui_->loadFileBrowse,   &QPushButton::clicked, this, &MainWindow::LoadFileBrowse);
    connect(ui_->loadFile_2,       &QLineEdit::textEdited, this, &MainWindow::LoadFileEdited2);
    connect(ui_->loadFileBrowse_2, &QPushButton::clicked, this, &MainWindow::LoadFileBrowse2);

    // enable sent out
    connect(ui_->enableSendToNetwork,   &QCheckBox::clicked, this, &MainWindow::EnableSendToNetwork);
    connect(ui_->enableSendToNetwork_2, &QCheckBox::clicked, this, &MainWindow::EnableSendToNetwork2);

    // Play/Pause
    connect(ui_->playPauseButton,   &QToolButton::clicked, this, &MainWindow::PlayPauseButtonClicked);
    connect(ui_->playPauseButton_2, &QToolButton::clicked, this, &MainWindow::PlayPauseButtonClicked2);

    // signals and slots related to connection status
    // If not queued, this signal will block the GUI. Same applies for Connect/Disconnect in TCPReceiver.
    connect(this, &MainWindow::ConnectRequested, tcpReceiver_, &TCPReceiver::ConnectRequested, Qt::QueuedConnection);
    connect(tcpReceiver_, &TCPReceiver::Connected, this, &MainWindow::Connected);
    connect(tcpReceiver_, &TCPReceiver::Disconnected, this, &MainWindow::Disconnected);
    connect(tcpReceiver_, &TCPReceiver::Connected, glWidget_, &GLWidget::Connected, Qt::DirectConnection);
    connect(tcpReceiver_, &TCPReceiver::Disconnected, glWidget_, &GLWidget::Disconnected, Qt::DirectConnection);

    connect(this, &MainWindow::ConnectRequested2, tcpReceiver2_, &TCPReceiver::ConnectRequested, Qt::QueuedConnection);
    connect(tcpReceiver2_, &TCPReceiver::Connected, this, &MainWindow::Connected2);
    connect(tcpReceiver2_, &TCPReceiver::Disconnected, this, &MainWindow::Disconnected2);
    connect(tcpReceiver2_, &TCPReceiver::Connected, glWidget2_, &GLWidget::Connected, Qt::DirectConnection);
    connect(tcpReceiver2_, &TCPReceiver::Disconnected, glWidget2_, &GLWidget::Disconnected, Qt::DirectConnection);

    connect(this, &MainWindow::FMUConnectRequested, fmuReceiver_, &FMUReceiver::ConnectRequested, Qt::QueuedConnection);
    connect(fmuReceiver_, &FMUReceiver::Connected, this, &MainWindow::Connected);
    connect(fmuReceiver_, &FMUReceiver::Disconnected, this, &MainWindow::Disconnected);
    connect(fmuReceiver_, &FMUReceiver::Connected, glWidget_, &GLWidget::Connected, Qt::DirectConnection);
    connect(fmuReceiver_, &FMUReceiver::Disconnected, glWidget_, &GLWidget::Disconnected, Qt::DirectConnection);

    connect(this, &MainWindow::FMUConnectRequested2, fmuReceiver2_, &FMUReceiver::ConnectRequested, Qt::QueuedConnection);
    connect(fmuReceiver2_, &FMUReceiver::Connected, this, &MainWindow::Connected2);
    connect(fmuReceiver2_, &FMUReceiver::Disconnected, this, &MainWindow::Disconnected2);
    connect(fmuReceiver2_, &FMUReceiver::Connected, glWidget2_, &GLWidget::Connected, Qt::DirectConnection);
    connect(fmuReceiver2_, &FMUReceiver::Disconnected, glWidget2_, &GLWidget::Disconnected, Qt::DirectConnection);

    connect(this, &MainWindow::StartPlaybackRequested, reader_, &OsiReader::StartReadFile, Qt::QueuedConnection);
    connect(reader_, &OsiReader::Connected, this, &MainWindow::Connected);
    connect(reader_, &OsiReader::Disconnected, this, &MainWindow::Disconnected);
    connect(reader_, &OsiReader::Connected, glWidget_, &GLWidget::Connected, Qt::DirectConnection);
    connect(reader_, &OsiReader::Disconnected, glWidget_, &GLWidget::Disconnected, Qt::DirectConnection);

    connect(reader_, &OsiReader::UpdateSliderRange, this, &MainWindow::UpdateSliderRange);
    connect(reader_, &OsiReader::UpdateSliderValue, this, &MainWindow::UpdateSliderValue);

    connect(this, &MainWindow::StartPlaybackRequested2, reader2_, &OsiReader::StartReadFile, Qt::QueuedConnection);
    connect(reader2_, &OsiReader::Connected, this, &MainWindow::Connected2);
    connect(reader2_, &OsiReader::Disconnected, this, &MainWindow::Disconnected2);
    connect(reader2_, &OsiReader::Connected, glWidget2_, &GLWidget::Connected, Qt::DirectConnection);
    connect(reader2_, &OsiReader::Disconnected, glWidget2_, &GLWidget::Disconnected, Qt::DirectConnection);

    connect(reader2_, &OsiReader::UpdateSliderRange, this, &MainWindow::UpdateSliderRange2);
    connect(reader2_, &OsiReader::UpdateSliderValue, this, &MainWindow::UpdateSliderValue2);

    //Main loop: receive(read) -> parse -> update objects
    connect(tcpReceiver_, &TCPReceiver::MessageReceived, osiparser_, &OsiParser::ParseReceivedMessage, Qt::QueuedConnection);
    connect(tcpReceiver2_, &TCPReceiver::MessageReceived, osiparser2_, &OsiParser::ParseReceivedMessage, Qt::QueuedConnection);

    connect(fmuReceiver_, &FMUReceiver::MessageReceived, osiparser_, &OsiParser::ParseReceivedMessage, Qt::QueuedConnection);
    connect(fmuReceiver2_, &FMUReceiver::MessageReceived, osiparser2_, &OsiParser::ParseReceivedMessage, Qt::QueuedConnection);

    connect(reader_, &OsiReader::MessageSendout, osiparser_, &OsiParser::ParseReceivedMessage, Qt::QueuedConnection);
    connect(reader2_, &OsiReader::MessageSendout, osiparser2_, &OsiParser::ParseReceivedMessage, Qt::QueuedConnection);

    connect(osiparser_, &OsiParser::MessageParsed, glWidget_, &GLWidget::MessageParsed, Qt::QueuedConnection);
    connect(osiparser2_, &OsiParser::MessageParsed, glWidget2_, &GLWidget::MessageParsed, Qt::QueuedConnection);

    //Export Osi message
    connect(osiparser_, &OsiParser::EnableExport, this, &MainWindow::EnableExport);
    connect(osiparser_, &OsiParser::SaveOSIMsgOverflow, this, &MainWindow::SaveOSIMsgOverflow);
    connect(ui_->saveOSIMessage, &QToolButton::clicked, this, &MainWindow::ToggleSaveOSIMessage);
    connect(ui_->saveOSIMessage, &QToolButton::clicked, osiparser_, &OsiParser::ExportOsiMessage);

    connect(osiparser2_, &OsiParser::EnableExport, this, &MainWindow::EnableExport2);
    connect(osiparser2_, &OsiParser::SaveOSIMsgOverflow, this, &MainWindow::SaveOSIMsgOverflow2);
    connect(ui_->saveOSIMessage_2, &QToolButton::clicked, this, &MainWindow::ToggleSaveOSIMessage2);
    connect(ui_->saveOSIMessage_2, &QToolButton::clicked, osiparser2_, &OsiParser::ExportOsiMessage);

    //items tree
    connect(ui_->objectTree, &QTreeWidget::itemClicked, glWidget_, &GLWidget::TreeItemClicked);
    connect(ui_->objectTree, &QTreeWidget::itemChanged, glWidget_, &GLWidget::TreeItemChanged);
    connect(ui_->objectTree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::ShowContextMenu);
    connect(ui_->objectTree, &QTreeWidget::itemClicked, this, &MainWindow::ConnectDisplayObjectInformation);

    connect(ui_->objectTree_2, &QTreeWidget::itemClicked, glWidget2_, &GLWidget::TreeItemClicked);
    connect(ui_->objectTree_2, &QTreeWidget::itemChanged, glWidget2_, &GLWidget::TreeItemChanged);
    connect(ui_->objectTree_2, &QTreeWidget::itemClicked, this, &MainWindow::ConnectDisplayObjectInformation2);

    // slider
    connect(ui_->hSlider, &QSlider::valueChanged, reader_, &OsiReader::SliderValueChanged);
    connect(ui_->hSlider_2, &QSlider::valueChanged, reader2_, &OsiReader::SliderValueChanged);

    //Tracking
    connect(glWidget_, &GLWidget::SetTrackingEnabled, this, &MainWindow::SetTrackingEnabled);
    connect(ui_->startStopTrackingButton, &QPushButton::clicked, glWidget_, &GLWidget::StartTracking);

    connect(glWidget2_, &GLWidget::SetTrackingEnabled, this, &MainWindow::SetTrackingEnabled2);
    connect(ui_->startStopTrackingButton_2, &QPushButton::clicked, glWidget2_, &GLWidget::StartTracking);

    // menu play
    connect(ui_->actionPlay, &QAction::triggered, this, &MainWindow::PlayPauseButtonClicked);

    // menu python compare
    connect(ui_->actionPyCompare, &QAction::triggered, this, &MainWindow::PythonCompare);

    // menu combine channels
    connect(ui_->actionCombiCh, &QAction::triggered, this, &MainWindow::CombineChannels);
    // menu show Grid
    connect(ui_->actionShowGrid, &QAction::toggled, this, &MainWindow::ShowGridCheckBoxToggled);
    connect(this, &MainWindow::UpdateGrid, glWidget_, &GLWidget::UpdateGrid);
    connect(this, &MainWindow::UpdateGrid, glWidget2_, &GLWidget::UpdateGrid);

    // menu show object overview
    connect(ui_->actionShowObject, &QAction::toggled, this, &MainWindow::ShowObjectOverview);

    // menu camera operations
    connect(ui_->actionLockCamera, &QAction::toggled, this, &MainWindow::LockCheckBoxToggled);
    connect(ui_->actionResetCameraM, &QAction::triggered, glWidget_, &GLWidget::ResetCameraAll);
    connect(ui_->actionResetCameraM, &QAction::triggered, glWidget2_, &GLWidget::ResetCameraAll);
    connect(ui_->actionResetCamera, &QAction::triggered, glWidget_, &GLWidget::ResetCameraOrient);
    connect(ui_->actionResetCamera, &QAction::triggered, glWidget2_, &GLWidget::ResetCameraOrient);

    // menu quit
    connect(ui_->actionQuit, &QAction::triggered, this, &MainWindow::Quit);

    // FOV connection
    connect(ui_->showFOV, &QToolButton::clicked, this, &MainWindow::ShowFOV);
    connect(ui_->showFOV_2, &QToolButton::clicked, this, &MainWindow::ShowFOV2);
    connect(ui_->dataType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::CBDataTypeCon);
    connect(ui_->playbackDataType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::CBDataTypePlay);
    connect(ui_->dataType_2, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::CBDataTypeCon2);
    connect(ui_->playbackDataType_2, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::CBDataTypePlay2);

}

void
MainWindow::EnableSrcRadioButton(bool enable)
{
    ui_->rbConnection->setEnabled(enable);
    ui_->rbPlayback->setEnabled(enable);
}

void
MainWindow::EnableSrcRadioButton2(bool enable)
{
    ui_->rbConnection_2->setEnabled(enable);
    ui_->rbPlayback_2->setEnabled(enable);
}

void
MainWindow::ToggleSrcGroups()
{
    ui_->rbConnection->setChecked(isSrcConnection_);
    ui_->rbPlayback->setChecked(!isSrcConnection_);

    EnableConnectionGroup1(isSrcConnection_);
    EnablePlaybackGroup1(!isSrcConnection_);
}

void
MainWindow::ToggleSrcGroups2()
{
    ui_->rbConnection_2->setChecked(isSrcConnection2_);
    ui_->rbPlayback_2->setChecked(!isSrcConnection2_);

    EnableConnectionGroup2(isSrcConnection2_);
    EnablePlaybackGroup2(!isSrcConnection2_);
}

void
MainWindow::EnableSrcGroups(bool enable)
{
    EnableSrcRadioButton(enable);

    EnableConnectionGroup1(enable);
    EnablePlaybackGroup1(enable);
}

void
MainWindow::EnableSrcGroups2(bool enable)
{
    EnableSrcRadioButton2(enable);

    EnableConnectionGroup2(enable);
    EnablePlaybackGroup2(enable);
}

void
MainWindow::EnableConnectionGroup1(bool enable)
{
    ui_->checkBoxFMURx->setEnabled(enable);
    if(enable)
    {
        bool enableFmu = ui_->checkBoxFMURx->isChecked();
        ui_->loadFMURx->setEnabled(enableFmu);
        ui_->loadFMURxBrowse->setEnabled(enableFmu);
    }
    else
    {
        ui_->loadFMURx->setEnabled(false);
        ui_->loadFMURxBrowse->setEnabled(false);
    }

    ui_->ipAddress->setEnabled(enable);
    ui_->portNumber->setEnabled(enable);
    ui_->dataType->setEnabled(enable);
}

void
MainWindow::EnableConnectionGroup2(bool enable)
{
    ui_->checkBoxFMURx_2->setEnabled(enable);
    if(enable)
    {
        bool enableFmu = ui_->checkBoxFMURx_2->isChecked();
        ui_->loadFMURx_2->setEnabled(enableFmu);
        ui_->loadFMURxBrowse_2->setEnabled(enableFmu);
    }
    else
    {
        ui_->loadFMURx_2->setEnabled(false);
        ui_->loadFMURxBrowse_2->setEnabled(false);
    }

    ui_->ipAddress_2->setEnabled(enable);
    ui_->portNumber_2->setEnabled(enable);
    ui_->dataType_2->setEnabled(enable);
}

void
MainWindow::EnablePlaybackGroup1(bool enable)
{
    ui_->loadFile->setEnabled(enable);
    ui_->loadFileBrowse->setEnabled(enable);
    ui_->playbackDataType->setEnabled(enable);
    ui_->deltaDelay->setEnabled(enable);
    ui_->enableSendToNetwork->setEnabled(enable);
    bool enableSend = ui_->enableSendToNetwork->checkState() == Qt::Checked;
    ui_->sendOutPortNum->setEnabled(enable && enableSend);
    ui_->checkBoxFMUTx->setEnabled(enable && enableSend);
    bool enableFMU = ui_->checkBoxFMUTx->checkState() == Qt::Checked;
    ui_->loadFMUTx->setEnabled(enable && enableSend && enableFMU);
    ui_->loadFMUTxBrowse->setEnabled(enable && enableSend && enableFMU);
}

void
MainWindow::EnablePlaybackGroup2(bool enable)
{
    ui_->loadFile_2->setEnabled(enable);
    ui_->loadFileBrowse_2->setEnabled(enable);
    ui_->playbackDataType_2->setEnabled(enable);
    ui_->deltaDelay_2->setEnabled(enable);
    ui_->enableSendToNetwork_2->setEnabled(enable);
    bool enableSend = ui_->enableSendToNetwork_2->checkState() == Qt::Checked;
    ui_->sendOutPortNum_2->setEnabled(enable && enableSend);
    ui_->checkBoxFMUTx_2->setEnabled(enable && enableSend);
    bool enableFMU = ui_->checkBoxFMUTx_2->checkState() == Qt::Checked;
    ui_->loadFMUTx_2->setEnabled(enable && enableSend && enableFMU);
    ui_->loadFMUTxBrowse_2->setEnabled(enable && enableSend && enableFMU);
}

void
MainWindow::EnableSlider(bool enable)
{
    ui_->hSlider->setEnabled(enable);
}

void
MainWindow::EnableSlider2(bool enable)
{
    ui_->hSlider_2->setEnabled(enable);
}

void
MainWindow::TogglePlayPauseButton()
{
    if(isPlaying_)
    {
        ui_->playPauseButton->setIcon(pauseIcon_);
    }
    else
    {
        ui_->playPauseButton->setIcon(playIcon_);
    }
}

void
MainWindow::TogglePlayPauseButton2()
{
    if(isPlaying2_)
    {
        ui_->playPauseButton_2->setIcon(pauseIcon_);
    }
    else
    {
        ui_->playPauseButton_2->setIcon(playIcon_);
    }
}

void
MainWindow::CancelSaveOSIMessage()
{
    isOverflow_ = false;

    osiparser_->CancelSaveOSIMessage();
    isSavingOSI_ = false;
    ui_->saveOSIMessage->setIcon(playIcon_);
}

void
MainWindow::CancelSaveOSIMessage2()
{
    isOverflow2_ = false;

    osiparser2_->CancelSaveOSIMessage();
    isSavingOSI2_ = false;
    ui_->saveOSIMessage_2->setIcon(playIcon_);
}

void
MainWindow::UpdateGLWidgetMessageSource()
{
    if(isSrcConnection_)
    {
        if(ui_->checkBoxFMURx->isChecked())
            glWidget_->UpdateIMessageSource(fmuReceiver_);
        else
            glWidget_->UpdateIMessageSource(tcpReceiver_);
    }
    else
    {
        glWidget_->UpdateIMessageSource(reader_);
    }
}

void
MainWindow::UpdateGLWidgetMessageSource2()
{
    if(isSrcConnection2_)
    {
        if(ui_->checkBoxFMURx_2->isChecked())
            glWidget2_->UpdateIMessageSource(fmuReceiver2_);
        else
            glWidget2_->UpdateIMessageSource(tcpReceiver2_);
    }
    else
    {
        glWidget2_->UpdateIMessageSource(reader2_);
    }
}

void
MainWindow::UpdateFields()
{
    if(isPlaying_)
    {
        EnableSrcGroups(false);
        EnableSrcRadioButton(false);
        EnableSlider(!isSrcConnection_);
    }
    else
    {
        ToggleSrcGroups();
        EnableSrcRadioButton(true);
        EnableSlider(!isSrcConnection_ && isPlayed_);
        CancelSaveOSIMessage();
    }

    TogglePlayPauseButton();
}

void
MainWindow::UpdateFields2()
{
    if(isPlaying2_)
    {
        EnableSrcGroups2(false);
        EnableSrcRadioButton2(false);
        EnableSlider2(!isSrcConnection2_);
    }
    else
    {
        ToggleSrcGroups2();
        EnableSrcRadioButton2(true);
        EnableSlider2(!isSrcConnection2_ && isPlayed2_);
        CancelSaveOSIMessage2();
    }

    TogglePlayPauseButton2();
}

bool
MainWindow::UpdateConfigure()
{
    bool hasChange (false);
    bool saveChange (false);

    if(config_.ch1IPAddress_ != ui_->ipAddress->text())
    {
        hasChange = true;
        config_.ch1IPAddress_ = ui_->ipAddress->text();
    }

    if(config_.ch1PortNum_ != ui_->portNumber->text())
    {
        hasChange = true;
        config_.ch1PortNum_ = ui_->portNumber->text();
    }

    if(config_.ch1DataType_ != (DataType)ui_->dataType->currentIndex())
    {
        hasChange = true;
        config_.ch1DataType_ = (DataType)ui_->dataType->currentIndex();
    }

    if(config_.ch1FMURxCheck_ != (ui_->checkBoxFMURx->checkState() == Qt::Checked))
    {
        saveChange = true;
        config_.ch1FMURxCheck_ = (ui_->checkBoxFMURx->checkState() == Qt::Checked);
    }

    if(config_.ch1LoadFMURx_ != ui_->loadFMURx->text())
    {
        hasChange = true;
        config_.ch1LoadFMURx_ = ui_->loadFMURx->text();
    }

    if(config_.ch1LoadFile_ != ui_->loadFile->text())
    {
        hasChange = true;
        config_.ch1LoadFile_ = ui_->loadFile->text();
    }

    if(config_.ch1PlaybackDataType_ != (DataType)ui_->playbackDataType->currentIndex())
    {
        hasChange = true;
        config_.ch1PlaybackDataType_ = (DataType)ui_->playbackDataType->currentIndex();
    }

    if(config_.ch1DeltaDelay_ != ui_->deltaDelay->text().toInt())
    {
        saveChange = true;
        config_.ch1DeltaDelay_ = ui_->deltaDelay->text().toInt();
    }

    if(config_.ch1EnableSendOut_ != (ui_->enableSendToNetwork->checkState() == Qt::Checked))
    {
        saveChange = true;
        config_.ch1EnableSendOut_ = (ui_->enableSendToNetwork->checkState() == Qt::Checked);
    }

    if(config_.ch1SendOutPortNum_ != ui_->sendOutPortNum->text())
    {
        saveChange = true;
        config_.ch1SendOutPortNum_ = ui_->sendOutPortNum->text();
        reader_->SetSendOutPortNum(config_.ch1SendOutPortNum_.toStdString());
    }

    if(config_.ch1FMUTxCheck_ != (ui_->checkBoxFMUTx->checkState() == Qt::Checked))
    {
        hasChange = true;
        config_.ch1FMUTxCheck_ = (ui_->checkBoxFMUTx->checkState() == Qt::Checked);
    }

    if(config_.ch1LoadFMUTx_ != ui_->loadFMUTx->text())
    {
        hasChange = true;
        config_.ch1LoadFMUTx_ = ui_->loadFMUTx->text();
//        reader_->SetSendOutFMUPath(config_.ch1LoadFMUTx_.toStdString());
    }

    if(hasChange || saveChange)
        config_.Save();

    return hasChange;
}

bool
MainWindow::UpdateConfigure2()
{
    bool hasChange (false);
    bool saveChange (false);

    if(config_.ch2IPAddress_ != ui_->ipAddress_2->text())
    {
        hasChange = true;
        config_.ch2IPAddress_ = ui_->ipAddress_2->text();
    }

    if(config_.ch2PortNum_ != ui_->portNumber_2->text())
    {
        hasChange = true;
        config_.ch2PortNum_ = ui_->portNumber_2->text();
    }

    if(config_.ch2DataType_ != (DataType)ui_->dataType_2->currentIndex())
    {
        hasChange = true;
        config_.ch2DataType_ = (DataType)ui_->dataType_2->currentIndex();
    }

    if(config_.ch2FMURxCheck_ != (ui_->checkBoxFMURx->checkState() == Qt::Checked))
    {
        saveChange = true;
        config_.ch2FMURxCheck_ = (ui_->checkBoxFMURx->checkState() == Qt::Checked);
    }

    if(config_.ch2LoadFMURx_ != ui_->loadFMURx->text())
    {
        hasChange = true;
        config_.ch2LoadFMURx_ = ui_->loadFMURx->text();
    }

    if(config_.ch2LoadFile_ != ui_->loadFile_2->text())
    {
        hasChange = true;
        config_.ch2LoadFile_ = ui_->loadFile_2->text();
    }

    if(config_.ch2PlaybackDataType_ != (DataType)ui_->playbackDataType_2->currentIndex())
    {
        hasChange = true;
        config_.ch2PlaybackDataType_ = (DataType)ui_->playbackDataType_2->currentIndex();
    }

    if(config_.ch2DeltaDelay_ != ui_->deltaDelay_2->text().toInt())
    {
        saveChange = true;
        config_.ch2DeltaDelay_ = ui_->deltaDelay_2->text().toInt();
    }

    if(config_.ch2EnableSendOut_ != (ui_->enableSendToNetwork_2->checkState() == Qt::Checked))
    {
        saveChange = true;
        config_.ch2EnableSendOut_ = (ui_->enableSendToNetwork_2->checkState() == Qt::Checked);
    }

    if(config_.ch2SendOutPortNum_ != ui_->sendOutPortNum_2->text())
    {
        saveChange = true;
        config_.ch2SendOutPortNum_ = ui_->sendOutPortNum_2->text();
        reader2_->SetSendOutPortNum(config_.ch2SendOutPortNum_.toStdString());
    }

    if(config_.ch2FMUTxCheck_ != (ui_->checkBoxFMUTx_2->checkState() == Qt::Checked))
    {
        hasChange = true;
        config_.ch2FMUTxCheck_ = (ui_->checkBoxFMUTx_2->checkState() == Qt::Checked);
    }

    if(config_.ch2LoadFMUTx_ != ui_->loadFMUTx_2->text())
    {
        hasChange = true;
        config_.ch2LoadFMUTx_ = ui_->loadFMUTx_2->text();
    }

    if(hasChange || saveChange)
        config_.Save();

    return hasChange;
}

void
MainWindow::InitLoadConfigure()
{
    if (config_.Load())
    {
        ui_->ipAddress->setText(config_.ch1IPAddress_);
        ui_->portNumber->setText(config_.ch1PortNum_);
        ui_->dataType->setCurrentIndex( static_cast<int>(config_.ch1DataType_) );

        if(config_.ch1FMURxCheck_)
            ui_->checkBoxFMURx->setCheckState(Qt::Checked);
        else
            ui_->checkBoxFMURx->setCheckState(Qt::Unchecked);
        ui_->loadFMURx->setText(config_.ch1LoadFMURx_);

        ui_->loadFile->setText(config_.ch1LoadFile_);
        ui_->playbackDataType->setCurrentIndex( static_cast<int>(config_.ch1PlaybackDataType_) );
        ui_->deltaDelay->setText(QString::number(config_.ch1DeltaDelay_));

        if(config_.ch1EnableSendOut_)
            ui_->enableSendToNetwork->setCheckState(Qt::Checked);
        else
            ui_->enableSendToNetwork->setCheckState(Qt::Unchecked);
        ui_->sendOutPortNum->setText(config_.ch1SendOutPortNum_);
        reader_->SetSendOutPortNum(config_.ch1SendOutPortNum_.toStdString());

        if(config_.ch1FMUTxCheck_)
            ui_->checkBoxFMUTx->setCheckState(Qt::Checked);
        else
            ui_->checkBoxFMUTx->setCheckState(Qt::Unchecked);
        ui_->checkBoxFMUTx->setEnabled(config_.ch1EnableSendOut_);
        ui_->loadFMUTx->setEnabled(config_.ch1EnableSendOut_ && config_.ch1FMUTxCheck_);
        ui_->loadFMUTx->setText(config_.ch1LoadFMUTx_);

        if(config_.ch1ShowFOV_)
            ui_->showFOV->setCheckState(Qt::Checked);
        else
            ui_->showFOV->setCheckState(Qt::Unchecked);
        ui_->minRadius->setText(QString::number(config_.ch1MinRadius_));
        ui_->maxRadius->setText(QString::number(config_.ch1MaxRadius_));
        ui_->azimuthPos->setText(QString::number(config_.ch1AzimuthPos_));
        ui_->azimuthNeg->setText(QString::number(config_.ch1AzimuthNeg_));



        ui_->ipAddress_2->setText(config_.ch2IPAddress_);
        ui_->portNumber_2->setText(config_.ch2PortNum_);
        ui_->dataType_2->setCurrentIndex( static_cast<int>(config_.ch2DataType_) );

        if(config_.ch2FMURxCheck_)
            ui_->checkBoxFMURx_2->setCheckState(Qt::Checked);
        else
            ui_->checkBoxFMURx_2->setCheckState(Qt::Unchecked);
        ui_->loadFMURx_2->setText(config_.ch2LoadFMURx_);

        ui_->loadFile_2->setText(config_.ch2LoadFile_);
        ui_->playbackDataType_2->setCurrentIndex( static_cast<int>(config_.ch2PlaybackDataType_) );
        ui_->deltaDelay_2->setText(QString::number(config_.ch2DeltaDelay_));

        if(config_.ch2EnableSendOut_)
            ui_->enableSendToNetwork_2->setCheckState(Qt::Checked);
        else
            ui_->enableSendToNetwork_2->setCheckState(Qt::Unchecked);
        ui_->sendOutPortNum_2->setText(config_.ch2SendOutPortNum_);
        reader2_->SetSendOutPortNum(config_.ch2SendOutPortNum_.toStdString());

        if(config_.ch2FMUTxCheck_)
            ui_->checkBoxFMUTx_2->setCheckState(Qt::Checked);
        else
            ui_->checkBoxFMUTx_2->setCheckState(Qt::Unchecked);
        ui_->checkBoxFMUTx_2->setEnabled(config_.ch2EnableSendOut_);
        ui_->loadFMUTx_2->setEnabled(config_.ch2EnableSendOut_ && config_.ch2FMUTxCheck_);
        ui_->loadFMUTx_2->setText(config_.ch2LoadFMUTx_);

        if(config_.ch2ShowFOV_)
            ui_->showFOV_2->setCheckState(Qt::Checked);
        else
            ui_->showFOV_2->setCheckState(Qt::Unchecked);
        ui_->minRadius_2->setText(QString::number(config_.ch2MinRadius_));
        ui_->maxRadius_2->setText(QString::number(config_.ch2MaxRadius_));
        ui_->azimuthPos_2->setText(QString::number(config_.ch2AzimuthPos_));
        ui_->azimuthNeg_2->setText(QString::number(config_.ch2AzimuthNeg_));



        ui_->actionCombiCh->setChecked(config_.combineChannel_);
        ui_->actionShowGrid->setChecked(config_.showGrid_);
        ui_->actionShowObject->setChecked(config_.showObjectDetails_);
        ui_->actionLockCamera->setChecked(config_.lockCamera_);
    }
    else
    {
        config_.typeColors_.insert(ObjectType::UnknownObject, Qt::gray);
        config_.typeColors_.insert(ObjectType::OtherObject,   Qt::gray);
        config_.typeColors_.insert(ObjectType::Car,           Qt::green);
        config_.typeColors_.insert(ObjectType::Truck,         Qt::red);
        config_.typeColors_.insert(ObjectType::MotorBike,     Qt::yellow);
        config_.typeColors_.insert(ObjectType::Bicycle,       Qt::cyan);
        config_.typeColors_.insert(ObjectType::Trailer,       Qt::darkBlue);
        config_.typeColors_.insert(ObjectType::Pedestrian,    Qt::blue);
        config_.typeColors_.insert(ObjectType::Animal,        Qt::darkMagenta);
        config_.typeColors_.insert(ObjectType::Bridge,        Qt::darkGray);
        config_.typeColors_.insert(ObjectType::Building,      Qt::darkGray);
        config_.typeColors_.insert(ObjectType::Pylon,         Qt::darkRed);
        config_.typeColors_.insert(ObjectType::ReflectorPost, Qt::darkRed);
        config_.typeColors_.insert(ObjectType::Delineator,    Qt::darkRed);
        config_.typeColors_.insert(ObjectType::TrafficSign,   Qt::white);
        config_.typeColors_.insert(ObjectType::TrafficLight,  Qt::darkGreen);
        ui_->actionShowGrid->setChecked(true);
        ui_->actionShowObject->setChecked(true);
    }
}

void
MainWindow::InitComboBoxes()
{
    ui_->dataType->addItem("GroundTruth");
    ui_->dataType->addItem("SensorData");
    ui_->dataType_2->addItem("GroundTruth");
    ui_->dataType_2->addItem("SensorData");

    ui_->playbackDataType->addItem("GroundTruth");
    ui_->playbackDataType->addItem("SensorData");
    ui_->playbackDataType_2->addItem("GroundTruth");
    ui_->playbackDataType_2->addItem("SensorData");
}

void
MainWindow::InitLaneTypeMenu()
{
    QAction* actBoundaryLane = ui_->menuLaneType->addAction("Boundary Lanes");
    QAction* actCenterLane = ui_->menuLaneType->addAction("Center Lanes");

    connect(actBoundaryLane, &QAction::triggered, this, &MainWindow::ChooseBoundaryLanes);
    connect(actCenterLane, &QAction::triggered, this, &MainWindow::ChooseCenterLanes);
}

void
MainWindow::InitLegendGroupBox()
{
    foreach (ObjectType key, config_.typeColors_.keys())
    {
        QColor color = config_.typeColors_.value(key);

        //black border
        QWidget* colorBaseWidget = new QWidget(this);
        colorBaseWidget->setFixedSize(15, 15);
        colorBaseWidget->setStyleSheet("background-color:black");
        ui_->legendLayout->addWidget(colorBaseWidget);

        //color widget
        QWidget* colorWidget = new QWidget(colorBaseWidget);
        colorWidget->resize(13, 13);
        colorWidget->move(1, 1);
        colorWidget->setStyleSheet("background-color:" + color.name());
        colorWidgets_.insert(key, colorWidget);

        //color label
        QLabel* colorLabel = new QLabel();
        colorLabel->setFixedHeight(15);
        colorLabel->setText(Global::GetObjectTypeName(key));
        ui_->legendLayout->addWidget(colorLabel);
    }
}

void
MainWindow::LockCheckBoxToggled(bool checked)
{
    config_.lockCamera_ = checked;
    config_.Save();
}

void
MainWindow::ShowGridCheckBoxToggled(bool checked)
{
    config_.showGrid_ = checked;
    config_.Save();
    emit UpdateGrid();
}

void
MainWindow::ShowObjectOverview(bool checked)
{
    config_.showObjectDetails_ = checked;
    config_.Save();

    if(checked == false)
    {
        if(displayObjectDlg_ != nullptr)
            displayObjectDlg_->done(0);

        if(displayObjectDlg2_ != nullptr)
            displayObjectDlg2_->done(0);
    }
}

void
MainWindow::PythonCompare()
{
    PythonCompareDialog* pDlg = new PythonCompareDialog(this,
                                                        config_.srcPath_,
                                                        config_.ch1LoadFile_,
                                                        config_.ch2LoadFile_);
    pDlg->showNormal();
}

void
MainWindow::CombineChannels()
{
    config_.combineChannel_ = !config_.combineChannel_;
    config_.Save();
}

void
MainWindow::ShowFOV()
{
    // fetch data
    if(ui_->showFOV->isChecked())
    {
        QString err;
        bool success = false;
        float minR = ui_->minRadius->text().toFloat(&success);
        if(!success)
            err += "input valid minimum radius parameter in Channel 1\n";

        float maxR = ui_->maxRadius->text().toFloat(&success);
        if(!success)
            err += "input valid maximum radius parameter in Channel 1\n";

        float aPos = ui_->azimuthPos->text().toFloat(&success);
        if(!success)
            err += "input valid azimuth positive angle in Channel 1\n";

        float aNeg = ui_->azimuthNeg->text().toFloat(&success);
        if(!success)
            err += "input valid azimuth negtive angle in Channel 1\n";

        if(err.isEmpty())
        {
            config_.ch1ShowFOV_ = true;
            config_.ch1MinRadius_ = minR;
            config_.ch1MaxRadius_ = maxR;
            config_.ch1AzimuthPos_ = aPos;
            config_.ch1AzimuthNeg_ = aNeg;
            EnableShowFOV(false);
            aPos = (aPos/180)*M_PI;
            aNeg = (aNeg/180)*M_PI;
            glWidget_->UpdateFOVParam(minR, maxR, aPos, aNeg);
        }
        else
        {
            config_.ch1ShowFOV_ = false;
            EnableShowFOV(true);
            glWidget_->UpdateFOVPaint(false);
            ui_->showFOV->setChecked(false);
            ShowErrorMessage(err);
        }
    }
    else
    {
        EnableShowFOV(true);
        glWidget_->UpdateFOVPaint(false);
    }

    config_.Save();
}

void
MainWindow::ShowFOV2()
{
    // fetch data
    if(ui_->showFOV_2->isChecked())
    {
        QString err;
        bool success = false;
        float minR = ui_->minRadius_2->text().toFloat(&success);
        if(!success)
            err += "input valid minimum radius parameter in Channel 2\n";

        float maxR = ui_->maxRadius_2->text().toFloat(&success);
        if(!success)
            err += "input valid maximum radius parameter in Channel 2\n";

        float aPos = ui_->azimuthPos_2->text().toFloat(&success);
        if(!success)
            err += "input valid azimuth positive angle in Channel 2\n";

        float aNeg = ui_->azimuthNeg_2->text().toFloat(&success);
        if(!success)
            err += "input valid azimuth negtive angle in Channel 2\n";

        if(err.isEmpty())
        {
            config_.ch2ShowFOV_ = true;
            config_.ch2MinRadius_ = minR;
            config_.ch2MaxRadius_ = maxR;
            config_.ch2AzimuthPos_ = aPos;
            config_.ch2AzimuthNeg_ = aNeg;
            EnableShowFOV2(false);
            aPos = (aPos/180)*M_PI;
            aNeg = (aNeg/180)*M_PI;
            glWidget2_->UpdateFOVParam(minR, maxR, aPos, aNeg);
        }
        else
        {
            config_.ch2ShowFOV_ = false;
            EnableShowFOV2(true);
            glWidget2_->UpdateFOVPaint(false);
            ui_->showFOV_2->setChecked(false);
            ShowErrorMessage(err);
        }
    }
    else
    {
        EnableShowFOV2(true);
        glWidget2_->UpdateFOVPaint(false);
    }

    config_.Save();
}

void
MainWindow::EnableShowFOV(const bool enable)
{
    ui_->minRadius->setEnabled(enable);
    ui_->maxRadius->setEnabled(enable);
    ui_->azimuthNeg->setEnabled(enable);
    ui_->azimuthPos->setEnabled(enable);
}

void
MainWindow::EnableShowFOV2(const bool enable)
{
    ui_->minRadius_2->setEnabled(enable);
    ui_->maxRadius_2->setEnabled(enable);
    ui_->azimuthNeg_2->setEnabled(enable);
    ui_->azimuthPos_2->setEnabled(enable);
}

void
MainWindow::ChooseBoundaryLanes()
{
    config_.laneType_ = LaneType::BoundaryLanes;
    config_.Save();
}

void
MainWindow::ChooseCenterLanes()
{
    config_.laneType_ = LaneType::CenterLanes;
    config_.Save();
}

void
MainWindow::Quit()
{
    Stop();
    Stop2();
    while(isConnected_ == true || isConnected2_ == true);
    QApplication::quit();
}

void
MainWindow::closeEvent(QCloseEvent * event)
{
    Quit();
    event->accept();
}

void
MainWindow::ShowContextMenu(const QPoint& pos)
{
    QPoint globalPos = ui_->objectTree->mapToGlobal(pos);
    CustomTreeWidgetItem* selectedNode = (CustomTreeWidgetItem*)ui_->objectTree->itemAt(pos);

    if (!selectedNode || !treeNodes_.values().contains(selectedNode))
    {
        return;
    }

    QMenu contextMenu;
    QAction* colorAction = new QAction(0);
    colorAction->setText("Set color...");
    contextMenu.addAction(colorAction);

    QAction* selectedAction = contextMenu.exec(globalPos);
    if (selectedAction)
    {
        if (selectedAction == colorAction)
        {
            ObjectType type = selectedNode->objectType_;

            QColor color = QColorDialog::getColor(config_.typeColors_.value(type));
            if (color.isValid())
            {
                // Insert will override the current value if the key already exists
                config_.typeColors_.insert(type, color);
                UpdateLegend();
                config_.Save();
                glWidget_->update();
            }
        }
    }
}

void
MainWindow::UpdateLegend()
{
    for(const auto& key: colorWidgets_.keys())
    {
        QColor color = config_.typeColors_.value(key);
        QWidget* widget = colorWidgets_.value(key);
        widget->setStyleSheet("background-color:" + color.name());
    }
}

void
MainWindow::InitObjectTree()
{
    QList<ObjectType> allTypes = Global::GetAllObjectTypes();
    for(const auto& type: allTypes)
    {
        QTreeWidgetItem* node = CreateTreeNode(Global::GetObjectTypeName(type, true), type);
        ui_->objectTree->addTopLevelItem(node);
        treeNodes_.insert(type, node);
    }
}

void
MainWindow::InitObjectTree2()
{
    QList<ObjectType> allTypes = Global::GetAllObjectTypes();
    for(const auto& type: allTypes)
    {
        QTreeWidgetItem* node = CreateTreeNode(Global::GetObjectTypeName(type, true), type);
        ui_->objectTree_2->addTopLevelItem(node);
        treeNodes2_.insert(type, node);
    }
}

QTreeWidgetItem*
MainWindow::CreateTreeNode(const QString& text, const ObjectType& type)
{
    CustomTreeWidgetItem* item = new CustomTreeWidgetItem(nullptr);
    item->objectType_ = type;
    item->setText(0, text);
    return item;
}

void
MainWindow::Play()
{
    isPlaying_ = true;

    if(isSrcConnection_)
    {
        if(ui_->checkBoxFMURx->isChecked())
        {
            emit FMUConnectRequested(config_.ch1IPAddress_,
                                     config_.ch1PortNum_,
                                     config_.ch1LoadFMURx_,
                                     config_.ch1DataType_);
        }
        else
        {
            emit ConnectRequested(config_.ch1IPAddress_,
                                  config_.ch1PortNum_,
                                  config_.ch1DataType_);
        }
    }
    else
    {
        emit StartPlaybackRequested(config_.ch1LoadFile_,
                                    config_.ch1PlaybackDataType_,
                                    config_.ch1LoadFMUTx_);
    }
}

void
MainWindow::TogglePause()
{
    QString errMsg;

    if(isConnected_)
    {
        if(ui_->checkBoxFMURx->isChecked())
            fmuReceiver_->isPaused_ = isPlaying_;
        else
            tcpReceiver_->isPaused_ = isPlaying_;
    }
    else if(isPlayed_)
    {
        if(isPlaying_)
        {
            reader_->isPaused_ = true;
            reader_->SetupConnection(false);
        }
        else
        {
            errMsg = reader_->SetupConnection(true);
            if(errMsg.isEmpty())
                reader_->isPaused_ = false;
            else
                ShowErrorMessage(errMsg);
        }
    }

    if(errMsg.isEmpty())
        isPlaying_ = !isPlaying_;
}

void
MainWindow::Stop()
{
    if(isConnected_)
    {
        if(ui_->checkBoxFMURx->isChecked())
            fmuReceiver_->DisconnectRequested();
        else
            tcpReceiver_->DisconnectRequested();
    }
    else if(isPlayed_)
    {
        reader_->StopReadFile();
    }

    ResetSliderTime();
}

void
MainWindow::Play2()
{
    isPlaying2_ = true;

    if(isSrcConnection2_)
    {
        if(ui_->checkBoxFMURx_2->isChecked())
        {
            emit FMUConnectRequested2(config_.ch2IPAddress_,
                                      config_.ch2PortNum_,
                                      config_.ch2LoadFMURx_,
                                      config_.ch2DataType_);
        }
        else
        {
            emit ConnectRequested2(config_.ch2IPAddress_,
                                   config_.ch2PortNum_,
                                   config_.ch2DataType_);
        }
    }
    else
    {
        emit StartPlaybackRequested2(config_.ch2LoadFile_,
                                     config_.ch2PlaybackDataType_,
                                     config_.ch2LoadFMUTx_);
    }
}

void
MainWindow::TogglePause2()
{
    QString errMsg;

    if(isConnected2_)
    {
        if(ui_->checkBoxFMURx_2->isChecked())
            fmuReceiver2_->isPaused_ = isPlaying_;
        else
            tcpReceiver2_->isPaused_ = isPlaying2_;
    }
    else if(isPlayed2_)
    {
        if(isPlaying2_)
        {
            reader2_->isPaused_ = true;
            reader2_->SetupConnection(false);
        }
        else
        {
            errMsg = reader2_->SetupConnection(true);
            if(errMsg.isEmpty())
                reader2_->isPaused_ = false;
            else
                ShowErrorMessage(errMsg);
        }
    }

    if(errMsg.isEmpty())
        isPlaying2_ = !isPlaying2_;
}

void
MainWindow::Stop2()
{
    if(isConnected2_)
    {
        if(ui_->checkBoxFMURx_2->isChecked())
            fmuReceiver2_->DisconnectRequested();
        else
            tcpReceiver2_->DisconnectRequested();
    }
    else if(isPlayed2_)
    {
        reader2_->StopReadFile();
    }

    ResetSliderTime2();
}

void
MainWindow::ResetSliderTime()
{
    ui_->hSliderTime->setText(tr("--:--"));
    ui_->hSliderTimeRange->setText(tr("--:--"));
}

void
MainWindow::ResetSliderTime2()
{
    ui_->hSliderTime_2->setText(tr("--:--"));
    ui_->hSliderTimeRange_2->setText(tr("--:--"));
}

void
MainWindow::Connected(DataType dataType)
{
    if(isSrcConnection_)
        isConnected_ = true;
    else
        isPlayed_ = true;
}

void
MainWindow::Connected2(DataType dataType)
{
    if(isSrcConnection2_)
        isConnected2_ = true;
    else
        isPlayed2_ = true;
}

void
MainWindow::Disconnected(const QString& message)
{
    isConnected_ = false;
    isPlayed_ = false;
    isPlaying_ = false;

    UpdateFields();

    if (!message.isEmpty())
    {
        if(config_.combineChannel_)
        {
            ui_->actionCombiCh->setChecked(false);
            config_.combineChannel_ = false;
        }

        ShowErrorMessage(message);
    }

    UpdateCombineChannelMenu();
}

void
MainWindow::Disconnected2(const QString& message)
{
    isConnected2_ = false;
    isPlayed2_ = false;
    isPlaying2_ = false;

    UpdateFields2();

    if (!message.isEmpty())
    {
        if(config_.combineChannel_)
        {
            ui_->actionCombiCh->setChecked(false);
            config_.combineChannel_ = false;
        }

        ShowErrorMessage(message);
    }

    UpdateCombineChannelMenu();
}

QString
MainWindow::GetStringFromMilliSecond(int milliSec)
{
    QString varString;

    int rangeSecond = milliSec / 1000;
    int rangeSec = rangeSecond % 60;
    int rangeMin = rangeSecond / 60;
    if(rangeMin >= 60)
    {
        int rangeHour = rangeMin / 60;
        rangeMin %= 60;

        varString = QString("%1:%2:%3").arg(rangeHour, 2, 10, QChar('0'))
                                       .arg(rangeMin, 2, 10, QChar('0'))
                                       .arg(rangeSec, 2, 10, QChar('0'));
    }
    else
    {
        varString = QString("%1:%2").arg(rangeMin, 2, 10, QChar('0'))
                                    .arg(rangeSec, 2, 10, QChar('0'));
    }

    return varString;
}

void
MainWindow::UpdateSliderRange(int sliderRange)
{
    ui_->hSlider->setRange(0, sliderRange);
    ui_->hSliderTimeRange->setText(GetStringFromMilliSecond(sliderRange));
}

void
MainWindow::UpdateSliderRange2(int sliderRange)
{
    ui_->hSlider_2->setRange(0, sliderRange);
    ui_->hSliderTimeRange_2->setText(GetStringFromMilliSecond(sliderRange));
}

void
MainWindow::UpdateSliderValue(int sliderValue)
{
    ui_->hSlider->blockSignals(true);
    ui_->hSlider->setValue(sliderValue);
    ui_->hSlider->blockSignals(false);
    ui_->hSliderTime->setText(GetStringFromMilliSecond(sliderValue));
}

void
MainWindow::UpdateSliderValue2(int sliderValue)
{
    ui_->hSlider_2->blockSignals(true);
    ui_->hSlider_2->setValue(sliderValue);
    ui_->hSlider_2->blockSignals(false);
    ui_->hSliderTime_2->setText(GetStringFromMilliSecond(sliderValue));
}

void
MainWindow::EnableExport(bool enable)
{
    ui_->saveOSIMessage->setEnabled(enable);
}

void
MainWindow::EnableExport2(bool enable)
{
    ui_->saveOSIMessage_2->setEnabled(enable);
}

void
MainWindow::SaveOSIMsgOverflow(int osiMsgSaveThreshold)
{
    if(isOverflow_ == false)
    {
        isOverflow_ = true;
        PlayPauseButtonClicked();

        QString warnMsg;
        warnMsg = "Save OSI message over flow. \nThe maximum number of message is: ";
        warnMsg += QString::number(osiMsgSaveThreshold);
        warnMsg += ".\nSuspend!";

        QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Warning", warnMsg);
    }
}

void
MainWindow::SaveOSIMsgOverflow2(int osiMsgSaveThreshold)
{
    if(isOverflow2_ == false)
    {
        isOverflow2_ = true;
        PlayPauseButtonClicked();

        QString warnMsg;
        warnMsg = "Save OSI message over flow. \nThe maximum number of message is: ";
        warnMsg += QString::number(osiMsgSaveThreshold);
        warnMsg += ".\nSuspend!";

        QMessageBox::StandardButton reply = QMessageBox::critical(nullptr, "Warning", warnMsg);
    }
}

void
MainWindow::ToggleSaveOSIMessage()
{
    if(isSavingOSI_ == false)
    {
        isSavingOSI_ = true;
        ui_->saveOSIMessage->setIcon(pauseIcon_);
    }
    else
    {
        isSavingOSI_ = false;
        ui_->saveOSIMessage->setIcon(playIcon_);
    }
}

void
MainWindow::ToggleSaveOSIMessage2()
{
    if(isSavingOSI2_ == false)
    {
        isSavingOSI2_ = true;
        ui_->saveOSIMessage_2->setIcon(pauseIcon_);
    }
    else
    {
        isSavingOSI2_ = false;
        ui_->saveOSIMessage_2->setIcon(playIcon_);
    }
}

void
MainWindow::SetTrackingEnabled(bool enable)
{
    ui_->startStopTrackingButton->setEnabled(enable);
}

void
MainWindow::SetTrackingEnabled2(bool enable)
{
    ui_->startStopTrackingButton_2->setEnabled(enable);
}

void
MainWindow::DisplayObjectInformation(GLObject* object)
{
    if(displayObjectDlg_ == nullptr)
    {
        displayObjectDlg_ = new DisplayObjectDialog(this);
        displayObjectDlg_->showNormal();
        displayObjectDlg_->move(QCursor::pos());

        connect(displayObjectDlg_, &QDialog::finished, this, &MainWindow::DisplayObjectDialogFinished);
    }

    displayObjectDlg_->UpdateObjectInformation(*object);
}

void
MainWindow::DisplayObjectInformation2(GLObject* object)
{
    if(displayObjectDlg2_ == nullptr)
    {
        displayObjectDlg2_ = new DisplayObjectDialog(this);
        displayObjectDlg2_->showNormal();
        displayObjectDlg2_->move(QCursor::pos());

        connect(displayObjectDlg2_, &QDialog::finished, this, &MainWindow::DisplayObjectDialogFinished2);
    }

    displayObjectDlg2_->UpdateObjectInformation(*object);
}

void
MainWindow::ConnectDisplayObjectInformation(QTreeWidgetItem* , int)
{
    if(ui_->actionShowObject->isChecked())
        connect(glWidget_, &GLWidget::DisplayObjectInformation, this, &MainWindow::DisplayObjectInformation);
}

void
MainWindow::ConnectDisplayObjectInformation2(QTreeWidgetItem* , int)
{
    if(ui_->actionShowObject->isChecked())
        connect(glWidget2_, &GLWidget::DisplayObjectInformation, this, &MainWindow::DisplayObjectInformation2);
}

void
MainWindow::DisplayObjectDialogFinished(int)
{
    displayObjectDlg_ = nullptr;
    disconnect(glWidget_, &GLWidget::DisplayObjectInformation, this, &MainWindow::DisplayObjectInformation);
}

void
MainWindow::DisplayObjectDialogFinished2(int)
{
    displayObjectDlg2_ = nullptr;
    disconnect(glWidget2_, &GLWidget::DisplayObjectInformation, this, &MainWindow::DisplayObjectInformation2);
}

