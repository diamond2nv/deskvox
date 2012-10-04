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

#include <GL/glew.h>
#include <QUrl>

#include "vvcanvas.h"
#include "vvprefdialog.h"

#include "ui_vvprefdialog.h"

#include <virvo/vvbonjour/vvbonjour.h>
#include <virvo/vvdebugmsg.h>
#include <virvo/vvremoteevents.h>
#include <virvo/vvshaderfactory.h>
#include <virvo/vvsocketio.h>
#include <virvo/vvtoolshed.h>
#include <virvo/vvvirvo.h>

#include <QMessageBox>

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <utility>

#define VV_UNUSED(x) ((void)(x))

namespace
{
  std::map<int, vvRenderer::RendererType> rendererMap;
  std::map<int, std::string> texRendTypeMap;
  std::map<int, std::string> voxTypeMap;

  double movingSpinBoxOldValue = 1.0;
  double stillSpinBoxOldValue = 1.0;
  int movingDialOldValue = 0;
  int stillDialOldValue = 0;

  /* make qt dials behave as if they had an unlimited range
   */
  int getDialDelta(int oldval, int newval, int minval, int maxval)
  {
    const int eps = 10; // largest possible step from a single user action
    const int mineps = minval + eps;
    const int maxeps = maxval - eps;

    if (oldval < mineps && newval > maxeps)
    {
      return -(oldval + maxval + 1 - newval);
    }
    else if (oldval > maxeps && newval < mineps)
    {
      return maxval + 1 - oldval + newval;
    }
    else
    {
      return newval - oldval;
    }
  }
}

vvPrefDialog::vvPrefDialog(vvCanvas* canvas, QWidget* parent)
  : QDialog(parent)
  , ui(new Ui_PrefDialog)
  , _canvas(canvas)
{
  vvDebugMsg::msg(1, "vvPrefDialog::vvPrefDialog()");

  ui->setupUi(this);

  // can't be done in designer unfortunately
  QIcon ic = style()->standardIcon(QStyle::SP_MessageBoxInformation);
  ui->texInfoIconLabel->setPixmap(ic.pixmap(32, 32));

  _canvas->makeCurrent();
  glewInit(); // we need glCreateProgram etc. when checking for glsl support

  // renderer combo box
  int idx = 0;
  if (vvRendererFactory::hasRenderer(vvRenderer::TEXREND))
  {
    ui->rendererBox->addItem("OpenGL textures");
    rendererMap.insert(std::pair<int, vvRenderer::RendererType>(idx, vvRenderer::TEXREND));
    ++idx;
  }

  if (vvRendererFactory::hasRenderer(vvRenderer::RAYREND))
  {
    ui->rendererBox->addItem("CUDA ray casting");
    rendererMap.insert(std::pair<int, vvRenderer::RendererType>(idx, vvRenderer::RAYREND));
    ++idx;
  }

  if (vvRendererFactory::hasRenderer(vvRenderer::SOFTRAYREND))
  {
    ui->rendererBox->addItem("Software ray casting");
    rendererMap.insert(std::pair<int, vvRenderer::RendererType>(idx, vvRenderer::SOFTRAYREND));
    ++idx;
  }

  if (ui->rendererBox->count() <= 0)
  {
    ui->rendererBox->setEnabled(false);
  }

  // texrend geometry combo box
  idx = 0;

  if (vvRendererFactory::hasRenderer(vvRenderer::TEXREND))
  {
    ui->geometryBox->addItem("Autoselect");
    texRendTypeMap.insert(std::pair<int, std::string>(idx, "default"));
    ++idx;
  }

  if (vvRendererFactory::hasRenderer("slices"))
  {
    ui->geometryBox->addItem("2D textures (slices)");
    texRendTypeMap.insert(std::pair<int, std::string>(idx, "slices"));
    ++idx;
  }

  if (vvRendererFactory::hasRenderer("cubic2d"))
  {
    ui->geometryBox->addItem("2D textures (cubic)");
    texRendTypeMap.insert(std::pair<int, std::string>(idx, "cubic2d"));
    ++idx;
  }

  if (vvRendererFactory::hasRenderer("planar"))
  {
    ui->geometryBox->addItem("3D textures (viewport aligned)");
    texRendTypeMap.insert(std::pair<int, std::string>(idx, "planar"));
    ++idx;
  }

  if (vvRendererFactory::hasRenderer("spherical"))
  {
    ui->geometryBox->addItem("3D textures (spherical)");
    texRendTypeMap.insert(std::pair<int, std::string>(idx, "spherical"));
    ++idx;
  }

  // voxel type combo box
  idx = 0;

  if (vvRendererFactory::hasRenderer(vvRenderer::TEXREND))
  {
    ui->voxTypeBox->addItem("Autoselect");
    voxTypeMap.insert(std::pair<int, std::string>(idx, "default"));
    ++idx;
  }

  if (vvRendererFactory::hasRenderer(vvRenderer::TEXREND))
  {
    ui->voxTypeBox->addItem("RGBA");
    voxTypeMap.insert(std::pair<int, std::string>(idx, "rgba"));
    ++idx;
  }

  if (vvRendererFactory::hasRenderer(vvRenderer::TEXREND))
  {
    ui->voxTypeBox->addItem("ARB fragment program");
    voxTypeMap.insert(std::pair<int, std::string>(idx, "arb"));
    ++idx;
  }

  if (vvRendererFactory::hasRenderer(vvRenderer::TEXREND) && vvShaderFactory::isSupported("glsl"))
  {
    ui->voxTypeBox->addItem("GLSL fragment program");
    voxTypeMap.insert(std::pair<int, std::string>(idx, "shader"));
    ++idx;
  }

  // remote rendering page
  if (virvo::hasFeature("bonjour"))
  {
    ui->browseButton->setEnabled(true);
  }

  connect(ui->rendererBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onRendererChanged(int)));
  connect(ui->geometryBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTexRendOptionChanged(int)));
  connect(ui->voxTypeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTexRendOptionChanged(int)));
  connect(ui->pixShdBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTexRendOptionChanged(int)));
  connect(ui->hostEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onHostChanged(const QString&)));
  connect(ui->portBox, SIGNAL(valueChanged(int)), this, SLOT(onPortChanged(int)));
  connect(ui->getInfoButton, SIGNAL(clicked()), this, SLOT(onGetInfoClicked()));
  connect(ui->browseButton, SIGNAL(clicked()), this, SLOT(onBrowseClicked()));
  connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(onConnectClicked()));
  connect(ui->interpolationCheckBox, SIGNAL(toggled(bool)), this, SLOT(onInterpolationToggled(bool)));
  connect(ui->mipCheckBox, SIGNAL(toggled(bool)), this, SLOT(onMipToggled(bool)));
  connect(ui->movingSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onMovingSpinBoxChanged(double)));
  connect(ui->stillSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onStillSpinBoxChanged(double)));
  connect(ui->movingDial, SIGNAL(valueChanged(int)), this, SLOT(onMovingDialChanged(int)));
  connect(ui->stillDial, SIGNAL(valueChanged(int)), this, SLOT(onStillDialChanged(int)));
}

