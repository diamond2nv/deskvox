// Virvo - Virtual Reality Volume Rendering
// Copyright (C) 1999-2003 University of Stuttgart, 2004-2005 Brown University
// Contact: Jurgen P. Schulze, jschulze@ucsd.edu
//
// This file is part of Virvo.
//
// Virvo is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library (see license.txt); if not, write to the
// Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

#include "vvcanvas.h"
#include "vvdimensiondialog.h"
#include "vvmainwindow.h"
#include "vvmergedialog.h"
#include "vvplugin.h"
#include "vvpluginutil.h"
#include "vvscreenshotdialog.h"
#include "vvtfdialog.h"
#include "vvtimestepdialog.h"

#include "ui_vvmainwindow.h"

#include <virvo/vvdebugmsg.h>
#include <virvo/vvfileio.h>
#include <virvo/vvvoldesc.h>

#include <QApplication>
#include <QByteArray>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

vvMainWindow::vvMainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui_MainWindow)
{
  vvDebugMsg::msg(1, "vvMainWindow::vvMainWindow()");

  ui->setupUi(this);

  // plugins
  m_plugins = vvPluginUtil::getAll();
  foreach (vvPlugin* plugin, m_plugins)
  {
    if (QDialog* dialog = plugin->dialog(this))
    {
      ui->menuPlugins->setEnabled(true);
      QAction* dialogAction = new QAction(plugin->name(), ui->menuPlugins);
      ui->menuPlugins->addAction(dialogAction);
      connect(dialogAction, SIGNAL(triggered()), dialog, SLOT(open()));
    }
  }

  // widgets and dialogs
  _canvas = new vvCanvas(this);
  setCentralWidget(_canvas);

  _tfDialog = new vvTFDialog(_canvas, this);

  _dimensionDialog = new vvDimensionDialog(_canvas, this);
  _mergeDialog = new vvMergeDialog(this);
  _screenshotDialog = new vvScreenshotDialog(_canvas, this);
  _timeStepDialog = new vvTimeStepDialog(this);

  // file menu
  connect(ui->actionLoadVolume, SIGNAL(triggered()), this, SLOT(onLoadVolumeTriggered()));
  connect(ui->actionReloadVolume, SIGNAL(triggered()), this, SLOT(onReloadVolumeTriggered()));
  connect(ui->actionSaveVolumeAs, SIGNAL(triggered()), this, SLOT(onSaveVolumeAsTriggered()));
  connect(ui->actionMergeFiles, SIGNAL(triggered()), this, SLOT(onMergeFilesTriggered()));
  connect(ui->actionLoadCamera, SIGNAL(triggered()), this, SLOT(onLoadCameraTriggered()));
  connect(ui->actionSaveCameraAs, SIGNAL(triggered()), this, SLOT(onSaveCameraAsTriggered()));
  connect(ui->actionScreenshot, SIGNAL(triggered()), this, SLOT(onScreenshotTriggered()));

  // settings menu
  connect(ui->actionBackgroundColor, SIGNAL(triggered()), this, SLOT(onBackgroundColorTriggered()));
  connect(ui->actionTransferFunction, SIGNAL(triggered()), this, SLOT(onTransferFunctionTriggered()));

  // edit menu
  connect(ui->actionSampleDistances, SIGNAL(triggered()), this, SLOT(onSampleDistancesTriggered()));

  // view menu
  connect(ui->actionShowOrientation, SIGNAL(triggered(bool)), this, SLOT(onShowOrientationTriggered(bool)));
  connect(ui->actionShowBoundaries, SIGNAL(triggered(bool)), this, SLOT(onShowBoundariesTriggered(bool)));
  connect(ui->actionShowPalette, SIGNAL(triggered(bool)), this, SLOT(onShowPaletteTriggered(bool)));
  connect(ui->actionShowNumTextures, SIGNAL(triggered(bool)), this, SLOT(onShowNumTexturesTriggered(bool)));
  connect(ui->actionShowFrameRate, SIGNAL(triggered(bool)), this, SLOT(onShowFrameRateTriggered(bool)));
  connect(ui->actionAutoRotation, SIGNAL(triggered(bool)), this, SLOT(onAutoRotationTriggered(bool)));
  connect(ui->actionTimeSteps, SIGNAL(triggered()), this, SLOT(onTimeStepsTriggered()));

  // misc.
  connect(_canvas, SIGNAL(newVolDesc(vvVolDesc*)), this, SLOT(onNewVolDesc(vvVolDesc*)));

  connect(_timeStepDialog, SIGNAL(valueChanged(int)), _canvas, SLOT(setTimeStep(int)));
  connect(_timeStepDialog, SIGNAL(play(double)), _canvas, SLOT(startAnimation(double)));
  connect(_timeStepDialog, SIGNAL(pause()), _canvas, SLOT(stopAnimation()));
  connect(_timeStepDialog, SIGNAL(back()), _canvas, SLOT(decTimeStep()));
  connect(_timeStepDialog, SIGNAL(fwd()), _canvas, SLOT(incTimeStep()));
  connect(_timeStepDialog, SIGNAL(first()), _canvas, SLOT(firstTimeStep()));
  connect(_timeStepDialog, SIGNAL(last()), _canvas, SLOT(lastTimeStep()));
  connect(_canvas, SIGNAL(currentFrame(int)), _timeStepDialog, SLOT(setCurrentFrame(int)));
}

