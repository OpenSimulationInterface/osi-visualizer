///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once
#include "types.h"
#include "globject.h"
#include "appconfig.h"
#include <QMainWindow>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPushButton>
#include <QTreeWidgetItem>

class GLWidget;
class TCPReceiver;
class AppConfig;
class OsiParser;
class OsiReader;
class DisplayObjectDialog;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow();

    signals:
        void UpdateGrid();
        void UpdateLane();

        void ConnectRequested(const QString& ipAddress,
                              const QString& port,
                              const DataType dataType);

        void ConnectRequested2(const QString& ipAddress,
                               const QString& port,
                               const DataType dataType);

        void StartPlaybackRequested(const QString& fileName,
                                    const DataType dataType);

        void StartPlaybackRequested2(const QString& fileName,
                                     const DataType dataType);

    public slots:
        void EnableExport(bool enable);
        void EnableExport2(bool enable);
        void SaveOSIMsgOverflow(int osiMsgSaveThreshold);
        void SaveOSIMsgOverflow2(int osiMsgSaveThreshold);
        void ToggleSaveOSIMessage();
        void ToggleSaveOSIMessage2();
        void Connected(DataType dataType);
        void Connected2(DataType dataType);
        void Disconnected(const QString& message);
        void Disconnected2(const QString& message);

        void UpdateSliderRange(int sliderRange);
        void UpdateSliderRange2(int sliderRange);
        void UpdateSliderValue(int sliderValue);
        void UpdateSliderValue2(int sliderValue);

        void DisplayObjectInformation(GLObject* object);
        void DisplayObjectInformation2(GLObject* object);

        void SetTrackingEnabled(bool enable);
        void SetTrackingEnabled2(bool enable);

        // slots for menu
    private slots:
        void LockCheckBoxToggled(bool checked);
        void ShowGridCheckBoxToggled(bool checked);
        void ShowObjectOverview(bool checked);
        void ChooseBoundaryLanes();
        void ChooseCenterLanes();
        void Quit();

    private slots:
        void RBConnection();
        void RBConnection2();
        void RBPlayback();
        void RBPlayback2();

        void LoadFileEdited(const QString& text);
        void LoadFileBrowse();

        void LoadFileEdited2(const QString& text);
        void LoadFileBrowse2();

        void EnableSendToNetwork();
        void EnableSendToNetwork2();

        void PlayPauseButtonClicked();
        void PlayPauseButtonClicked2();

        void ShowContextMenu(const QPoint& pos);

        void ConnectDisplayObjectInformation(QTreeWidgetItem* , int);
        void ConnectDisplayObjectInformation2(QTreeWidgetItem* , int);
        void DisplayObjectDialogFinished(int);
        void DisplayObjectDialogFinished2(int);

        void CombineChannels();


    private:

        void closeEvent(QCloseEvent * event);

        void ConnectSignalsToSlots();
        void EnableSrcRadioButton(bool enable);
        void EnableSrcRadioButton2(bool enable);
        void ToggleSrcGroups();
        void ToggleSrcGroups2();

        void EnableSrcGroups(bool enable);
        void EnableSrcGroups2(bool enable);

        void EnableConnectionGroup1(bool enable);
        void EnableConnectionGroup2(bool enable);

        void EnablePlaybackGroup1(bool enable);
        void EnablePlaybackGroup2(bool enable);

        void EnableSlider(bool enable);
        void EnableSlider2(bool enable);

        void TogglePlayPauseButton();
        void TogglePlayPauseButton2();

        void CancelSaveOSIMessage();
        void CancelSaveOSIMessage2();
        void UpdateGLWidgetMessageSource();
        void UpdateGLWidgetMessageSource2();

        void UpdateFields();
        void UpdateFields2();

        bool UpdateConfigure();
        bool UpdateConfigure2();

        void InitLoadConfigure();
        void InitComboBoxes();
        void InitLaneTypeMenu();

        void UpdateLegend();
        void InitObjectTree();
        void InitObjectTree2();
        void InitLegendGroupBox();
        QTreeWidgetItem* CreateTreeNode(const QString& text, const ObjectType& type);

        void PythonCompare();

        void LocalPlayPause();
        void LocalPlayPause2();

        void Play();
        void TogglePause();
        void Stop();

        void Play2();
        void TogglePause2();
        void Stop2();

        void ResetSliderTime();
        void ResetSliderTime2();

        QString GetStringFromMilliSecond(int milliSec);

        void UpdateCombineChannelMenu();

        bool CheckFieldsValidity();
        bool CheckFieldsValidity2();

        void ShowErrorMessage(const QString& errMsg);


        // Configurations
        AppConfig config_;

        // Source come from ethernet or input file
        bool isSrcConnection_;
        bool isSrcConnection2_;

        // Play or Pause
        bool isPlaying_;
        bool isPlaying2_;

        // ethernet connected
        bool isConnected_;
        bool isConnected2_;
        // input file playback
        bool isPlayed_;
        bool isPlayed2_;

        bool isSavingOSI_;
        bool isSavingOSI2_;
        bool isOverflow_;
        bool isOverflow2_;

        Ui::MainWindow *ui_;

        GLWidget*    glWidget_;
        TCPReceiver* receiver_;
        OsiReader*   reader_;
        OsiParser*   osiparser_;

        GLWidget*    glWidget2_;
        TCPReceiver* receiver2_;
        OsiReader*   reader2_;
        OsiParser*   osiparser2_;

        QMap<ObjectType, QWidget*> colorWidgets_;
        QMap<ObjectType, QTreeWidgetItem*> treeNodes_;
        QMap<ObjectType, QTreeWidgetItem*> treeNodes2_;

        QIcon playIcon_;
        QIcon pauseIcon_;

        DisplayObjectDialog* displayObjectDlg_;
        DisplayObjectDialog* displayObjectDlg2_;
};

