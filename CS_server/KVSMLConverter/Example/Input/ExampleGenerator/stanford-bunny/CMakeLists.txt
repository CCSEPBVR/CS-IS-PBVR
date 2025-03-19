cmake_minimum_required(VERSION 3.28)
find_package(CGAL REQUIRED)
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(VTK REQUIRED)

add_executable(stanford-bunny stanford-bunny.cc)
target_link_libraries(stanford-bunny PUBLIC ${VTK_LIBRARIES} CGAL::CGAL
                                            Eigen3::Eigen)
vtk_module_autoinit(TARGETS stanford-bunny MODULES ${VTK_LIBRARIES})
