/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkContourVtkMapper3D.h"
#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkOpenGLRenderer.h"


#include <vtkActor.h>
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkFollower.h>
#include <vtkAssembly.h>
#include <vtkProp3DCollection.h>
#include <vtkRenderer.h>
#include <vtkLinearTransform.h>
#include <vtkTubeFilter.h>
#include <vtkPolygon.h>


#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <stdlib.h>

mitk::ContourVtkMapper3D::ContourVtkMapper3D()
{
  m_VtkPolyDataMapper = vtkPolyDataMapper::New();
  m_Actor = vtkActor::New();
  m_Actor->SetMapper(m_VtkPolyDataMapper);

  m_Contour = NULL;
  m_TubeFilter = vtkTubeFilter::New();
}

mitk::ContourVtkMapper3D::~ContourVtkMapper3D()
{
  m_VtkPolyDataMapper->Delete();
  m_Actor->Delete();
  if(m_Contour!=NULL)
    m_Contour->Delete();
}

void mitk::ContourVtkMapper3D::GenerateData(mitk::BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false)
  {
    m_Actor->VisibilityOff();
    return;
  }
  m_Actor->VisibilityOn();

  if(m_Contour!=NULL)
    m_Contour->Delete();

  m_Contour = vtkPolyData::New();


  mitk::Contour::Pointer input  = const_cast<mitk::Contour*>(this->GetInput());
  bool makeContour = true;

  if (makeContour)
  {
    vtkPoints *points = vtkPoints::New();
    vtkCellArray *lines = vtkCellArray::New();

    int numPts=input->GetNumberOfPoints();
    if(numPts > 200000)
      numPts = 200000;
    mitk::Contour::PathPointer path = input->GetContourPath();
    mitk::Contour::PathType::InputType cstart = path->StartOfInput();
    mitk::Contour::PathType::InputType cend   = path->EndOfInput();
    mitk::Contour::PathType::InputType cstep  = (cend-cstart+1)/numPts;
    mitk::Contour::PathType::InputType ccur;

    vtkIdType ptIndex = 0;
    vtkIdType lastPointIndex = 0;

    mitk::Contour::PointsContainerPointer contourPoints = input->GetPoints();
    mitk::Contour::PointsContainerIterator pointsIt = contourPoints->Begin();

    float vtkpoint[3];
    unsigned int i;
    for(i=0, ccur=cstart; i<numPts; ++i, ccur+=cstep)
    {
      itk2vtk(path->Evaluate(ccur), vtkpoint);
      points->InsertPoint(ptIndex, vtkpoint);
      if (ptIndex>0)
      {
        int cell[2] = {ptIndex-1,ptIndex};
        lines->InsertNextCell((vtkIdType)2,(vtkIdType*) cell);
      }
      lastPointIndex = ptIndex;
      ptIndex++;
    }

    if (input->GetClosed())
    {
      int cell[2] = {lastPointIndex,0};
      lines->InsertNextCell((vtkIdType)2,(vtkIdType*) cell);
    }      

    m_Contour->SetPoints(points);
    points->Delete();
    m_Contour->SetLines(lines);
    m_Contour->Update();

    m_TubeFilter->SetInput(m_Contour);
    m_TubeFilter->SetRadius(1);
    m_TubeFilter->SetNumberOfSides(8);
    m_TubeFilter->Update();
    m_VtkPolyDataMapper->SetInput(m_TubeFilter->GetOutput());
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    double rgba[4]={0.0f,1.0f,0.0f,0.6f};
#else
    float rgba[4]={0.0f,1.0f,0.0f,0.6f};
#endif
    m_Actor->GetProperty()->SetColor(rgba);
    m_Actor->SetMapper(m_VtkPolyDataMapper);
  }
}

vtkProp* mitk::ContourVtkMapper3D::GetProp()
{
  return m_Actor;
}

const mitk::Contour* mitk::ContourVtkMapper3D::GetInput()
{
  return static_cast<const mitk::Contour* > ( GetData() );
}
