#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Polyhedral_mesh_domain_3.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/boost/graph/helpers.h>
#include <CGAL/make_mesh_3.h>
#include <CGAL/refine_mesh_3.h>

#include <CGAL/IO/output_to_vtu.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/border.h>
#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>

#include <CGAL/Aff_transformation_3.h>
#include <CGAL/Polygon_mesh_processing/remesh.h>
#include <CGAL/Polygon_mesh_processing/transform.h>

// Domain
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Polyhedron_3<K> Polyhedron;
typedef CGAL::Polyhedral_mesh_domain_3<Polyhedron, K> Mesh_domain;
#ifdef CGAL_CONCURRENT_MESH_3
typedef CGAL::Parallel_tag Concurrency_tag;
#else
typedef CGAL::Sequential_tag Concurrency_tag;
#endif
// Triangulation
typedef CGAL::Mesh_triangulation_3<Mesh_domain, CGAL::Default,
                                   Concurrency_tag>::type Tr;
typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;
// Criteria
typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;

typedef CGAL::Surface_mesh<K::Point_3> Mesh;
typedef boost::graph_traits<Mesh>::vertex_descriptor vertex_descriptor;
typedef boost::graph_traits<Mesh>::halfedge_descriptor halfedge_descriptor;
typedef boost::graph_traits<Mesh>::face_descriptor face_descriptor;

// To avoid verbose function and named parameters call
using namespace CGAL::parameters;
int main(int argc, char *argv[]) {
  const char *const filename =
      "/home/m-suzuki/Downloads/bunny/reconstruction/bun_zipper.ply";
  CGAL::Surface_mesh<K::Point_3> mesh;
  if (!CGAL::Polygon_mesh_processing::IO::read_polygon_mesh(filename, mesh)) {
    std::cerr << "Invalid input." << std::endl;
    return 1;
  }

  // fill hole
  std::vector<halfedge_descriptor> border_cycles;
  // collect one halfedge per boundary cycle
  CGAL::Polygon_mesh_processing::extract_boundary_cycles(
      mesh, std::back_inserter(border_cycles));
  for (halfedge_descriptor h : border_cycles) {
    std::vector<face_descriptor> patch_facets;
    std::vector<vertex_descriptor> patch_vertices;
    bool success = std::get<0>(
        CGAL::Polygon_mesh_processing::triangulate_refine_and_fair_hole(
            mesh, h,
            CGAL::parameters::face_output_iterator(
                std::back_inserter(patch_facets))
                .vertex_output_iterator(std::back_inserter(patch_vertices))));
    std::cout << "* Number of facets in constructed patch: "
              << patch_facets.size() << std::endl;
    std::cout << "  Number of vertices in constructed patch: "
              << patch_vertices.size() << std::endl;
    std::cout << "  Is fairing successful: " << success << std::endl;
  }

  Polyhedron polyhedron;
  CGAL::copy_face_graph(mesh, polyhedron);

  CGAL::Aff_transformation_3<K> scale(CGAL::SCALING, 2000, 1);
  CGAL::Polygon_mesh_processing::transform(scale, polyhedron);
  // double target_edge_length = 0.8;
  // double facet_size=1.5;
  //
  // double facet_size=0.4;
  double target_edge_length = 0.4;
  CGAL::Polygon_mesh_processing::isotropic_remeshing(
      faces(polyhedron), target_edge_length, polyhedron,
      CGAL::parameters::number_of_iterations(16));

  std::ofstream vtp_file("isomeshed.ply");
  CGAL::IO::write_PLY(vtp_file, polyhedron);

  std::cout << "surface mesh is generated" << std::endl << std::flush;

  // Create domain
  Mesh_domain domain(polyhedron);

  // Mesh criteria (no cell_size set)
  Mesh_criteria criteria(facet_angle = 25,
                         // facet_size = 0.75,
                         // facet_distance = 0.4,
                         facet_size = target_edge_length / 2,
                         facet_distance = target_edge_length / 8,
                         edge_size = target_edge_length / 8,
                         cell_radius_edge_ratio = 3);

  // Mesh generation
  C3t3 c3t3 =
      CGAL::make_mesh_3<C3t3>(domain, criteria, no_perturb(), no_exude());
  // Output
  std::ofstream vtu_file("bunny.vtu");
  CGAL::IO::output_to_vtu(vtu_file, c3t3);

  return EXIT_SUCCESS;
}
