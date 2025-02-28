/***************************************************************************
 *   Copyright (c) 2022 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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


#include "PreCompiled.h"
#ifndef _PreComp_
# include <ostream>
# include <sstream>
#endif


#include "Writer3MF.h"
#include "Core/Evaluation.h"
#include "Core/MeshKernel.h"
#include <Base/Tools.h>

using namespace MeshCore;

Writer3MF::Writer3MF(std::ostream &str)
  : zip(str)
  , objectIndex(0)
{
    zip.putNextEntry("3D/3dmodel.model");
    Initialize(zip);
}

Writer3MF::Writer3MF(const std::string &filename)
  : zip(filename)
  , objectIndex(0)
{
    zip.putNextEntry("3D/3dmodel.model");
    Initialize(zip);
}

void Writer3MF::Initialize(std::ostream &str)
{
    str << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           "<model unit=\"millimeter\" xml:lang=\"en-US\" xmlns=\"http://schemas.microsoft.com/3dmanufacturing/core/2015/02\">\n"
           " <metadata name=\"Application\">FreeCAD</metadata>\n";
    str << Base::blanks(1) << "<resources>\n";
}

void Writer3MF::Finish(std::ostream &str)
{
    str << Base::blanks(1) << "</resources>\n";
    str << Base::blanks(1) << "<build>\n";
    for (const auto& it : items)
        str << Base::blanks(2) << it;
    str << Base::blanks(1) << "</build>\n";
    str << "</model>\n";
}

bool Writer3MF::AddMesh(const MeshKernel& mesh, const Base::Matrix4D& mat)
{
    int id = ++objectIndex;
    SaveBuildItem(id, mat);
    return SaveObject(zip, id, mesh);
}

bool Writer3MF::Save()
{
    Finish(zip);
    zip.closeEntry();

    zip.putNextEntry("_rels/.rels");
    if (!SaveRels(zip))
        return false;
    zip.closeEntry();

    zip.putNextEntry("[Content_Types].xml");
    if (!SaveContent(zip))
        return false;
    zip.closeEntry();
    return true;
}

bool Writer3MF::SaveObject(std::ostream &str, int id, const MeshKernel& mesh) const
{
    const MeshPointArray& rPoints = mesh.GetPoints();
    const MeshFacetArray& rFacets = mesh.GetFacets();

    if (!str || str.bad())
        return false;

    str << Base::blanks(2) << "<object id=\"" << id << "\" type=\"" << GetType(mesh) << "\">\n";
    str << Base::blanks(3) << "<mesh>\n";

    // vertices
    str << Base::blanks(4) << "<vertices>\n";
    std::size_t index = 0;
    for (MeshPointArray::_TConstIterator it = rPoints.begin(); it != rPoints.end(); ++it, ++index) {
        str << Base::blanks(5) << "<vertex x=\"" << it->x
                                    << "\" y=\"" << it->y
                                    << "\" z=\"" << it->z
                                    << "\" />\n";
    }
    str << Base::blanks(4) << "</vertices>\n";

    // facet indices
    str << Base::blanks(4) << "<triangles>\n";
    for (MeshFacetArray::_TConstIterator it = rFacets.begin(); it != rFacets.end(); ++it) {
        str << Base::blanks(5) << "<triangle v1=\"" << it->_aulPoints[0]
                                      << "\" v2=\"" << it->_aulPoints[1]
                                      << "\" v3=\"" << it->_aulPoints[2]
                                      << "\" />\n";
    }
    str << Base::blanks(4) << "</triangles>\n";

    str << Base::blanks(3) << "</mesh>\n";
    str << Base::blanks(2) << "</object>\n";

    return true;
}

std::string Writer3MF::GetType(const MeshKernel& mesh) const
{
    if (MeshEvalSolid(mesh).Evaluate())
        return "model";
    else
        return "surface";
}

void Writer3MF::SaveBuildItem(int id, const Base::Matrix4D& mat)
{
    std::stringstream str;
    str << "<item objectid=\"" << id << "\" transform=\"" << DumpMatrix(mat) << "\" />\n";
    items.push_back(str.str());
}

std::string Writer3MF::DumpMatrix(const Base::Matrix4D& mat) const
{
    // The matrix representation in the specs is the transposed version of Matrix4D
    // This means that for the 3x3 sub-matrix the indices must be swapped
    //
    // 3D Manufacturing Format / Core Specification & Reference Guide v1.2.3
    // Chapter: 3.3 3D Matrices (page 9)
    std::stringstream str;
    str << mat[0][0] << " " << mat[1][0] << " " << mat[2][0] << " "
        << mat[0][1] << " " << mat[1][1] << " " << mat[2][1] << " "
        << mat[0][2] << " " << mat[1][2] << " " << mat[2][2] << " "
        << mat[0][3] << " " << mat[1][3] << " " << mat[2][3];
    return str.str();
}

bool Writer3MF::SaveRels(std::ostream &str) const
{
    str << "<?xml version='1.0' encoding='UTF-8'?>\n"
        << "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
           "<Relationship Id=\"rel0\" Target=\"/3D/3dmodel.model\" Type=\"http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel\" />"
           "</Relationships>";
    return true;
}

bool Writer3MF::SaveContent(std::ostream &str) const
{
    str << "<?xml version='1.0' encoding='UTF-8'?>\n"
        << "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
           "<Default ContentType=\"application/vnd.openxmlformats-package.relationships+xml\" Extension=\"rels\" />"
           "<Default ContentType=\"application/vnd.ms-package.3dmanufacturing-3dmodel+xml\" Extension=\"model\" />"
           "</Types>";
    return true;
}
