# Parallel Performance Testing Script

Simple script for running SANA performance tests on OpenLab across:
- Different versions
- Different core counts
- Different OpenLab nodes

## Commands

- `./ppt.py compute [N]` - Submit N jobs per configuration and wait for completion. Defaults to 1 job.
- `./ppt.py extract` - Parse output files and extract statistics into CSV at `results/results.csv`
- `./ppt.py graph` - Generate performance graphs from the CSV data
- `./ppt.py clear` - Clear all output files and sbatch scripts
- `-v` - Print out more details

## Notes

- The `compute` command automatically clears previous outputs before starting
- Cores are shared within Openlab; actual concurrency may be limited
- Output files are named: `{version}-{threads}-{machine}-run{N}.out`
