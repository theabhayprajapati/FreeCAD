/***************************************************************************
 *   Copyright (c) 2016 WandererFan <wandererfan@gmail.com>                *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef _DrawViewImage_h_
#define _DrawViewImage_h_

#include <Mod/TechDraw/TechDrawGlobal.h>

#include <App/DocumentObject.h>
#include <App/FeaturePython.h>
#include <App/PropertyFile.h>
#include <Base/BoundBox.h>

#include "DrawView.h"


namespace TechDraw
{

class TechDrawExport DrawViewImage : public TechDraw::DrawView
{
    PROPERTY_HEADER_WITH_OVERRIDE(TechDraw::DrawViewImage);

public:
    /// Constructor
    DrawViewImage();
    ~DrawViewImage() override;

    App::PropertyFile         ImageFile;
    App::PropertyFileIncluded ImageIncluded;
    App::PropertyFloat        Width;
    App::PropertyFloat        Height;

    /** @name methods override Feature */
    //@{
    /// recalculate the Feature
    App::DocumentObjectExecReturn *execute() override;
    //@}

    short mustExecute() const override;

    /// returns the type name of the ViewProvider
    const char* getViewProviderName() const override {
        return "TechDrawGui::ViewProviderImage";
    }
    QRectF getRect() const override;

protected:
    void setupImageIncluded();
    void replaceImageIncluded(std::string newFileName);

    void onChanged(const App::Property* prop) override;
    Base::BoundBox3d bbox;
};

using DrawViewImagePython = App::FeaturePythonT<DrawViewImage>;


} //namespace TechDraw


#endif