vvMainWindow::~vvMainWindow()
{
  vvDebugMsg::msg(1, "vvMainWindow::~vvMainWindow()");
}

void vvMainWindow::loadVolumeFile(const QString& filename)
{
  QByteArray ba = filename.toLatin1();
  vvVolDesc* vd = new vvVolDesc(ba.data());
  vvFileIO* fio = new vvFileIO;
  switch (fio->loadVolumeData(vd, vvFileIO::ALL_DATA))
  {
  case vvFileIO::OK:
  {
    vvDebugMsg::msg(2, "Loaded file: ", ba.data());
    // use default TF if none stored
    if (vd->tf.isEmpty())
    {
      vd->tf.setDefaultAlpha(0, vd->real[0], vd->real[1]);
      vd->tf.setDefaultColors((vd->chan == 1) ? 0 : 2, vd->real[0], vd->real[1]);
    }
    if (vd->bpc == 4 && vd->real[0] == 0.0f && vd->real[1] == 1.0f)
    {
      vd->setDefaultRealMinMax();
    }
    _canvas->setVolDesc(vd);
    _dimensionDialog->setInitialDist(vd->dist);
    break;
  }
  case vvFileIO::FILE_NOT_FOUND:
    vvDebugMsg::msg(2, "File not found: ", ba.data());
    delete vd;
    QMessageBox::warning(this, tr("Error loading file"), tr("File not found: ") + filename, QMessageBox::Ok);
    break;
  default:
    vvDebugMsg::msg(2, "Cannot load file: ", ba.data());
    delete vd;
    QMessageBox::warning(this, tr("Error loading file"), tr("Cannot load file: ") + filename, QMessageBox::Ok);
    break;
  }
}

void vvMainWindow::mergeFiles(const QString& firstFile, const int num, const int increment, vvVolDesc::MergeType mergeType)
{
  vvDebugMsg::msg(1, "vvMainWindow::mergeFiles()");

  QByteArray ba = firstFile.toLatin1();
  vvVolDesc* vd = new vvVolDesc(ba.data());
  vvFileIO* fio = new vvFileIO;
  switch (fio->mergeFiles(vd, num, increment, mergeType))
  {
  case vvFileIO::OK:
    vvDebugMsg::msg(2, "Loaded slice sequence: ", vd->getFilename());
    // use default TF if non stored
    if (vd->tf.isEmpty())
    {
      vd->tf.setDefaultAlpha(0, vd->real[0], vd->real[1]);
      vd->tf.setDefaultColors((vd->chan == 1) ? 0 : 2, vd->real[0], vd->real[1]);
    }
    if (vd->bpc == 4 && vd->real[0] == 0.0f && vd->real[1] == 1.0f)
    {
      vd->setDefaultRealMinMax();
    }
    _canvas->setVolDesc(vd);
    break;
  case vvFileIO::FILE_NOT_FOUND:
    vvDebugMsg::msg(2, "File not found: ", ba.data());
    delete vd;
    QMessageBox::warning(this, tr("Error merging file"), tr("File not found: ") + firstFile, QMessageBox::Ok);
    break;
  default:
    vvDebugMsg::msg(2, "Cannot merge file: ", ba.data());
    delete vd;
    QMessageBox::warning(this, tr("Error merging file"), tr("Cannot merge file: ") + firstFile, QMessageBox::Ok);
    break;
  }
}

