from asyncio.windows_events import NULL

import pandas as pd
import matplotlib.pyplot as plt
import os
import matplot2tikz as tkz

# Path to save the experiment data as a YAML file
current_file_path = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file_path)
parent_path = os.path.dirname(current_dir)
filename = os.path.basename(current_dir)

path = f"{current_dir}/1772621117_measurements.csv"

SAVE = False
PLOT = False
TIKZ = True

print(path)

# Load CSV
df = pd.read_csv(path)

df = df[0:680]

print(df["js_power_pw"]-df["pwr_pw"])

# -- P1 --> power EP calculated via energy profiler
js_voltage = "P1"
# -- P2 --> due to energy loss over the joulescope shunt resistor EP power is calculated via JouleScope voltage
js_voltage = "P2"
df["pwr_pw"] = (df["js_voltage_mv"]*1e3) ** 2 / df["resistance"]


plt.figure()
plt.plot(df["js_power_pw"]/1e6,df["pwr_pw"]/1e6)#, label="Joulescope Power (pW)")
# plt.plot(df["pwr_pw"], label="Calculated Power (pW)")
plt.xlabel("JS Power [uW]")
plt.ylabel("EP Power [uW]")
# plt.title("Power Comparison")
plt.legend()
plt.grid(True)
if SAVE:
  plt.savefig(f"{current_dir}/js_vs_ep_{js_voltage}.png")
if PLOT:
  plt.show()
if TIKZ:
  tkz.save(f"{current_dir}/js_vs_ep_{js_voltage}.tex")

# Plot both power columns
plt.figure()
plt.plot(df["js_power_pw"]/1e6, (df["js_power_pw"]-df["pwr_pw"])/1e6)#, label="Joulescope Power (pW)")
plt.xlabel("JS Power [uW]")
plt.ylabel("Delta [uW]")
# plt.title("Power Comparison")
plt.legend()
plt.grid(True)
if SAVE:
  plt.savefig(f"{current_dir}/js_vs_ep_delta_{js_voltage}.png")
if PLOT:
  plt.show()
if TIKZ:
  tkz.save(f"{current_dir}/js_vs_ep_delta_{js_voltage}.tex")

# Plot both power columns
plt.figure()
plt.plot(df["js_power_pw"]/1e6, (df["js_power_pw"]-df["pwr_pw"])/df["js_power_pw"]*100)#, label="Joulescope Power (pW)")
plt.xlabel("JS Power [uW]")
plt.ylabel("Error [%]")
# plt.title("Power Comparison")
plt.legend()
plt.grid(True)
if SAVE:
  plt.savefig(f"{current_dir}/js_vs_ep_error_{js_voltage}.png")
if PLOT:
  plt.show()
if TIKZ:
  tkz.save(f"{current_dir}/js_vs_ep_error_{js_voltage}.tex")

# Plot both power columns
plt.figure()
plt.plot(df["js_power_pw"]/1e6, (df["js_power_pw"]-df["pwr_pw"])/df["js_power_pw"]*100)#, label="Joulescope Power (pW)")
plt.xlabel("JS Power [uW]")
plt.ylabel("Error [%]")
# plt.title("Power Comparison")
plt.xscale("log")
plt.legend()
plt.grid(True)
if SAVE:
  plt.savefig(f"{current_dir}/js_vs_ep_error_log_{js_voltage}.png")
if PLOT:
  plt.show()
if TIKZ:
  tkz.save(f"{current_dir}/js_vs_ep_error_log_{js_voltage}.tex")

# Plot both power columns
plt.figure()
plt.plot(df["pot_val"], (df["js_power_pw"]-df["pwr_pw"])/1e6)#, label="Joulescope Power (pW)")
plt.xlabel("pot_val")
plt.ylabel("Delta [uW]")
# plt.title("Power Comparison")
plt.legend()
plt.grid(True)
if SAVE:
  plt.savefig(f"{current_dir}/js_vs_pot_val_{js_voltage}.png")
if PLOT:
  plt.show()
if TIKZ:
  tkz.save(f"{current_dir}/js_vs_pot_val_{js_voltage}.tex")