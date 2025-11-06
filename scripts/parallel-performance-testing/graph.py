#!/usr/bin/env python3
import csv
import matplotlib.pyplot as plt
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
CSV_FILE = os.path.join(SCRIPT_DIR, "results", "results.csv")

if not os.path.exists(CSV_FILE):
    print(f"Error: {CSV_FILE} not found. Run './ppt.sh extract' first.")
    exit(1)

data = []
with open(CSV_FILE, 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        data.append(row)

if not data:
    print("No data found in CSV")
    exit(1)

by_version = {}

for row in data:
    if not row['threads']:
        continue
    version = row['sana_version']
    if version not in by_version:
        by_version[version] = []
    
    cores_val = int(row['threads'])
    tinitial_val = float(row['TInitial']) if row['TInitial'] else 0
    if not row['ExecutionTime']:
        continue
    execution_val = float(row['ExecutionTime'])
    total_val = tinitial_val + execution_val
    final_score_val = float(row['FinalScore']) if row['FinalScore'] else 0
    by_version[version].append((cores_val, tinitial_val, execution_val, total_val, final_score_val))

for version in by_version:
    by_version[version].sort(key=lambda x: x[0])

all_cores = set()
for version in by_version:
    for point in by_version[version]:
        all_cores.add(point[0])
all_cores = sorted(all_cores)

fig = plt.figure(figsize=(20, 5))

ax1 = plt.subplot(141)
ax1.set_xscale('log')
ax1.set_xticks(all_cores)
ax1.set_xticklabels(all_cores)
for version in sorted(by_version.keys()):
    points = by_version[version]
    cores = [p[0] for p in points]
    execution = [p[2] for p in points]
    ax1.plot(cores, execution, 'o-', label=f'{version} ExecutionTime')
ax1.set_xlabel('Cores')
ax1.set_ylabel('Time (s)')
ax1.set_title('A: Time vs Cores (Log X)')
ax1.legend()
ax1.grid(True)

ax2 = plt.subplot(142)
ax2.set_xscale('log')
ax2.set_yscale('log')
ax2.set_xticks(all_cores)
ax2.set_xticklabels(all_cores)
for version in sorted(by_version.keys()):
    points = by_version[version]
    cores = [p[0] for p in points]
    execution = [p[2] for p in points]
    ax2.plot(cores, execution, 'o-', label=f'{version} ExecutionTime')
ax2.set_xlabel('Cores')
ax2.set_ylabel('Time (s)')
ax2.set_title('B: Time vs Cores (Log-Log)')
ax2.legend()
ax2.grid(True)

ax3 = plt.subplot(143)
ax3.set_xticks(all_cores)
ax3.set_xticklabels(all_cores)
for version in sorted(by_version.keys()):
    points = by_version[version]
    cores = [p[0] for p in points]
    execution = [p[2] for p in points]
    ax3.plot(cores, execution, 'o-', label=f'{version} ExecutionTime')
ax3.set_xlabel('Cores')
ax3.set_ylabel('Time (s)')
ax3.set_title('C: Time vs Cores (Linear)')
ax3.legend()
ax3.grid(True)

ax4 = plt.subplot(144)
ax4.set_xticks(all_cores)
ax4.set_xticklabels(all_cores)
for version in sorted(by_version.keys()):
    points = by_version[version]
    cores = [p[0] for p in points]
    final_score = [p[4] for p in points]
    ax4.plot(cores, final_score, 'o-', label=f'{version} EC Performance')
ax4.set_xlabel('Cores')
ax4.set_ylabel('Final Score')
ax4.set_title('D: EC Performance vs Cores')
ax4.legend()
ax4.grid(True)

plt.tight_layout()
output_file = os.path.join(SCRIPT_DIR, "results", "performance_graphs.png")
plt.savefig(output_file)
print(f"Graphs saved to {output_file}")
plt.show()

