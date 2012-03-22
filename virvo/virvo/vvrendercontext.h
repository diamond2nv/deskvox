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

#ifndef _VV_RENDERCONTEXT_H_
#define _VV_RENDERCONTEXT_H_

#include "vvexport.h"
#include "vvinttypes.h"

#include <iostream>

/*! Has to be defined in .cpp
 *  contains architecture specific variables
 */
struct ContextArchData;

/**
  Class handling render contexts

  @author Stefan Zellmann (zellmans@uni-koeln.de)
  @author Stavros Delisavas (stavros.delisavas@uni-koeln.de)
  */
class VIRVOEXPORT vvRenderContext
{
public:
  enum ContextType
  {
    VV_WINDOW,
    VV_PBUFFER
  };

//  enum ContextWindow
//  {
//    VV_X11,
//    VV_WINAPI
//  };

  struct ContextOptions
  {
    vvRenderContext::ContextType type;  ///< context type
    std::string displayName;            ///< name of display e.g. ":0" leave empty for default
    uint width;                         ///< width of rendercontext (and windows if related)
    uint height;                        ///< height of rendercontext (and windows if related)
    bool doubleBuffering;               ///< flag indicating usage of double-buffering
  };

  /**
    Creates a render context with given context options.
    Call makeCurrent() to enable context for rendering
    */
  vvRenderContext(ContextOptions*);
  ~vvRenderContext();

  /**
    Make this rendercontext current (in according thread)
    @return true on success, false on error
    */
  bool makeCurrent() const;
  /**
    Swap buffers in case of double-buffering
    */
  void swapBuffers() const;
private:
  ContextArchData *_archData;
  ContextOptions  *_options;

  bool _initialized;

  void init();
};

#endif // _VV_RENDERCONTEXT_H_
// vim: sw=2:expandtab:softtabstop=2:ts=2:cino=\:0g0t0
