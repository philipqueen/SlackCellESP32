import struct
import csv
import argparse

LOG_ENTRY_FORMAT = "<iIf"  # int32, uint32, float (12 bytes)
ENTRY_SIZE = struct.calcsize(LOG_ENTRY_FORMAT)

def find_binary_start(file):
    """Finds the offset where binary data starts after a string header."""
    while True:
        pos = file.tell()
        chunk = file.read(ENTRY_SIZE)
        if len(chunk) < ENTRY_SIZE:
            return pos  # End of file or corrupt
        try:
            struct.unpack(LOG_ENTRY_FORMAT, chunk)
            return pos  # Found valid binary start
        except struct.error:
            file.seek(pos + 1)

def parse_binary_file(binary_path, csv_path):
    with open(binary_path, "rb") as bin_file:
        start = find_binary_start(bin_file)
        bin_file.seek(start)

        with open(csv_path, "w", newline="") as csv_file:
            writer = csv.writer(csv_file)
            writer.writerow(["Reading ID", "Time (µs)", "Force (N)"])

            while True:
                chunk = bin_file.read(ENTRY_SIZE)
                if len(chunk) < ENTRY_SIZE:
                    break
                reading_id, timestamp, force = struct.unpack(LOG_ENTRY_FORMAT, chunk)
                writer.writerow([reading_id, timestamp, round(force, 3)])

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("binary_file", help="Path to the binary log file")
    parser.add_argument("output_csv", help="Path to the output CSV")
    args = parser.parse_args()

    parse_binary_file(args.binary_file, args.output_csv)