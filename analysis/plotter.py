import argparse
from datetime import timedelta
from io import StringIO
import re

import matplotlib.pyplot as plt
import numpy as np
from scipy.ndimage import gaussian_filter1d

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
parser.add_argument(
    "--smooth", metavar="sigma",
    help="Sigma for a Gaussian filter to smooth out the battery lut, defaults to no filter",
    default=0, type=float
)
args = parser.parse_args()

do_smoothing = args.smooth > 0

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


def voltage_to_steps(v: float):
    "represents voltages with two decimal places as int. Example 3.73 V => 373, 4.15 V => 415"
    return np.int_(np.round(v * 100))


########################
# Main part

with open(args.file) as f:
    all_text = f.read()
    # split by any line starting with 'Reading', which should be or title headings, but include the titles in the array
    recordings_split = [s for s in re.split(r'(Reading.*)', all_text) if s.strip() != '']
    # collect pairs of csv titles and the data of the individual recordings
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
        # kick out 0V from battery reading, occurs at the beginning, before voltage value is available
        id_time_force_filtered = id_time_force_filtered[id_time_force_filtered[battery_title] != 0]

        ids = id_time_force_filtered[ids_title]
        times = id_time_force_filtered[times_title]
        battery = id_time_force_filtered[battery_title]
        forces = id_time_force_filtered[forces_title]

        td = timedelta(milliseconds=int(times[-1]))
        print(f"idx: {idx}, samples: {ids[-1]}, recording time (hh:mm:ss): {str(td)}, recording time seconds: {td.seconds}")
        print(f"min force (N): {np.min(forces)}, max force (N): {np.max(forces)}")

        real_percent = 100 - ids / max(ids) * 100

        sums = {}

        # create a dictionary which contains summed up percentages and a counter for each voltage step
        for i, v in enumerate(battery):
            v_int = voltage_to_steps(v)
            if v_int in sums:
                sums[v_int] = (sums[v_int][0] + real_percent[i], sums[v_int][1] + 1)
            else:
                sums[v_int] = (real_percent[i], 1)

        # from the sums and counters generate the average per voltage step
        look_up_dict = {voltage: sum_and_counter[0] / sum_and_counter[1] for voltage, sum_and_counter in sums.items()}

        # parameters for lut generation
        min_vol = min(battery)
        max_vol = max(battery)
        lut_length = voltage_to_steps(max_vol) - voltage_to_steps(min_vol) + 1

        # generate one float voltage for each space in the lut
        lut_v = np.linspace(min_vol, max_vol, num=lut_length)

        # contains potential incomplete pairs of voltage with percentage based on averaged values from above
        dict_as_arr = np.array(sorted(look_up_dict.items()))

        # fill the potential missing voltage with linear interpolation
        lut = np.interp(voltage_to_steps(lut_v), dict_as_arr[:, 0], dict_as_arr[:, 1], 0, 100)
        # smooth the values
        if do_smoothing:
            lut_gau = gaussian_filter1d(lut, args.smooth, mode="nearest")

            # make sure the lut starts with 0% and ends with 100%
            lut_gau[0] = 0
            lut_gau[-1] = 100

        if (min_vol > 3.4 or max_vol < 4.1):
            print("\nWARNING: this recording doesn't seem to contain a complete battery drain. Use another one to replace your LUT")

        print("\nPaste and replace the following lines in include/user_config.h: \n")

        print(f"const uint16_t min_lut_voltage = {voltage_to_steps(min_vol)};")
        print(f"const uint16_t max_lut_voltage = {voltage_to_steps(max_vol)};")
        c_lut = (
            'const uint8_t battery_percent_lut [] = {' +
            ', '.join(map(str, map(int, np.round(lut_gau if do_smoothing else lut)))) +
            '};\n'
        )
        print(c_lut)

        # plot the recorded battery values and the lut
        plt.plot(battery, real_percent, 'r')
        plt.plot(lut_v, np.round(lut))
        if do_smoothing:
            plt.plot(lut_v, np.round(lut_gau))

        plt.legend([
            'Battery readings',
            'Look up table',
            'Look up table Gaussian filter',
        ])
        plt.xlabel('Battery Voltage in V')
        plt.ylabel('Battery percentage')
        plt.show(block=True)