void vvPrefDialog::toggleInterpolation()
{
  vvDebugMsg::msg(3, "vvPrefDialog::toggletInterpolation()");

  const bool interpolation = ui->interpolationCheckBox->isChecked();
  ui->interpolationCheckBox->setChecked(!interpolation);
  emit parameterChanged(vvRenderer::VV_SLICEINT, !interpolation);
}

void vvPrefDialog::scaleStillQuality(const float s)
{
  vvDebugMsg::msg(3, "vvPrefDialog::scaleStillQuality()");

  assert(s >= 0.0f);

  float quality = static_cast<float>(ui->stillSpinBox->value());
  if (quality <= 0.0f)
  {
    // never let quality drop to or below 0
    quality = std::numeric_limits<float>::epsilon();
  }
  quality *= s;
  
  ui->stillSpinBox->setValue(quality);
}

void vvPrefDialog::emitRenderer()
{
  vvDebugMsg::msg(3, "vvPrefDialog::emitRenderer()");

  ui->texInfoLabel->setText("");
  std::string name = "";

  // indices to activate appropriate options tool box pages
  const int texidx = 0;
  const int rayidx = 1;

  vvRendererFactory::Options options;

  switch (rendererMap[ui->rendererBox->currentIndex()])
  {
  case vvRenderer::RAYREND:
    ui->optionsToolBox->setCurrentIndex(rayidx);
    name = "rayrend";
    break;
  case vvRenderer::SOFTRAYREND:
    ui->optionsToolBox->setCurrentIndex(rayidx);
    name = "softrayrend";
    break;
  case vvRenderer::TEXREND:
    ui->optionsToolBox->setCurrentIndex(texidx);
    name = texRendTypeMap[ui->geometryBox->currentIndex()];
    options["voxeltype"] = voxTypeMap[ui->voxTypeBox->currentIndex()];
    break;
  default:
    name = "default";
    break;
  }

  if (options["voxeltype"] == "rgba")
  {
    ui->texInfoLabel->setText(ui->texInfoLabel->text() + "<html><b>Voxel type RGBA</b><br />"
      "Pre-interpolative transfer function,"
      " is applied by assigning each voxel an RGBA color before rendering.</html>");
  }
  else if (options["voxeltype"] == "arb")
  {
    ui->texInfoLabel->setText(ui->texInfoLabel->text() + "<html><b>Voxel type ARB fragment program</b><br />"
      "Post-interpolative transfer function,"
      " is applied after sampling the volume texture.</html>");
  }
  else if (options["voxeltype"] == "shader")
  {
    ui->texInfoLabel->setText(ui->texInfoLabel->text() + "<html><b>Voxel type GLSL fragment program</b><br />"
      "Post-interpolative transfer function,"
      " is applied after sampling the volume texture.</html>");
  }

  if (name != "")
  {
    emit rendererChanged(name, options);
  }
}

