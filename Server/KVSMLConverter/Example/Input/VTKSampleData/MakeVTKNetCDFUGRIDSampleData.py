"""CDL for the three UGRID sample files."""


def _format_value(value):
    return str(value) if isinstance(value, int) else format(value, ".17g")


def _format_values(values, values_per_line=8):
    lines = []
    for begin in range(0, len(values), values_per_line):
        lines.append(
            "    "
            + ", ".join(
                _format_value(value)
                for value in values[begin : begin + values_per_line]
            )
        )
    return ",\n".join(lines)


def _cdl(filename, time_index):
    heights = (
        (0.0, 0.5, 0.2, -1.0, 0.5, 0.0, 0.4),
        (0.2, 0.3, 0.3, -1.0, 0.2, 0.2, 0.3),
        (0.4, 0.1, 0.6, -1.0, 0.3, 0.5, 0.2),
    )
    areas = (
        (1.0, 0.5),
        (0.5, 1.5),
        (0.75, 1.25),
    )

    return f"""netcdf {filename} {{
dimensions:
    time = 1 ;
    nMesh2_node = 7 ;
    nMesh2_face = 2 ;
    nMaxMesh2_face_nodes = 4 ;
variables:
    double Mesh2_node_x(nMesh2_node) ;
        Mesh2_node_x:standard_name = "projection_x_coordinate" ;
        Mesh2_node_x:units = "m" ;
    double Mesh2_node_y(nMesh2_node) ;
        Mesh2_node_y:standard_name = "projection_y_coordinate" ;
        Mesh2_node_y:units = "m" ;
    double h(time, nMesh2_node) ;
        h:mesh = "Mesh2" ;
        h:location = "node" ;
        h:_FillValue = -1. ;
    float area(time, nMesh2_face) ;
        area:mesh = "Mesh2" ;
        area:location = "face" ;
    double time(time) ;
        time:units = "days since 2000-01-01" ;
    int Mesh2_face_nodes(nMesh2_face, nMaxMesh2_face_nodes) ;
        Mesh2_face_nodes:start_index = 1 ;
        Mesh2_face_nodes:_FillValue = -1 ;
    int Mesh2 ;
        Mesh2:cf_role = "mesh_topology" ;
        Mesh2:topology_dimension = 2 ;
        Mesh2:node_coordinates = "Mesh2_node_x Mesh2_node_y" ;
        Mesh2:face_node_connectivity = "Mesh2_face_nodes" ;

// global attributes:
        :Conventions = "CF-1.8 UGRID-1.0" ;
data:
    Mesh2_node_x = 0, 1, 1, 0, 1, 2, 2 ;
    Mesh2_node_y = 1, 1, 0, 0, 1, 1, 0 ;
    h =
{_format_values(heights[time_index])} ;
    area =
{_format_values(areas[time_index])} ;
    time = {(0, 31, 62)[time_index]} ;
    Mesh2_face_nodes =
    1, 2, 3, 4,
    5, 6, 7, -1 ;
    Mesh2 = 0 ;
}}
"""


def generate(output_dir, writer):
    files = []
    for time_index in range(3):
        filename = f"vtkNetCDFUGRIDReader_{time_index}.nc"
        files.append(
            writer(output_dir, filename, "cdf-2", _cdl(filename, time_index))
        )
    return files
