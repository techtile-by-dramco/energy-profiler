# ((3.5*7.37)*1e-6-218100*(7.37*1e-6)**2)*1e12

# In ep-evaluation.csv staat de gemeten data van de evaluatie
# We berekenen de verwachte power van de energy profiler en vergelijken die met de gemeten power
# De verwachte power wordt berekend op basis van de serieweerstand, de geselecteerde spanning en de gemeten stroom van de Otii Arc
# Het gemeten vermogen van de energy profiler is beschikbaar via het ep_live_status.py script

import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("ep-evaluation.csv")
# print(df.head())

df["resistance"]
df["selected_otii_voltage_v"]
df["otii_current_ua"]
df["measured_ep_power_pw"]


expected_ep_power_pw = ((df["selected_otii_voltage_v"]*df["otii_current_ua"])*(1e-6)-df["resistance"]*(df["otii_current_ua"]*1e-6)**2)*1e12

error = round((expected_ep_power_pw-df["measured_ep_power_pw"])/1000)

print(error)

plt.figure(figsize=(8, 6))
plt.plot(error)
plt.xlabel("Measurement Index")
plt.ylabel("Power Error (nW)")
plt.title("Expected vs Measured Power Error")
plt.grid(True)
plt.savefig("ep_evaluation_power_error.png", dpi=300)
plt.show()