void vvMainWindow::onLoadVolumeTriggered()
{
  vvDebugMsg::msg(3, "vvMainWindow::onLoadVolumeTriggered()");

  QString caption = tr("Load Volume File");
  QString dir;
  QString filter = tr("All Volume Files (*.rvf *.xvf *.avf *.tif *.tiff *.hdr *.volb);;"
    "3D TIF Files (*.tif,*.tiff);;"
    "ASCII Volume Files (*.avf);;"
    "Extended Volume Files (*.xvf);;"
    "Raw Volume Files (*.rvf);;"
    "All Files (*.*)");
  QString filename = QFileDialog::getOpenFileName(this, caption, dir, filter);
  if (!filename.isEmpty())
  {
    loadVolumeFile(filename);
  }
  else
  {
    QMessageBox::warning(this, tr("Error loading file"), tr("File name is empty"), QMessageBox::Ok);
  }
}

void vvMainWindow::onReloadVolumeTriggered()
{
  vvDebugMsg::msg(3, "vvMainWindow::onReloadVolumeTriggered()");

  vvVolDesc* vd = _canvas->getVolDesc();
  if (vd != NULL)
  {
    loadVolumeFile(vd->getFilename());
  }
}

void vvMainWindow::onSaveVolumeAsTriggered()
{
  vvDebugMsg::msg(3, "vvMainWindow::onSaveVolumeTriggered()");

  QString caption = tr("Save Volume");
  QString dir;
  QString filter = tr("All Volume Files (*.xvf *.rvf *.avf);;"
    "Extended Volume Files (*.xvf);;"
    "Raw Volume Files (*.rvf);;"
    "ASCII Volume Files (*.avf);;"
    "All Files (*.*)");
  QString filename = QFileDialog::getSaveFileName(this, caption, dir, filter);
  if (!filename.isEmpty())
  {
    vvFileIO* fio = new vvFileIO;
    QByteArray ba = filename.toLatin1();
    _canvas->getVolDesc()->setFilename(ba.data());
    switch (fio->saveVolumeData(_canvas->getVolDesc(), true))
    {
    case vvFileIO::OK:
      vvDebugMsg::msg(2, "Volume saved as ", _canvas->getVolDesc()->getFilename());
      break;
    default:
      vvDebugMsg::msg(0, "Unhandled error saving ", _canvas->getVolDesc()->getFilename());
      break;
    }
  }
}

void vvMainWindow::onMergeFilesTriggered()
{
  vvDebugMsg::msg(3, "vvMainWindow::onMergeFilesTriggered()");

  if (_mergeDialog->exec() == QDialog::Accepted)
  {
    const QString filename = _mergeDialog->getFilename();

    int numFiles = 0;
    if (_mergeDialog->numFilesLimited())
    {
      numFiles = _mergeDialog->getNumFiles();
    }

    int increment = 0;
    if (_mergeDialog->filesNumbered())
    {
      increment = _mergeDialog->getFileIncrement();
    }

    const vvVolDesc::MergeType mergeType = _mergeDialog->getMergeType();

    mergeFiles(filename, numFiles, increment, mergeType);
  }
}

void vvMainWindow::onLoadCameraTriggered()
{
  vvDebugMsg::msg(3, "vvMainWindow::onLoadCameraTriggered()");

  QString caption = tr("Load Camera File");
  QString dir;
  QString filter = tr("Camera Files (*cam);;"
    "All Files (*.*)");
  QString filename = QFileDialog::getOpenFileName(this, caption, dir, filter);
  if (!filename.isEmpty())
  {
    _canvas->loadCamera(filename);
  }
}

