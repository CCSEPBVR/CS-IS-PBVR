"""CDL for the three MPAS sample files."""


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


def _cells_on_vertex():
    values = []
    nx = 12
    for row_index in range(3):
        for column_index in range(nx - 1):
            p00 = row_index * nx + column_index + 1
            p10 = p00 + 1
            p01 = (row_index + 1) * nx + column_index + 1
            p11 = p01 + 1
            values.extend((p00, p10, p11))
            values.extend((p00, p11, p01))
    return values


def _coordinates():
    x_cell = []
    y_cell = []
    z_cell = []
    for row_index in range(4):
        for column_index in range(12):
            x_cell.append(column_index * 10000)
            y_cell.append(row_index * 10000)
            z_cell.append(0)
    return x_cell, y_cell, z_cell


def _temperature_values(time_index):
    values = []
    for _row_index in range(3):
        for column_index in range(11):
            for triangle_index in range(2):
                numerator = 3 * column_index + (1 if triangle_index == 0 else 0)
                value = 270.0 + 5.0 * time_index + 20.0 * numerator / 31.0
                values.extend((value, value, value, value))
    return values


def _cdl(filename, time_index):
    x_cell, y_cell, z_cell = _coordinates()
    return f"""netcdf {filename} {{
dimensions:
    Time = 1 ;
    nCells = 48 ;
    nVertices = 66 ;
    vertexDegree = 3 ;
    nVertLevels = 4 ;
variables:
    int cellsOnVertex(nVertices, vertexDegree) ;
    double xCell(nCells) ;
    double yCell(nCells) ;
    double zCell(nCells) ;
    float temperature(Time, nVertices, nVertLevels) ;
        temperature:units = "K" ;
        temperature:long_name = "simple cell-centered temperature gradient" ;

// global attributes:
        :on_a_sphere = "NO" ;
        :title = "Simple fixed vtkMPASReader multilayer volume sample" ;
data:
    cellsOnVertex =
{_format_values(_cells_on_vertex(), 3)} ;
    xCell =
{_format_values(x_cell)} ;
    yCell =
{_format_values(y_cell)} ;
    zCell =
{_format_values(z_cell)} ;
    temperature =
{_format_values(_temperature_values(time_index))} ;
}}
"""


def generate(output_dir, writer):
    files = []
    for time_index in range(3):
        filename = f"vtkMPASReader_{time_index}.nc"
        files.append(
            writer(output_dir, filename, "cdf-2", _cdl(filename, time_index))
        )
    return files
