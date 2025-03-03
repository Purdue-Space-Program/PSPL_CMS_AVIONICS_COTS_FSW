from datetime import datetime
from matplotlib import pyplot as plt
import polars as pl

test_data = pl.read_csv("sitl_data/sensornet_data_delta_cf_2_out_cut.csv")
t0_str = test_data["time"][0]
t0 = datetime.strptime(t0_str, "%Y-%m-%d %H:%M:%S.%f%z")

# Function to correctly parse timestamps with full precision
def parse_timestamp(ts_str):
    dt = datetime.strptime(ts_str, "%Y-%m-%d %H:%M:%S.%f%z")
    return (dt - t0).total_seconds()

# Replace the time column directly, maintaining full precision
test_data = test_data.with_columns(
    pl.col("time").map_elements(parse_timestamp, return_dtype=pl.Float64())
)

print(test_data)
"""
┌────────────┬───────────┬────────────┬────────────────┬──────────────────┐
│ time       ┆ Fuel_psi  ┆ Oxygen_psi ┆ Bang_Bang_Fuel ┆ Bang_Bang_Oxygen │
│ ---        ┆ ---       ┆ ---        ┆ ---            ┆ ---              │
│ f64        ┆ f64       ┆ f64        ┆ bool           ┆ bool             │
╞════════════╪═══════════╪════════════╪════════════════╪══════════════════╡
│ 0.0        ┆ 24.774019 ┆ 22.397823  ┆ true           ┆ true             │
│ 0.004      ┆ 24.82654  ┆ 22.342541  ┆ true           ┆ true             │
│ 0.008      ┆ 24.767691 ┆ 22.346353  ┆ true           ┆ true             │
│ 0.012      ┆ 24.829704 ┆ 22.309975  ┆ true           ┆ true             │
│ 0.016001   ┆ 24.850744 ┆ 22.32459   ┆ true           ┆ true             │
"""

def analyze_state_changes(df):
    times = df["ts"].to_numpy()
    values = df["value"].to_numpy()

    last_state = values[0]
    last_change_time = 0

    min_change_time = float('inf')

    for ts, val in zip(times, values):
        if val != last_state:
            change_time = (ts - last_change_time) * 1000
            # print(change_time)

            last_change_time = ts
            last_state = val

            min_change_time = min(min_change_time, change_time)

    print(min_change_time)

test_out = pl.read_csv("sitl_out.csv")
fu_state = test_out.filter(pl.col("system") == "FU").drop("system")
ox_state = test_out.filter(pl.col("system") == "OX").drop("system")

print("analyze ox")
analyze_state_changes(ox_state)
print("analyze fu")
analyze_state_changes(fu_state)

exit()

# Create figure
fig, ax1 = plt.subplots(figsize=(12, 6))

# Plot Fuel pressure on the first y-axis
color1 = 'tab:blue'
ax1.set_xlabel('Time (seconds)')
# ax1.set_ylabel('Fuel Pressure (psi)', color=color1)
# ax1.plot(test_data["time"].to_numpy(), test_data["Fuel_psi"].to_numpy(), color=color1)
# ax1.tick_params(axis='y', labelcolor=color1)

# Create second y-axis for Bang_Bang_Fuel
ax2 = ax1.twinx()
color2 = 'tab:green'
ax2.set_ylabel('Bang Bang Fuel', color=color2)
ax2.plot(test_data["time"].to_numpy(), 
         test_data["Bang_Bang_Fuel"].map_elements(lambda x: 1 if x else 0, return_dtype=pl.Int64()).to_numpy(), 
         color=color2)
ax2.tick_params(axis='y', labelcolor=color2)
ax2.set_ylim(-0.1, 1.1)  # Set limits for binary values

# Create third y-axis for FU state
# Offset the third axis to the right
ax2.spines['right'].set_position(('outward', 60))
color3 = 'tab:red'
ax2.set_ylabel('FU State', color=color3)
ax2.plot(fu_state["ts"].to_numpy(), fu_state["value"].to_numpy(), color=color3)
ax2.tick_params(axis='y', labelcolor=color3)

# Add legend
lines1, labels1 = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
# lines3, labels3 = ax2.get_legend_handles_labels()
lines = [plt.Line2D([0], [0], color=color1), 
         plt.Line2D([0], [0], color=color2), 
         plt.Line2D([0], [0], color=color3)]
labels = ['Fuel Pressure', 'Bang Bang Fuel', 'FU State']
ax1.legend(lines, labels, loc='upper right')

fig.tight_layout()
plt.grid(True)
plt.show()