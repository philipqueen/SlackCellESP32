from pathlib import Path
from typing import Optional
import pandas as pd

def csv_to_dataframes(csv_path: Path) -> dict[str, pd.DataFrame]:
    dataframes = {}
    chunk_iterator = pd.read_csv(csv_path, chunksize=1, header=None)
    
    current_data = []
    headers = None
    section_id = 0

    header_start = "Reading ID"
    
    for chunk in chunk_iterator:
        if chunk.iloc[0, 0] == header_start:
            if current_data:
                dataframes[f"Recording_{section_id}"] = pd.DataFrame(current_data, columns=headers)
                section_id += 1
                current_data = []
            headers = chunk.iloc[0].tolist()
        else:
            current_data.append(chunk.iloc[0].tolist())
    
    if current_data:
        dataframes[f"Recording_{section_id}"] = pd.DataFrame(current_data, columns=headers)
        
    return dataframes

if __name__ == "__main__":
    csv_path = Path("/PATH/slackcell.txt")
    dataframes = csv_to_dataframes(csv_path)
    for name, df in dataframes.items():
        df.columns = df.columns.str.strip()
        for key in df.columns:
            print(f"Column: {key}")
        df["Time Difference"] = df["Time (ms)"].diff()
        df["Sample Rate (Hz)"] = 1000 / df["Time Difference"]
        print(f"DataFrame '{name}':")
        print(df.head(10))

        print(f"Average Sample Rate: {df['Sample Rate (Hz)'].mean()}")
        print(f"Median Sample Rate: {df['Sample Rate (Hz)'].median()}")
        print(f"Standard Deviation of Sample Rate: {df['Sample Rate (Hz)'].std()}")
        print(f"Minimum Sample Rate: {df['Sample Rate (Hz)'].min()}")
        print(f"Maximum Sample Rate: {df['Sample Rate (Hz)'].max()}")