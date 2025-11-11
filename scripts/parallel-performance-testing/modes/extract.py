"""Extract mode: Parse results from output files into CSV."""

import sys
import csv
import re


def run_extract(tester) -> None:
    """
    Extract statistics from output files and write to CSV.
    
    Args:
        tester: ParallelPerformanceTester instance
    """
    tester.ensure_directories()
    print("Extracting statistics from output files...")
    
    results_file = tester.results_dir / "results.csv"
    
    # Prepare header
    header = ["sana_version", "threads", "machine", "output_file"] + tester.detection_order
    
    results = []
    count = 0
    
    # Process output files
    for output_file in sorted(tester.output_dir.glob("*.out")):
        basename_file = output_file.name
        
        # Parse filename for version, threads, and machine
        match = re.match(r"([0-9]+\.[0-9]+)-([0-9]+)-([a-z\-]+)-run\d+\.out", basename_file)
        if not match:
            continue
        
        sana_version = match.group(1)
        threads = match.group(2)
        machine = match.group(3)
        
        # Read file content
        content = output_file.read_text()
        
        # Extract values using patterns
        row = {
            "sana_version": sana_version,
            "threads": threads,
            "machine": machine,
            "output_file": basename_file,
        }
        
        for label in tester.detection_order:
            pattern = tester.detections[label]
            match = re.search(pattern, content)
            row[label] = match.group(1) if match else ""
        
        results.append(row)
        count += 1
    
    # Write results
    if count == 0:
        print("No results to write")
        if results_file.exists():
            results_file.unlink()
    else:
        with open(results_file, "w", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=header)
            writer.writeheader()
            writer.writerows(results)
        
        print(f"Extracted statistics from {count} files")
        print(f"Results written to: {results_file}")

