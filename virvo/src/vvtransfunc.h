// Virvo - Virtual Reality Volume Rendering
// Copyright (C) 1999-2003 University of Stuttgart, 2004-2005 Brown University
// Contact: Jurgen P. Schulze, schulze@cs.brown.edu
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

#ifndef _VVTRANSFUNC_H_
#define _VVTRANSFUNC_H_

#include "vvexport.h"
#include "vvtoolshed.h"
#include "vvtfwidget.h"

/** Description of a transfer function.
  @author Jurgen P. Schulze (jschulze@ucsd.edu)
  @see vvTFWidget
*/
class VIRVOEXPORT vvTransFunc
{
  private:
    enum                                          /// number of elements in ring buffer
    {
      BUFFER_SIZE = 20
    };
    vvSLList<vvTFWidget*> _buffer[BUFFER_SIZE];   ///< ring buffer which can be used to implement Undo functionality
    int _nextBufferEntry;                         ///< index of next ring buffer entry to use for storage
    int _bufferUsed;                              ///< number of ring buffer entries used
    int _discreteColors;                          ///< number of discrete colors to use for color interpolation (0 for smooth colors)

  public:
    enum WidgetType
    {
      TF_COLOR,
      TF_PYRAMID,
      TF_BELL,
      TF_SKIP,
      TF_CUSTOM
    };
    vvSLList<vvTFWidget*> _widgets;  ///< TF widget list

    vvTransFunc();
    vvTransFunc(vvTransFunc*);
    virtual ~vvTransFunc();
    bool isEmpty();
    void deleteColorWidgets();
    void setDefaultColors(int=0);
    int  getNumDefaultColors();
    void setDefaultAlpha(int=0);
    int  getNumDefaultAlpha();
    int  getNumWidgets(WidgetType);
    void deleteWidgets(WidgetType);
    void computeTFTexture(int, int, int, float*);
    vvColor computeBGColor(float, float, float);
    vvColor computeColor(float, float=0.0f, float=0.0f);
    float computeOpacity(float, float=0.0f, float=0.0f);
    void makeColorBar(int, uchar*);
    void makeAlphaTexture(int, int, uchar*);
    void make2DTFTexture(int, int, uchar*);
    void make8bitLUT(int, uchar*);
    void makeFloatLUT(int, float*);
    void makePreintLUTOptimized(int width, uchar *preintLUT, float thickness=1.0);
    void makePreintLUTCorrect(int width, uchar *preintLUT, float thickness=1.0);
    static void copy(vvSLList<vvTFWidget*>*, vvSLList<vvTFWidget*>*);
    void putUndoBuffer();
    void getUndoBuffer();
    void clearUndoBuffer();
    void setDiscreteColors(int);
    int  getDiscreteColors();
    int  saveMeshviewer(const char*);
    int  loadMeshviewer(const char*);
};
#endif

//============================================================================
// End of File
//============================================================================
