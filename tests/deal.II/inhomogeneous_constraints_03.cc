//------------------  inhomogeneous_constraints.cc  ------------------------
//    $Id: inhomogeneous_constraints.cc 23710 2011-05-17 04:50:10Z bangerth $
//    Version: $Name$ 
//
//    Copyright (C) 2009 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//------------------  inhomogeneous_constraints.cc  ------------------------


// We take two cells with two common dofs and create the system_matrix
// and the right-hand-side-vector for this system.  But we have the following
// two inhomogeneous constraints:
//       x_1 = -5   and   x_4 = 2*x_0 + 1
// And we want to test if the distribute_local_to_global function supplies the correct
// right-hand-side-vector if the use_inhomogeneities_for_rhs-parameter
// is set to true or false. The problem is that for the 4th component of the rhs-vector
// it is not possible to compute the correct value because of the unknown x_0. So
// for the 4th component of the rhs-vector distribute_local_to_global fill in
// 1*4 ( mat(4,4)*inhomogeneity(4) ).

#include "../tests.h"

#include <deal.II/base/quadrature_lib.h>
#include <deal.II/base/function.h>
#include <deal.II/base/logstream.h>
#include <deal.II/base/utilities.h>
#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/solver_cg.h>
#include <deal.II/lac/precondition.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/grid/grid_refinement.h>
#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/lac/constraint_matrix.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/numerics/vectors.h>
#include <deal.II/numerics/matrices.h>
#include <deal.II/numerics/error_estimator.h>
#include <deal.II/lac/compressed_simple_sparsity_pattern.h>

#include <fstream>
#include <iostream>
#include <complex>

std::ofstream logfile("inhomogeneous_constraints_03/output");

using namespace dealii;

void test(bool use_inhomogeneity_for_rhs)
{
  ConstraintMatrix cm;

  cm.add_line(1);
  cm.set_inhomogeneity(1, -5.0);
  cm.add_line(4);
  cm.add_entry(4, 0, 2.0);
  cm.set_inhomogeneity(4, 1.0);

  cm.close();
  cm.print(logfile);

  
  CompressedSimpleSparsityPattern csp(8,8);
  for (unsigned int i=0;i<8;++i)
    csp.add(i,i);
    
  SparsityPattern sp;
  sp.copy_from(csp);
  SparseMatrix<double> mat(sp);
  Vector<double> rhs(8);

  // "assemble":

  std::vector<unsigned int> local_dofs1;
  for (unsigned int i=0;i<5;++i)
    local_dofs1.push_back(i);

  std::vector<unsigned int> local_dofs2;
  local_dofs2.push_back(1);
  for (unsigned int i=1;i<5;++i)
    local_dofs2.push_back(3+i);

  FullMatrix<double> local_mat(5,5);
  Vector<double> local_vec(5);
  for (unsigned int i=0;i<5;++i)
    local_mat(i,i)=2.0;
  
  local_vec = 1;

  cm.distribute_local_to_global(local_mat, local_vec, local_dofs1, mat, rhs, use_inhomogeneity_for_rhs);
  cm.distribute_local_to_global(local_mat, local_vec, local_dofs2, mat, rhs, use_inhomogeneity_for_rhs);


  mat.print(logfile);
  rhs.print(logfile);

  Vector<double> solution(8);
  for (unsigned int i=0;i<8;++i)
    {
      solution(i)=rhs(i)/mat(i,i);
    }

  solution.print(logfile);
  cm.distribute(solution);
  solution.print(logfile);  
}


int main () 
{
  deallog << std::setprecision (2);
  logfile << std::setprecision (2);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  // Use the constraints for the right-hand-side
  {
    test(true);	
  }

  // Don not use the constraints for the right-hand-side
  {
    test(false);
  }
}