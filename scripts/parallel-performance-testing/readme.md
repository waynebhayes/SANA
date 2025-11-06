# Parallel Performance Testing Script

Simple script for running SANA performance tests across different versions and thread counts.

## Commands

- `./ppt.sh compute` - Creates sbatch scripts and submits all jobs to the Openlab SLURM queue
- `./ppt.sh extract` - Parses the output files and extracts statistics into a CSV file at `results/results.csv`.
- `./ppt.sh graph` - Generates performance graphs from the CSV data.
- `./ppt.sh clear` - Clears all output files and sbatch scripts. Useful if you want to re-run tests.

## Tidbits

The script will automatically **skip** any configurations that already have output files. If you want to re-run a test, use `./ppt.sh clear` first to remove existing outputs.

The monitoring table shows real-time status from SLURM, including job IDs, elapsed time, and why jobs might be pending (like "Priority" or "QOSMaxCpuPerNode").

## Limitations

- Testing only happens **once per configuration** for now.
- Cores are **shared within Openlab**.