bool vvPrefDialog::validateRemoteHost(const QString& host, const ushort port)
{
  int parsedPort = vvToolshed::parsePort(host.toStdString());
  if (parsedPort >= 0 && parsedPort <= std::numeric_limits<ushort>::max()
   && static_cast<ushort>(parsedPort) != port)
  {
    ui->portBox->setValue(parsedPort);
  }

  std::string h = (parsedPort == -1)
    ? host.toStdString()
    : vvToolshed::stripPort(host.toStdString());
  ushort p = static_cast<ushort>(ui->portBox->value());

  if (h == "")
  {
    return false;
  }

  QUrl url(h.c_str());
  url.setPort(p);
  return url.isValid();
}

void vvPrefDialog::onRendererChanged(const int index)
{
  vvDebugMsg::msg(3, "vvPrefDialog::onRendererChanged()");

  VV_UNUSED(index);
  assert(index == ui->rendererBox->currentIndex());
  emitRenderer();
}

void vvPrefDialog::onTexRendOptionChanged(const int index)
{
  vvDebugMsg::msg(3, "vvPrefDialog::onTexRendOptionChanged()");

  VV_UNUSED(index);

  if (rendererMap[ui->rendererBox->currentIndex()] == vvRenderer::TEXREND)
  {
    emitRenderer();
  }
}

void vvPrefDialog::onHostChanged(const QString& text)
{
  const ushort port = static_cast<ushort>(ui->portBox->value());
  if (validateRemoteHost(text, port))
  {
    ui->getInfoButton->setEnabled(true);
    ui->connectButton->setEnabled(true);
  }
  else
  {
    ui->getInfoButton->setEnabled(false);
    ui->connectButton->setEnabled(false);
  }
}

void vvPrefDialog::onPortChanged(const int i)
{
  const ushort port = static_cast<ushort>(i);
  if (validateRemoteHost(ui->hostEdit->text(), port))
  {
    ui->getInfoButton->setEnabled(true);
    ui->connectButton->setEnabled(true);
  }
  else
  {
    ui->getInfoButton->setEnabled(false);
    ui->connectButton->setEnabled(false);
  }
}

void vvPrefDialog::onGetInfoClicked()
{
  if (validateRemoteHost(ui->hostEdit->text(), static_cast<ushort>(ui->portBox->value())))
  {
    vvTcpSocket* sock = new vvTcpSocket;
    if (sock->connectToHost(ui->hostEdit->text().toStdString(),
      static_cast<ushort>(static_cast<ushort>(ui->portBox->value()))) == vvSocket::VV_OK)
    {
      sock->setParameter(vvSocket::VV_NO_NAGLE, true);
      vvSocketIO io(sock);
     // io.putInt32(virvo::Statistics);
     // float wload;
     // io.getFloat(wload);
     // int resCount;
     // io.getInt32(resCount);
     // std::cerr << "Total work-load " << wload << " caused with " << resCount << " resources." << endl;
      vvServerInfo info;
      io.putEvent(virvo::ServerInfo);
      io.getServerInfo(info);
      std::cerr << info.renderers << std::endl;
      io.putEvent(virvo::Exit);
    }
    else
    {
      QMessageBox::warning(this, tr("Failed to connect"), tr("Could not connect to host \"") + ui->hostEdit->text()
        + tr("\" on port \"") + QString::number(ui->portBox->value()) + tr("\""), QMessageBox::Ok);
    }
    delete sock;
  }
}

void vvPrefDialog::onBrowseClicked()
{
  if (virvo::hasFeature("bonjour"))
  {
    vvBonjour bonjour;
    std::vector<std::string> servers = bonjour.getConnectionStringsFor("_vserver._tcp");
    for (std::vector<std::string>::const_iterator it = servers.begin();
         it != servers.end(); ++it)
    {
      std::cerr << *it << std::endl;
    }
  }
}

void vvPrefDialog::onConnectClicked()
{
  if (validateRemoteHost(ui->hostEdit->text(), static_cast<ushort>(ui->portBox->value())))
  {
    vvTcpSocket* sock = new vvTcpSocket;
    if (sock->connectToHost(ui->hostEdit->text().toStdString(),
      static_cast<ushort>(static_cast<ushort>(ui->portBox->value()))) == vvSocket::VV_OK)
    {
      sock->setParameter(vvSocket::VV_NO_NAGLE, true);
      vvSocketIO io(sock);
     // io.putInt32(virvo::Statistics);
     // float wload;
     // io.getFloat(wload);
     // int resCount;
     // io.getInt32(resCount);
     // std::cerr << "Total work-load " << wload << " caused with " << resCount << " resources." << endl;
    }
    else
    {
      QMessageBox::warning(this, tr("Failed to connect"), tr("Could not connect to host \"") + ui->hostEdit->text()
        + tr("\" on port \"") + QString::number(ui->portBox->value()) + tr("\""), QMessageBox::Ok);
    }
    delete sock;
  }

}

