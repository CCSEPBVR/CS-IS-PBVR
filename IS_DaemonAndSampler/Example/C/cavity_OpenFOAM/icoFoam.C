/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2021 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    icoFoam

Description
    Transient solver for incompressible, laminar flow of Newtonian fluids.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "pisoControl.H"
#include "kvs_wrapper.h"
#include "interpolation.H"
#include <mpi.h>
//#include <vtkPointData.h>
//#include <vtkCellData.h>
//#include <vtkCellArray.h>
//#include <vtkHexahedron.h>
//#include <vtkWedge.h>
//#include <vtkPyramid.h>
//#include <vtkTetra.h>
//#include <vtkFloatArray.h>
//#include <vtkSmartPointerBase.h>
//#include <vtkSmartPointer.h>
//#include <vtkUnstructuredGrid.h>
//#include <vtkCellDataToPointData.h>
//#include "vector.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void dataConversion(const volVectorField U , const polyMesh& mesh, const int time_step)
{

   int ncoords = mesh.points().size();
   int ncells = mesh.cells().size();
   int nvariables = 3;
   float coords[3 * ncoords];
   float** values ; // u,v,w
   int cell_point =8 ; //hexahedra

//    Info << " nCoords = " <<mesh.points().size() <<endl;
//    Info << " nCells = " <<mesh.cells().size() <<endl;
//    Info << " nU = " << U.size() << endl;

    values = new float*[nvariables]; 
    for (int i = 0; i< nvariables ; i++)
    {
        values[i] = new float[ncoords]; 
        memset( values[i], 0, sizeof( float ) * ncoords ); 
    }

   domain_parameters dom = {
    0,
    0,
    0,
    1,
    1,
    0.5 
    };


   // vtkライブラリへの換装は混合要素対応をマージした後
    std::vector<float> add_count(ncoords,0);
    forAll(mesh.cells(),cid)
    {
        labelList of_connect = mesh.cellShapes()[cid]; 
        forAll(of_connect, con_id)
        {
           label pid = of_connect[con_id];
           values[0][pid] += U[cid].x(); 
           values[1][pid] += U[cid].y(); 
           values[2][pid] += U[cid].z(); 
           add_count[pid] += 1; 
        }
    }

    forAll(mesh.points(),pid)
    {
        values[0][pid] /= add_count[pid]; 
        values[1][pid] /= add_count[pid]; 
        values[2][pid] /= add_count[pid]; 
    }

 //座標
    forAll(mesh.points(),pid)
    {
        coords[3*pid  ] = mesh.points()[pid][0];
        coords[3*pid+1] = mesh.points()[pid][1];
        coords[3*pid+2] = mesh.points()[pid][2];
    }

    std::vector<unsigned int> connect;
    forAll(mesh.cells(), cid) //接続情報の並べ替え
    {
       labelList of_connect = mesh.cellShapes()[cid]; // Foam::List<int>
       forAll(mesh.cellShapes()[cid], cpid)
       {
           connect.push_back(of_connect[cpid]); 
       } 
    }        

   generate_particles( time_step, dom,
           values, nvariables,
           coords, ncoords,
           connect.data(), ncells, pbvr::VolumeObjectBase::CellType::Hexahedra );

}

int main(int argc, char *argv[])
{
    #include "setRootCaseLists.H"
    #include "createTime.H"
    #include "createMesh.H"

    pisoControl piso(mesh);

    #include "createFields.H"
    #include "initContinuityErrs.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nStarting time loop\n" << endl;

    int step = 0;
    int output_step = 0;
    while (runTime.loop())
    {
        Info<< "Time = " << runTime.userTimeName() << nl << endl;

        #include "CourantNo.H"

        // Momentum predictor

        fvVectorMatrix UEqn
        (
            fvm::ddt(U)
          + fvm::div(phi, U)
          - fvm::laplacian(nu, U)
        );

        if (piso.momentumPredictor())
        {
            solve(UEqn == -fvc::grad(p));
        }

        // --- PISO loop
        while (piso.correct())
        {
            volScalarField rAU(1.0/UEqn.A());
            volVectorField HbyA(constrainHbyA(rAU*UEqn.H(), U, p));
            surfaceScalarField phiHbyA
            (
                "phiHbyA",
                fvc::flux(HbyA)
              + fvc::interpolate(rAU)*fvc::ddtCorr(U, phi)
            );

            adjustPhi(phiHbyA, U, p);

            // Update the pressure BCs to ensure flux consistency
            constrainPressure(p, U, phiHbyA, rAU);

            // Non-orthogonal pressure corrector loop
            while (piso.correctNonOrthogonal())
            {
                // Pressure corrector

                fvScalarMatrix pEqn
                (
                    fvm::laplacian(rAU, p) == fvc::div(phiHbyA)
                );

                pEqn.setReference(pRefCell, pRefValue);

                pEqn.solve();

                if (piso.finalNonOrthogonalIter())
                {
                    phi = phiHbyA - pEqn.flux();
                }
            }

            #include "continuityErrs.H"

            U = HbyA - rAU*fvc::grad(p);
            U.correctBoundaryConditions();
        }

        runTime.write();

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;

        if (step % 10 == 0)
        {
            dataConversion(U, mesh, output_step);
            output_step++;
        }
        step ++ ;
        sleep (5);
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
