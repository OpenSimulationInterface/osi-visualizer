/*
 * pythoncomparedialog.cpp
 *
 *  Created on: Nov 4, 2017
 *      Author:
 */



#include "pythoncomparedialog.h"

#include <fstream>

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QDirIterator>
#include <QMessageBox>


PythonCompareDialog::PythonCompareDialog(QWidget *parent,
                                         const QString& scriptPath,
                                         const QString& osiFile1,
                                         const QString& osiFile2)
    : QDialog(parent)
    , osiFile1_(new QLineEdit(osiFile1, this))
    , osiFile2_(new QLineEdit(osiFile2, this))
    , scripts_(new QComboBox(this))
    , logWin_(new QTextEdit(this))
    , scriptPath_(scriptPath)
{
    setWindowTitle(tr("Python Compare"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QGridLayout *mainGridLayout = new QGridLayout(this);
    // File 1
    mainGridLayout->addWidget(new QLabel(tr("OSI File 1:"), this), 0, 0);
    mainGridLayout->addWidget(osiFile1_, 0, 1);
    QPushButton* pBrowse1 = new QPushButton(tr("..."), this);
    pBrowse1->setMaximumWidth(30);
    connect(pBrowse1, &QPushButton::clicked, this, &PythonCompareDialog::OnBrowseFile1);
    mainGridLayout->addWidget(pBrowse1, 0, 2);
    // File 2
    mainGridLayout->addWidget(new QLabel(tr("OSI File 2:"), this), 1, 0);
    mainGridLayout->addWidget(osiFile2_, 1, 1);
    QPushButton* pBrowse2 = new QPushButton(tr("..."), this);
    pBrowse2->setMaximumWidth(30);
    connect(pBrowse2, &QPushButton::clicked, this, &PythonCompareDialog::OnBrowseFile2);
    mainGridLayout->addWidget(pBrowse2, 1, 2);

    QHBoxLayout* hBoxLayout = new QHBoxLayout();
    hBoxLayout->addWidget(new QLabel(tr("Python Scripts:"), this));
    // combobox selection
    InitScriptsSelection();
    hBoxLayout->addWidget(scripts_, 1);
    // run button
    QPushButton* pRun = new QPushButton(tr("Run"), this);
    connect(pRun, &QPushButton::clicked, this, &PythonCompareDialog::OnRun);
    pRun->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    hBoxLayout->addWidget(pRun, 1);
    // export button
    QPushButton* pExport = new QPushButton(tr("Export"), this);
    connect(pExport, &QPushButton::clicked, this, &PythonCompareDialog::OnExport);
    pExport->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    hBoxLayout->addWidget(pExport, 1);

    mainGridLayout->addLayout(hBoxLayout, 2, 0, 1, 2);

    logWin_->setMinimumSize(400, 200);
    logWin_->setReadOnly(true);

    mainGridLayout->addWidget(logWin_, 3, 0, 1, 2);
}

void
PythonCompareDialog::OnBrowseFile1()
{
    QString fileName = osiFile1_->text();

    fileName = QFileDialog::getOpenFileName(this,
        tr("Load file"), fileName, tr("Text Files (*.txt)"));

    if(!fileName.isEmpty())
    {
        osiFile1_->setText(fileName);
        osiFile1_->setToolTip(fileName);
    }
}

void
PythonCompareDialog::OnBrowseFile2()
{
    QString fileName = osiFile2_->text();

    fileName = QFileDialog::getOpenFileName(this,
            tr("Open OSI GroundTruth Files"), "", tr("All Files (*)"));

    if(!fileName.isEmpty())
    {
        osiFile2_->setText(fileName);
        osiFile2_->setToolTip(fileName);
    }
}

void
PythonCompareDialog::OnRun()
{
    static const std::string tmpLog = "python_compare_log.txt";
    remove(tmpLog.c_str());
    logWin_->clear();

    QString script = scripts_->currentText();
    if( script.isEmpty() == false &&
        osiFile1_->text().isEmpty() == false &&
        osiFile2_->text().isEmpty() == false )
    {
        std::string command = "python " +
                              scriptPath_.toStdString() +
                              "Resources/Python/";
        command += script.toStdString();
        command += " " + osiFile1_->text().toStdString();
        command += " " + osiFile2_->text().toStdString();
        command += " >> " + tmpLog;
        system(command.c_str());

        std::ifstream pLog(tmpLog.c_str());
        if(pLog.is_open())
        {
            logWin_->append(tr("---- Start Running ") + script + tr(" ----"));

            while (!pLog.good()){
                std::string s;
                getline(pLog, s);
                logWin_->append(QString::fromStdString(s));
            }
            logWin_->append(tr("---- End of Running ----"));
        }
        pLog.close();
    }
    else
    {
        QString errMsg;
        if(script.isEmpty())
            errMsg += "No python script found!\n";

        if(osiFile1_->text().isEmpty())
            errMsg += "OSI file 1 is empty!\n";

        if(osiFile2_->text().isEmpty())
            errMsg += "OSI file 2 is empty!\n";

        QMessageBox::critical(nullptr,
                              "Error",
                              errMsg,
                              QMessageBox::Ok,
                              QMessageBox::Ok);
    }
}

void
PythonCompareDialog::OnExport()
{
    QString fileNamePyLog = QFileDialog::getSaveFileName(this,
            tr("Save Python Validation Results"), "", tr("All Files (*)"));

    std::ofstream pythonLog(fileNamePyLog.toStdString().c_str());
    if(pythonLog.is_open())
    {
        pythonLog << logWin_->toPlainText().toStdString();
    }

    pythonLog.close();
}

void
PythonCompareDialog::InitScriptsSelection()
{
    QDirIterator it(scriptPath_ + tr("/Resources/Python/"), QStringList() << "*.py", QDir::Files);
    while (it.hasNext())
    {
        it.next();
        scripts_->addItem(it.fileName());
    }
}

