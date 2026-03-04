import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

# Path to save the experiment data as a YAML file
current_file_path = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file_path)
parent_path = os.path.dirname(current_dir)
filename = os.path.basename(current_dir)

# path = f"{current_dir}/1772541144_measurements_calibrated.csv"
path = f"{current_dir}/1772555271_measurements_calibrated.csv"

# CSV inlezen
df_raw = pd.read_csv(path)

# Plot: index op x-as, measured op y-as
plt.figure(figsize=(8, 6))
plt.plot(df_raw["index"], df_raw["epc"])
plt.plot(df_raw["index"], df_raw["measured"])
plt.xlabel("Digital potentiometer Index")
plt.ylabel("Measured")
plt.grid(True)
plt.savefig(f"{current_dir}/{filename}.png", dpi=300)
plt.show()

error = df_raw["epc"] - df_raw["measured"]

plt.figure(figsize=(8, 6))
plt.plot(df_raw["index"], error)
plt.xlabel("Digital potentiometer Index")
plt.ylabel("Resistance Error")
plt.grid(True)
plt.savefig(f"{current_dir}/{filename}_error.png", dpi=300)
plt.show()

plt.figure(figsize=(8, 6))
plt.plot(df_raw["index"], (error/df_raw["epc"])*100)
plt.xlabel("Digital potentiometer Index")
plt.ylabel("Resistance Error [%]")
plt.grid(True)
plt.savefig(f"{current_dir}/{filename}_error_percentage.png", dpi=300)
plt.show()

data_len = len(df_raw["measured"])

print(f"constexpr float lookup_table[{data_len}] = {{ ", end="")
print(", ".join(f"{v:.2f}f" for v in df_raw["measured"]), end="")
print(" };")
