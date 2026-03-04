import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

filename = "1769004253_ADS-connected_results-uncalibrated"
# filename = "1769004758_ADS-connected_results-calibrated"

# CSV inlezen
df_raw = pd.read_csv(f"{filename}.csv")

df_raw = df_raw[0:45]

# Plot: index op x-as, measured op y-as
plt.figure(figsize=(8, 6))
plt.plot(df_raw["index"], df_raw["epc"])
plt.plot(df_raw["index"], df_raw["measured"])
plt.xlabel("Digital potentiometer Index")
plt.ylabel("Measured")
plt.grid(True)
plt.savefig(f"{filename}.png", dpi=300)
plt.show()

error = df_raw["epc"] - df_raw["measured"]

plt.figure(figsize=(8, 6))
plt.plot(df_raw["index"], error)
plt.xlabel("Digital potentiometer Index")
plt.ylabel("Resistance Error")
plt.grid(True)
plt.savefig(f"{filename}_error.png", dpi=300)
plt.show()

plt.figure(figsize=(8, 6))
plt.plot(df_raw["index"], (error/df_raw["epc"])*100)
plt.xlabel("Digital potentiometer Index")
plt.ylabel("Resistance Error [%]")
plt.grid(True)
plt.savefig(f"{filename}_error_percentage.png", dpi=300)
plt.show()


exit()

df1 = df_raw[(df_raw["index"] >= 0) & (df_raw["index"] <= 231)]
df2 = df_raw[(df_raw["index"] >= 232) & (df_raw["index"] <= 486)]

df_range = [df1, df2]

for df in df_range:

    # if len(df["index"]) == len(df_raw["index"]):
    #     exit()

    error = df["epc"] - df["measured"]


    # a, b = np.polyfit(df["index"], error, 1)
    # print(f"y = {a:.6e} * x + {b:.6e}")

    a, b, c = np.polyfit(df["index"], error, 2)
    print(f"y = {a:.6e} * x^2 + {b:.6e} * x + {c:.6e}")

    # Plot: index op x-as, measured op y-as
    plt.figure()
    plt.plot(df["index"], error)
    # plt.plot(df["index"], a * df["index"] + b, label=f"Fit: y = {a:.2e} * x + {b:.2e}")
    plt.plot(
        df["index"],
        a * df["index"]**2 + b * df["index"] + c,
        label=f"Fit: y = {a:.2e}·x² + {b:.2e}·x + {c:.2e}"
    )
    plt.xlabel("Digital potentiometer Index")
    plt.ylabel("Resistance Error")
    plt.title("Measured vs Index")
    plt.grid(True)
    plt.show()


    # Correctie toepassen
    ep_res_calc = df["epc"] - (a * df["index"] + b)

    # Plot: index op x-as, measured op y-as
    plt.figure()
    plt.plot(df["index"], ep_res_calc)
    plt.plot(df["index"], df["measured"])
    plt.xlabel("Digital potentiometer Index")
    plt.ylabel("Measured")
    plt.title("Measured vs Index")
    plt.grid(True)
    plt.show()


    error = ep_res_calc - df["measured"]

    # Plot: index op x-as, measured op y-as
    plt.figure()
    plt.plot(df["index"], error)
    plt.xlabel("Digital potentiometer Index")
    plt.ylabel("Resistance Error")
    plt.title("Measured vs Index")
    plt.grid(True)
    plt.show()

