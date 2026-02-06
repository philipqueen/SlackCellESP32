import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import re
from datetime import timedelta
from io import StringIO
import numpy as np
import argparse

description = """
This python file analyzes a csv file to estimate remaining battery run time based on battery voltage..

Requirements: python installation with external libraries Numpy and Scipy installed.

How to use:
1. Make sure HAS_BATTERY_READOUT feature is activated (include/pins.h)
   and standby feature deactivated (set variable automaticStandbyActive in src/power.cpp to false) and flash code onto SlackCell
2. Fully charge SlackCell
3. Unplug from charger
4. Make sure SD card is inserted
5. Restart SlackCell to restart recording (which will include battery voltage over time)
6. Let it run until it turn off due to low battery. (That will take hours, depending on the battery connected)
7. Copy only the section of the latest recording from the sd card into a new csv file on your computer.
8. Run in the terminal: python analysis/plotter.py path_to_your_recording.csv
9. Paste the LUT into your code as instructed in the serial output  of the script
10. Flash code again
"""


# print all floats with 3 decimal places
np.set_printoptions(formatter={'float': "{0:0.3f}".format})

# setup interactive plot
plt.ion()

# make it nice to use from the terminal
parser = argparse.ArgumentParser("plotter.py", formatter_class=argparse.RawDescriptionHelpFormatter, description=description)
parser.add_argument("file", help="Path to csv file containing recording of one full battery drain", type=str)
args = parser.parse_args()

# The titles in the csv files with _ instead of whitespace. Change here if renamed in the generation code
ids_title = 'Reading_ID'
times_title = 'Time_(ms)'
battery_title = 'Battery_Voltage_(V)'
forces_title = 'Force_(N)'

datatypes_per_title = {
    ids_title: np.int_,
    times_title: np.int_,
    battery_title: np.float64,
    forces_title: np.int_
}

######################
# Helper functions


def index_of_unique_values(a):
    """
    if there are subsequent equal values in an array, returns the last index of the groups
    [0, 1, 1, 5, 5, 5, 2, 6, 7, 2, 2, 9] => [0, 2, 5, 6, 7, 8, 10, 11]
    """
    # works by getting the indices where two shifted versions of "a" are different and adding the last index manually
    return np.concatenate([np.where(a[:-1] != a[1:])[0], [len(a) - 1]])


# represents voltages with two decimal places as int. Example 3.73 V => 373, 4.15 V => 415
def voltage_to_steps(v: float):
    return int(round(v * 100))


########################
# Main part

