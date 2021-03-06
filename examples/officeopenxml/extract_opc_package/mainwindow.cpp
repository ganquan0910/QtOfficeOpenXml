/****************************************************************************
**
** Copyright (C) 2014 Debao Zhang <hello@debao.me>
**
** This library is free software; you can redistribute it and/or modify
** it under the terms of the GNU Library General Public License version
** 2.1 or version 3 as published by the Free Software Foundation.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public License
** along with this library; see the file LICENSE.LGPLv21 and LICENSE.LGPLv3
** included in the packaging of this file. If not, write to the Free
** Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
** 02110-1301, USA.
**
****************************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "binedit.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_package(0)
{
    ui->setupUi(this);

    connect(ui->action_Open, SIGNAL(triggered()), SLOT(onOpen()));
    connect(ui->action_About, SIGNAL(triggered()), SLOT(onAbout()));
    connect(ui->partListWidget, SIGNAL(currentTextChanged(QString)), SLOT(onPartChanged(QString)));
    connect(ui->partContentButton, SIGNAL(clicked()), SLOT(onShowContentButtonClicked()));
}

MainWindow::~MainWindow()
{
    delete m_package;
    delete ui;
}

void MainWindow::onOpen()
{
    QSettings settings("opcpackagedemo.ini", QSettings::IniFormat);
    const QString path = settings.value("lastFile").toString();

    QString fn = QFileDialog::getOpenFileName( 0, tr("Open OPC Package"), path, "Office files (*.xlsx *.docx *.pptx);;All files(*.*)");
    if (fn.isEmpty())
        return;

    ui->partListWidget->clear();

    m_package = Opc::Package::open(fn, QIODevice::ReadOnly);
    if (!m_package) {
        //Error occur.
        statusBar()->showMessage(tr("Fail to open the package %1").arg(fn));
        return;
    }
    settings.setValue("lastFile", fn);

    ui->partListWidget->addItem("/");
    foreach (Opc::PackagePart *part, m_package->parts())
        ui->partListWidget->addItem(part->partName());
}

void MainWindow::onAbout()
{
    QMessageBox::information(this, tr("About"), tr("This example is designed to show how to used Opc::Package class."));
}

void MainWindow::onPartChanged(const QString &partName)
{
    ui->partContentButton->setDisabled(true);
    ui->partRelationshipListWidget->clear();
    ui->partNameEdit->clear();
    ui->partContentTypeEdit->clear();

    if (partName.isEmpty())
        return;

    if (partName == "/") {
        foreach (Opc::PackageRelationship *relation, m_package->relationships())
            ui->partRelationshipListWidget->addItem(QString("%1: %2").arg(relation->id(), relation->target()));
        return;
    }

    Opc::PackagePart *part = m_package->part(partName);
    if (!part) {
        statusBar()->showMessage(tr("Error open the part %1").arg(partName), 2000);
        return;
    }

    ui->partContentButton->setEnabled(true);
    ui->partNameEdit->setText(part->partName());
    ui->partContentTypeEdit->setText(part->contentType());

    foreach (Opc::PackageRelationship *relation, part->relationships())
        ui->partRelationshipListWidget->addItem(QString("%1: %2").arg(relation->id(), relation->target()));
}

void MainWindow::onShowContentButtonClicked()
{
    Opc::PackagePart *part = m_package->part(ui->partNameEdit->text());
    if (!part)
        return;

    //Extract the data from part stream.
    QByteArray contentData = part->getDevice()->readAll();
    part->releaseDevice();

    //Show data in bin edit.
    BinEdit *bEdit = new BinEdit;
    bEdit->setData(contentData);
    bEdit->setWindowTitle(part->partName());

    bEdit->setReadOnly(true);
    bEdit->setAttribute(Qt::WA_DeleteOnClose);
    QFont font = bEdit->font();
    font.setPointSize(14);
    bEdit->setFont(font);
    bEdit->resize(800, 600);

    bEdit->show();
}
