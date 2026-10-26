"""CDL for the three CF rectilinear sample files."""

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


def _temperature_values(time_index):
    values = []
    for z_offset in (0.0, 2.0, 5.0, 10.0):
        for y_index in range(5):
            for x_index in range(6):
                values.append(
                    271.0
                    + 3.0 * time_index
                    + z_offset
                    + 0.25 * y_index
                    + 0.5 * x_index
                )
    return values


def _cdl(filename, time_index):
    temperature = _format_values(_temperature_values(time_index))
    return f"""netcdf {filename} {{
dimensions:
    time = 1 ;
    z = 4 ;
    y = 5 ;
    x = 6 ;
variables:
    double time(time) ;
        time:standard_name = "time" ;
        time:long_name = "time" ;
        time:units = "hours since 2000-01-01 00:00:00" ;
        time:calendar = "gregorian" ;
        time:axis = "T" ;
    double x(x) ;
        x:long_name = "Cartesian x coordinate" ;
        x:units = "km" ;
    double y(y) ;
        y:long_name = "Cartesian y coordinate" ;
        y:units = "km" ;
    double z(z) ;
        z:standard_name = "height" ;
        z:long_name = "height above reference level" ;
        z:units = "m" ;
        z:positive = "up" ;
        z:axis = "Z" ;
    float temperature(time, z, y, x) ;
        temperature:standard_name = "air_temperature" ;
        temperature:long_name = "air temperature" ;
        temperature:units = "K" ;
        temperature:coordinates = "time z y x" ;

// global attributes:
        :Conventions = "CF-1.8" ;
        :title = "Rectilinear time-series volume for vtkNetCDFCFReader testing" ;
        :institution = "CCSEPBVR test data" ;
        :source = "Synthetic structured-grid volume" ;
        :history = "Created 2026-08-28 for vtkNetCDFCFReader testing" ;
data:
    time = {time_index} ;
    x = 0, 10, 20, 30, 40, 50 ;
    y = 0, 10, 20, 30, 40 ;
    z = 0, 100, 250, 500 ;
    temperature =
{temperature} ;
}}
"""


def generate(output_dir, writer):
    files = []
    for time_index in range(3):
        filename = f"vtkNetCDFCFReader_{time_index}.nc"
        files.append(
            writer(output_dir, filename, "cdf-2", _cdl(filename, time_index))
        )
    return files
