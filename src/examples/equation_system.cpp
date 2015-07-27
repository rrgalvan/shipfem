// equation_system.cpp ---

// Copyright (C) 2015 Rafa Rodríguez Galván <rafaelDOTrodriguezATucaDOTes>

// Author: Rafa Rodríguez Galván

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include <chrono>  // std time handling (C++11)
#include <shfem.hpp>
using namespace shfem;

#include <Eigen/Dense>
using namespace Eigen;

Real rhs_function(Real x, Real y) {
  // Right hand side function
  return -x*x-y*y;
}

int main()
{
  // Try to read mesh contents from a .msh file (for information about the
  // structure of these files, see FreeFem++ documentation).
  TriangleMesh mesh;
  try {
    mesh.read_file_msh("circle200.msh"); }
  catch (...) {
    std::cerr << "Error reading mesh file" << std::endl;
    exit(1); }

  // Define quadrature rule with nodes located at vertices of the reference triangle
  VerticesQuadRule quad_rule;

  // Define a finite element space on current mesh
  P1_FE_Space fe_space(mesh, quad_rule);

  // Global finite element matrix
  int N = fe_space.get_ndofs();
  MatrixXf A(N, N);

  // Global finite element rhs vector
  VectorXf b(N);

  // Start chronometer
  auto start = std::chrono::high_resolution_clock::now();

  // For each cell, r:
  for (Index r=0; r<mesh.get_ncel(); ++r)
    {
      auto fe = fe_space.get_element(r); // Build a finite element on cell r

      // Get x-derivatives of all the basis functions of curent element
      const std::vector<FE_Function>& dx_phi = fe.get_dx_phi();
      // Get also y-derivatives of basis functions
      const std::vector<FE_Function>& dy_phi = fe.get_dy_phi();

      // Local stiffness matrix
      Index ndofs = fe.get_ndofs();
      MatrixXf A_r(ndofs,ndofs);

      // Local rhs vector
      VectorXf b_r(ndofs);

      // For each degree of freedom, i
      for (Index i = 0; i < ndofs; ++i)
	{
	  // For each degree of freedom, j
	  for (Index j = 0; j < ndofs; ++j)
	    {
	      // Compute integral of product of x-derivatives
	      Real Ax_ij = fe.integrate(dx_phi[i], dx_phi[j]);

	      // Compute integral of product of y-derivatives
	      Real Ay_ij = fe.integrate(dy_phi[i], dy_phi[j]);

	      // Store the result in the local matrix
	      A_r(i,j) = Ax_ij + Ay_ij;
	    }

	  // Store also the i-th element in the rhs vector
	  Real rhs_i = fe.integrate(dx_phi[i]);
	  b_r(i) = rhs_i;

	}

      // Add local matrix A_r into global matrix A
      fe_space.assemble_matrix(A_r, A);

      // Add local vector b_r into global rhs vector b
      fe_space.assemble_vector(b_r, b);
    }

  // Print matrix assembiling time
  auto end_matrix = std::chrono::high_resolution_clock::now();
  int elapsed_time =
    std::chrono::duration_cast<std::chrono::milliseconds>( end_matrix - start ).count();
  std::cout << "Matrix assembled. Elapsed time: " << elapsed_time << " miliseconds" << std::endl;

  std::cout << "Resulting matrix:" << std::endl << A << std::endl;
  std::cout << "Resulting vector:" << std::endl << b << std::endl;
}
