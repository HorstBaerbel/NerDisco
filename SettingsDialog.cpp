#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "Settings.h"

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>


SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        ui->comboBoxSerialPort->addItem(info.portName());
    //read saved port name and try to select it
    Settings & settings = Settings::getInstance();
    if (!settings.portName().isEmpty())
    {
        if (ui->comboBoxSerialPort->findText(settings.portName()) >= 0)
        {
            ui->comboBoxSerialPort->setCurrentIndex(ui->comboBoxSerialPort->findText(settings.portName()));
        }
    }
    //read rest of settings
    ui->spinBoxDisplayWidth->setValue(settings.displayWidth());
    ui->spinBoxDisplayHeight->setValue(settings.displayHeight());
    ui->spinBoxFramebufferWidth->setValue(settings.frameBufferWidth());
    ui->spinBoxFramebufferHeight->setValue(settings.frameBufferHeight());
    ui->spinBoxDisplayInterval->setValue(settings.displayInterval());
    ui->lineEditConnectionName->setText(settings.audioSinkName());
    ui->checkBoxFlipHorizontal->setChecked(settings.displayFlipHorizontal());
    ui->checkBoxFlipVertical->setChecked(settings.displayFlipVertical());
    switch (settings.scanlineDirection())
    {
        case Settings::ConstantLeftToRight:
            ui->radioButtonConstantLeftRight->setChecked(true);
            break;
        case Settings::ConstantRightToLeft:
            ui->radioButtonConstantRightLeft->setChecked(true);
            break;
        case Settings::AlternatingStartLeft:
            ui->radioButtonAlternatingLeft->setChecked(true);
            break;
        case Settings::AlternatingStartRight:
            ui->radioButtonAlternatingRight->setChecked(true);
            break;
    }
    //connect signal when user chooses ok to store settings
    connect(this, SIGNAL(accepted()), this, SLOT(storeSettings()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::storeSettings()
{
    Settings & settings = Settings::getInstance();
    settings.setPortName(ui->comboBoxSerialPort->currentText());
    settings.setDisplayWidth(ui->spinBoxDisplayWidth->value());
    settings.setDisplayHeight(ui->spinBoxDisplayHeight->value());
    settings.setFrameBufferWidth(ui->spinBoxFramebufferWidth->value());
    settings.setFrameBufferHeight(ui->spinBoxFramebufferHeight->value());
    settings.setDisplayInterval(ui->spinBoxDisplayInterval->value());
    settings.setAudioSinkName(ui->lineEditConnectionName->text());
    settings.setDisplayFlipHorizontal(ui->checkBoxFlipHorizontal->isChecked());
    settings.setDisplayFlipVertical(ui->checkBoxFlipVertical->isChecked());
    if (ui->radioButtonConstantLeftRight->isChecked())
    {
        settings.setScanlineDirection(Settings::ConstantLeftToRight);
    }
    if (ui->radioButtonConstantRightLeft->isChecked())
    {
        settings.setScanlineDirection(Settings::ConstantRightToLeft);
    }
    else if (ui->radioButtonAlternatingLeft->isChecked())
    {
        settings.setScanlineDirection(Settings::AlternatingStartLeft);
    }
    else if (ui->radioButtonAlternatingRight->isChecked())
    {
        settings.setScanlineDirection(Settings::AlternatingStartRight);
    }
    settings.save();
}
