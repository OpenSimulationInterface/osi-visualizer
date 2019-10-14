///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_PYTHON_COMPARE_DIALOG_H
#define OSI_VISUALIZER_PYTHON_COMPARE_DIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>

class PythonCompareDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit PythonCompareDialog(QWidget* parent,
                                 const QString& scriptPath,
                                 const QString& osiFile1,
                                 const QString& osiFile2);

  private slots:
    void OnBrowseFile1();
    void OnBrowseFile2();
    void OnRun();
    void OnExport();

  private:
    void InitScriptsSelection();

  private:
    QLineEdit* osiFile1_;
    QLineEdit* osiFile2_;
    QComboBox* scripts_;
    QTextEdit* logWin_;

    QString scriptPath_;
};
#endif  // OSI_VISUALIZER_PYTHON_COMPARE_DIALOG_H