void vvPrefDialog::onInterpolationToggled(const bool checked)
{
  vvDebugMsg::msg(3, "vvPrefDialog::onInterpolationToggled()");

  emit parameterChanged(vvRenderer::VV_SLICEINT, checked);
}

void vvPrefDialog::onMipToggled(bool checked)
{
  vvDebugMsg::msg(3, "vvPrefDialog::onMipToggled()");

  const int mipMode = checked ? 1 : 0; // don't support mip == 2 (min. intensity) for now
  emit parameterChanged(vvRenderer::VV_MIP_MODE, mipMode);
}

void vvPrefDialog::onMovingSpinBoxChanged(double value)
{
  vvDebugMsg::msg(3, "vvPrefDialog::onMovingSpinBoxChanged()");

  disconnect(ui->movingDial, SIGNAL(valueChanged(int)), this, SLOT(onMovingDialChanged(int)));
  const int upper = ui->movingDial->maximum() + 1;
  double d = value - movingSpinBoxOldValue;
  int di = round(d * upper);
  int dialval = ui->movingDial->value();
  dialval += di;
  dialval %= upper;
  while (dialval < ui->movingDial->minimum())
  {
    dialval += upper;
  }
  movingDialOldValue = dialval;
  ui->movingDial->setValue(dialval);
  movingSpinBoxOldValue = value;
  emit parameterChanged(vvParameters::VV_MOVING_QUALITY, static_cast<float>(ui->movingSpinBox->value()));
  connect(ui->movingDial, SIGNAL(valueChanged(int)), this, SLOT(onMovingDialChanged(int)));
}

void vvPrefDialog::onStillSpinBoxChanged(double value)
{
  vvDebugMsg::msg(3, "vvPrefDialog::onStillSpinBoxChanged()");

  disconnect(ui->stillDial, SIGNAL(valueChanged(int)), this, SLOT(onStillDialChanged(int)));
  const int upper = ui->stillDial->maximum() + 1;
  double d = value - stillSpinBoxOldValue;
  int di = round(d * upper);
  int dialval = ui->stillDial->value();
  dialval += di;
  dialval %= upper;
  while (dialval < ui->stillDial->minimum())
  {
    dialval += upper;
  }
  stillDialOldValue = dialval;
  ui->stillDial->setValue(dialval);
  stillSpinBoxOldValue = value;
  emit parameterChanged(vvRenderer::VV_QUALITY, static_cast<float>(ui->stillSpinBox->value()));
  connect(ui->stillDial, SIGNAL(valueChanged(int)), this, SLOT(onStillDialChanged(int)));
}

void vvPrefDialog::onMovingDialChanged(int value)
{
  vvDebugMsg::msg(3, "vvPrefDialog::onMovingDialChanged()");

  const int d = getDialDelta(movingDialOldValue, value, ui->movingDial->minimum(), ui->movingDial->maximum());
  const double dd = static_cast<double>(d) / static_cast<double>(ui->movingDial->maximum() + 1);
  disconnect(ui->movingSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onMovingSpinBoxChanged(double)));
  ui->movingSpinBox->setValue(ui->movingSpinBox->value() + dd);
  movingSpinBoxOldValue = ui->movingSpinBox->value();
  movingDialOldValue = value;
  emit parameterChanged(vvParameters::VV_MOVING_QUALITY, static_cast<float>(ui->movingSpinBox->value()));
  connect(ui->movingSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onMovingSpinBoxChanged(double)));
}

void vvPrefDialog::onStillDialChanged(int value)
{
  vvDebugMsg::msg(3, "vvPrefDialog::onStillDialChanged()");

  const int d = getDialDelta(stillDialOldValue, value, ui->stillDial->minimum(), ui->stillDial->maximum());
  const double dd = static_cast<double>(d) / static_cast<double>(ui->stillDial->maximum() + 1);
  disconnect(ui->stillSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onStillSpinBoxChanged(double)));
  ui->stillSpinBox->setValue(ui->stillSpinBox->value() + dd);
  stillSpinBoxOldValue = ui->stillSpinBox->value();
  stillDialOldValue = value;
  emit parameterChanged(vvRenderer::VV_QUALITY, static_cast<float>(ui->stillSpinBox->value()));
  connect(ui->stillSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onStillSpinBoxChanged(double)));
}

