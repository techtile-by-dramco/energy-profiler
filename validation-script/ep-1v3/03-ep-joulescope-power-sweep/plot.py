import pandas as pd
import matplotlib.pyplot as plt
import os

# Path to save the experiment data as a YAML file
current_file_path = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file_path)
parent_path = os.path.dirname(current_dir)
filename = os.path.basename(current_dir)

path = f"{current_dir}/1769440189_measurements.csv"

SAVE = False

print(path)

# Load CSV
df = pd.read_csv(path)

# df = df[0:460]

print(df["js_power_pw"]-df["pwr_pw"])


plt.figure()
plt.plot(df["js_power_pw"]/1e6,df["pwr_pw"]/1e6)#, label="Joulescope Power (pW)")
# plt.plot(df["pwr_pw"], label="Calculated Power (pW)")
plt.xlabel("JS Power [uW]")
plt.ylabel("EP Power [uW]")
# plt.title("Power Comparison")
plt.legend()
plt.grid(True)
if SAVE:
  plt.savefig(f"{current_dir}/js_vs_ep.png")
plt.show()

# Plot both power columns
plt.figure()
plt.plot(df["js_power_pw"]/1e6, (df["js_power_pw"]-df["pwr_pw"])/1e6)#, label="Joulescope Power (pW)")
plt.xlabel("JS Power [uW]")
plt.ylabel("Delta [uW]")
# plt.title("Power Comparison")
plt.legend()
plt.grid(True)
if SAVE:
  plt.savefig(f"{current_dir}/js_vs_ep_delta.png")
plt.show()

# Plot both power columns
plt.figure()
plt.plot(df["js_power_pw"]/1e6, (df["js_power_pw"]-df["pwr_pw"])/df["js_power_pw"]*100)#, label="Joulescope Power (pW)")
plt.xlabel("JS Power [uW]")
plt.ylabel("Error [%]")
# plt.title("Power Comparison")
plt.legend()
plt.grid(True)
if SAVE:
  plt.savefig(f"{current_dir}/js_vs_ep_error.png")
plt.show()