with open(args.file) as f:
    all_text = f.read()
    # split by any line starting with 'Reading', which should be or title headings, but include the titles in the array
    recordings_split = [s for s in re.split(r'(Reading.*)', all_text) if s.strip() != '']
    # collect pairs of csv titles and the data of the induvidual recordings
    recordings = zip(recordings_split[::2], recordings_split[1::2])

    for idx, (rec_header, rec) in enumerate(recordings):
        print(rec_header)
        # Get the titles as array
        # also trims whitespace and replaces whitespaces in titles with _
        titles = [s.replace(' ', '_') for s in re.split(r'\s*,\s*', rec_header)]

        needed_titles = [ids_title, times_title, battery_title, forces_title]
        if any((delim not in titles for delim in needed_titles)):
            print(f'Skipping recording. One or more fields are missing from the csv Header, expecting {needed_titles}')
            continue

        id_time_force_arr = np.genfromtxt(
            StringIO(rec), delimiter=",", dtype=[datatypes_per_title.get(n) for n in titles], names=titles, deletechars=''
        )
        # kick out weird outliers in the force
        id_time_force_filtered = id_time_force_arr[id_time_force_arr[forces_title] != -104]

        ids = id_time_force_filtered[ids_title]
        times = id_time_force_filtered[times_title]
        battery = id_time_force_filtered[battery_title]
        forces = id_time_force_filtered[forces_title]

        td = timedelta(milliseconds=int(times[-1]))
        print(f"idx: {idx}, samples: {ids[-1]}, recording time (hh:mm:ss): {str(td)}, recording time seconds: {td.seconds}")
        print(f"min force (N): {np.min(forces)}, max force (N): {np.max(forces)}")
        unique_battery_indices = index_of_unique_values(battery)[1:]

        real_percent = 100 - ids[unique_battery_indices] / max(ids) * 100

        # the code to fit the curve comes from here: https://stackoverflow.com/a/75598551

        x = battery[unique_battery_indices]
        y = real_percent

        sums = {}

        # create a dictionary which contains summed up percentages and a counter for each voltage step
        for i, v in enumerate(x):
            v_int = voltage_to_steps(v)
            if v_int in sums:
                sums[v_int] = (sums[v_int][0] + real_percent[i], sums[v_int][1] + 1)
            else:
                sums[v_int] = (real_percent[i], 1)

        # from the sums and counters generate the average per voltage step
        look_up_dict = {voltage: sum_and_counter[0] / sum_and_counter[1] for voltage, sum_and_counter in sums.items()}

        # returns percentage for given voltage based on averaged values from above

        def look_up(v):
            v_int = voltage_to_steps(v)
            if v_int in look_up_dict:
                return look_up_dict[voltage_to_steps(v)]

        # a curve fitting well for the lipo discharge behavior, comes from here: https://electronics.stackexchange.com/a/551667
        def lipo_curve(x, a, b, c, d):
            return a - (a / (((1 + (x / b)**c)**d)))

        # fit lipo_curve to x, y
        # the original values, also come from the same post: https://electronics.stackexchange.com/a/551667
        coefs_lipo_curve, _ = curve_fit(lipo_curve, x, y, [123, 3.7, 80, 0.165])

        def lipo_curve_formula_str(a, b, c, d):
            return f"{a:.3f}f - ({a:.3f}f/((pow(1 + pow(x/{b:.3f}f,{c:.3f}f),{d:.3f}f))))"

        # parameters for lut generation
        min_vol = min(battery[unique_battery_indices])
        max_vol = max(battery[unique_battery_indices])
        lut_length = voltage_to_steps(max_vol) - voltage_to_steps(min_vol) + 1

        # generate one float voltage for each space in the lut
        lut_v = np.linspace(min_vol, max_vol, num=lut_length)

        lut = np.minimum(np.rint(lipo_curve(lut_v, *coefs_lipo_curve)), 100)

        print(coefs_lipo_curve)
        print(lipo_curve_formula_str(*coefs_lipo_curve))

        def mse(func, x, y, coefs):
            return np.mean((func(x, *coefs) - y)**2)

        print(f"Mean square error: {mse(lipo_curve, x, y, coefs_lipo_curve)}")

        # initialize some points
        x_data = np.linspace(min(x), max(x), 50)
        # transform x_data to y-axis values via lipo_curve
        y_data = lipo_curve(x_data, *coefs_lipo_curve)
        # plot the points

        y_data_dict = [look_up(v) for v in x_data]

        y_data_lut = [lut[voltage_to_steps(v) - voltage_to_steps(min_vol)] for v in x_data]

        if (min_vol > 3.45 or max_vol < 4.1):
            print("\nWARNING: this recording doesn't seem to contain a complete battery drain. Use another on to replace your LUT")

        print("\nPaste and replace the following lines in include/user_config.h: \n")

        print(f"const uint16_t min_lut_voltage = {voltage_to_steps(min_vol)};")
        print(f"const uint16_t max_lut_voltage = {voltage_to_steps(max_vol)};")
        c_lut = (
            'const uint8_t battery_percent_lut [] = {' +
            ', '.join(map(str, map(int, lut))) +
            '};\n'
        )

        print(c_lut)

        plt.plot(x, y, 'r')
        plt.plot(x_data, y_data)
        plt.plot(x_data, y_data_dict, 'g')
        plt.plot(x_data, y_data_lut)
        plt.legend(['Battery readings', 'Fitted lipo curve', 'Averaged battery readings', 'Look up table'])
        plt.xlabel('Battery Voltage in V')
        plt.ylabel('Battery percentage')
        plt.show(block=True)