void vvMainWindow::onSaveCameraAsTriggered()
{
  vvDebugMsg::msg(3, "vvMainWindow::onSaveCameraAsTriggered()");

  QString caption = tr("Save Camera to File");
  QString dir = "camera.cam";
  QString filter = tr("Camera Files (*cam);;"
    "All Files (*.*)");
  QString filename = QFileDialog::getSaveFileName(this, caption, dir, filter);
  if (!filename.isEmpty())
  {
    _canvas->saveCamera(filename);
  }
}

void vvMainWindow::onScreenshotTriggered()
{
  vvDebugMsg::msg(3, "vvMainWindow::onScreenshotTriggered()");

  _screenshotDialog->raise();
  _screenshotDialog->show();
}

void vvMainWindow::onTransferFunctionTriggered()
{
  vvDebugMsg::msg(3, "vvMainWindow::onTransferFunctionTriggered()");

  _tfDialog->raise();
  _tfDialog->show();
}

void vvMainWindow::onBackgroundColorTriggered()
{
  vvDebugMsg::msg(3, "vvMainWindow::onBackgroundColorTriggered()");

  QColor qcolor = QColorDialog::getColor();
  vvColor color(qcolor.redF(), qcolor.greenF(), qcolor.blueF());
  _canvas->setParameter(vvCanvas::VV_BG_COLOR, color);
  QSettings settings;
  settings.setValue("canvas/bgcolor", qcolor);
}

void vvMainWindow::onSampleDistancesTriggered()
{
  vvDebugMsg::msg(3, "vvMainWindow::onSampleDistancesTriggered()");

  _dimensionDialog->raise();
  _dimensionDialog->show();
}

void vvMainWindow::onShowOrientationTriggered(bool checked)
{
  vvDebugMsg::msg(3, "vvMainWindow::onShowOrientationTriggered()");

  _canvas->getRenderer()->setParameter(vvRenderState::VV_ORIENTATION, checked);
}

void vvMainWindow::onShowBoundariesTriggered(bool checked)
{
  vvDebugMsg::msg(3, "vvMainWindow::onShowBoundariesTriggered()");

  _canvas->getRenderer()->setParameter(vvRenderState::VV_BOUNDARIES, checked);
}

void vvMainWindow::onShowPaletteTriggered(bool checked)
{
  vvDebugMsg::msg(3, "vvMainWindow::onShowPaletteTriggered()");

  _canvas->getRenderer()->setParameter(vvRenderState::VV_PALETTE, checked);
}

void vvMainWindow::onShowNumTexturesTriggered(bool checked)
{
  vvDebugMsg::msg(3, "vvMainWindow::onShowNumTexturesTriggered()");

  _canvas->getRenderer()->setParameter(vvRenderState::VV_QUALITY_DISPLAY, checked);
}

void vvMainWindow::onShowFrameRateTriggered(bool checked)
{
  vvDebugMsg::msg(3, "vvMainWindow::onShowFrameRateTriggered()");

  _canvas->getRenderer()->setParameter(vvRenderState::VV_FPS_DISPLAY, checked);
}

void vvMainWindow::onAutoRotationTriggered(bool)
{
  vvDebugMsg::msg(3, "vvMainWindow::onAutoRotationTriggered()");
}

void vvMainWindow::onTimeStepsTriggered()
{
  vvDebugMsg::msg(3, "vvMainWindow::onTimeStepsTriggered()");

  _timeStepDialog->raise();
  _timeStepDialog->show();
}

void vvMainWindow::onNewVolDesc(vvVolDesc* vd)
{
  vvDebugMsg::msg(3, "vvMainWindow::onNewVolDesc()");

  _timeStepDialog->setFrames(vd->frames);
}

int main(int argc, char** argv)
{
  vvDebugMsg::setDebugLevel(0);

  QApplication a(argc, argv);
  vvMainWindow win;
  win.show();
  return a.exec();
}

