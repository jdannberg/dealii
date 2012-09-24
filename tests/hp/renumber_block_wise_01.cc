//----------------------------  renumber_block_wise_01.cc  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2000, 2001, 2003, 2004, 2009, 2012 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  renumber_block_wise_01.cc  ---------------------------


// Test DoFRenumbering::block_wise. For the element used here, it
// needs to produce the exact same numbering as that for
// DoFRenumber::component_wise



#include "../tests.h"
#include <deal.II/base/logstream.h>
#include <deal.II/base/function_lib.h>
#include <deal.II/lac/vector.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/hp/dof_handler.h>
#include <deal.II/dofs/dof_renumbering.h>
#include <deal.II/multigrid/mg_dof_accessor.h>
#include <deal.II/multigrid/mg_dof_handler.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_dgq.h>
#include <deal.II/fe/fe_dgp.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/hp/fe_collection.h>

#include <fstream>



template <int dim>
std::vector<unsigned int>
get_dofs (const hp::DoFHandler<dim> &dof)
{
  std::vector<unsigned int> local;
  std::vector<unsigned int> global;
  for (typename hp::DoFHandler<dim>::active_cell_iterator cell=dof.begin_active();
       cell != dof.end(); ++cell)
    {
      local.resize (cell->get_fe().dofs_per_cell);
      cell->get_dof_indices (local);

      global.insert (global.end(), local.begin(), local.end());
    }

  return global;
}



template <int dim>
void
check_renumbering(hp::DoFHandler<dim>& dof)
{
				   // do component-wise and save the
				   // results
  DoFRenumbering::component_wise (dof);
  const std::vector<unsigned int> vc = get_dofs (dof);

				   // now do the same with blocks
  DoFRenumbering::block_wise (dof);
  const std::vector<unsigned int> vb = get_dofs (dof);

  Assert (vc == vb, ExcInternalError());

  deallog << "OK" << std::endl;
}


template <int dim>
void
check ()
{
  Triangulation<dim> tr;
  if (dim==2)
    GridGenerator::hyper_ball(tr, Point<dim>(), 1);
  else
    GridGenerator::hyper_cube(tr, -1,1);
  tr.refine_global (1);
  tr.begin_active()->set_refine_flag ();
  tr.execute_coarsening_and_refinement ();
  if (dim==1)
    tr.refine_global(2);

  hp::DoFHandler<dim> dof(tr);
  {
    bool coin = false;
    for (typename hp::DoFHandler<dim>::active_cell_iterator cell=dof.begin_active();
	 cell != dof.end(); ++cell)
      {
	cell->set_active_fe_index (coin ? 0 : 1);
	coin = !coin;
      }
  }

  FESystem<dim> e1 (FE_Q<dim>(2), 2, FE_DGQ<dim>(1), 2);
  FESystem<dim> e2 (FE_Q<dim>(1), 2, FE_DGQ<dim>(2), 2);

  hp::FECollection<dim> fe_collection;
  fe_collection.push_back (e1);
  fe_collection.push_back (e2);

  dof.distribute_dofs(fe_collection);
  check_renumbering(dof);
  dof.clear();
}


int main ()
{
  std::ofstream logfile ("renumber_block_wise_01/output");
  deallog << std::setprecision (2);
  deallog << std::fixed;
  deallog.attach(logfile);
  deallog.depth_console (0);

  deallog.push ("1d");
  check<1> ();
  deallog.pop ();
  deallog.push ("2d");
  check<2> ();
  deallog.pop ();
  deallog.push ("3d");
  check<3> ();
  deallog.pop ();
}