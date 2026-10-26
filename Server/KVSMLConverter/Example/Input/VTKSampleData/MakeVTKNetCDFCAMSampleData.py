"""CDL for the CAM points and connectivity sample files."""


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


def _point_cdl(filename, time_index):
    temperatures = []
    for lev_index in range(3):
        for ncol_index in range(6):
            temperatures.append(
                280.0 + time_index - 4.0 * lev_index + ncol_index
            )

    return f"""netcdf {filename} {{
dimensions:
    time = 1 ;
    lev = 3 ;
    ncol = 6 ;
variables:
    double lon(ncol) ;
    double lat(ncol) ;
    float T(time, lev, ncol) ;
    double time(time) ;
    float lev(lev) ;
data:
    lon = 0, 1, 2, 0, 1, 2 ;
    lat = 0, 0, 0, 1, 1, 1 ;
    T =
{_format_values(temperatures)} ;
    time = {time_index} ;
    lev = 1000, 700, 400 ;
}}
"""


def _connectivity_cdl(filename):
    return f"""netcdf {filename} {{
dimensions:
    four = 4 ;
    ncells = 2 ;
variables:
    int element_corners(four, ncells) ;
data:
    element_corners =
    1, 2,
    2, 3,
    5, 6,
    4, 5 ;
}}
"""


def generate(output_dir, writer):
    files = []

    filename = "vtkNetCDFCAMReader_connectivity.nc"
    files.append(
        writer(output_dir, filename, "cdf-2", _connectivity_cdl(filename))
    )

    for time_index in range(3):
        filename = f"vtkNetCDFCAMReader_point_{time_index}.nc"
        files.append(
            writer(output_dir, filename, "cdf-2", _point_cdl(filename, time_index))
        )
    return files
