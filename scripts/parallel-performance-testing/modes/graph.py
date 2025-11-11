"""Graph mode: Visualize results with performance graphs."""

import sys
import csv
from collections import defaultdict
import numpy as np

try:
    import matplotlib.pyplot as plt
    from matplotlib.ticker import FixedLocator
except ImportError:
    plt = None
    FixedLocator = None


def run_graph(tester) -> None:
    """
    Generate performance graphs from results CSV.
    
    For each characteristic (UserTime, SystemTime, FinalScore, etc.):
    - Creates 3 graphs: log-x, log-log, and linear
    - Each graph shows one line per (sana_version, machine) combination
    - Lines show mean values with ±1 std dev shaded regions
    
    Args:
        tester: ParallelPerformanceTester instance
    """
    if plt is None:
        print("Error: matplotlib is not installed. Install with: pip install matplotlib", file=sys.stderr)
        sys.exit(1)
    
    tester.ensure_directories()
    
    # Ensure graphs subfolder exists
    graphs_dir = tester.results_dir / "graphs"
    graphs_dir.mkdir(parents=True, exist_ok=True)
    
    results_file = tester.results_dir / "results.csv"
    
    if not results_file.exists():
        print(f"Error: {results_file} not found. Run './ppt.py extract' first.", file=sys.stderr)
        sys.exit(1)
    
    # Read CSV
    data = []
    with open(results_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            data.append(row)
    
    if not data:
        print("No data found in CSV", file=sys.stderr)
        sys.exit(1)
    
    # Identify characteristics (all columns except: sana_version, threads, machine, output_file)
    non_characteristic_cols = {'sana_version', 'threads', 'machine', 'output_file'}
    characteristics = [col for col in data[0].keys() if col not in non_characteristic_cols and col.strip()]
    
    if not characteristics:
        print("No characteristics found in CSV", file=sys.stderr)
        sys.exit(1)
    
    # Process data: group by (version, machine, threads) and collect characteristic values
    # Structure: {characteristic: {(version, machine, threads): [values]}}
    processed_data = defaultdict(lambda: defaultdict(list))
    all_threads = set()
    all_configs = set()  # (version, machine) pairs
    
    for row in data:
        try:
            version = row['sana_version'].strip()
            machine = row['machine'].strip()
            threads = int(row['threads'])
            all_threads.add(threads)
            all_configs.add((version, machine))
            
            config_key = (version, machine)
            threads_key = (version, machine, threads)
            
            for char in characteristics:
                value_str = row.get(char, '').strip()
                if value_str:
                    try:
                        value = float(value_str)
                        processed_data[char][threads_key].append(value)
                    except ValueError:
                        pass
        except (KeyError, ValueError):
            continue
    
    all_threads = sorted(all_threads)
    
    # Calculate statistics for each characteristic and configuration
    # Structure: {characteristic: {(version, machine, threads): (mean, std)}}
    stats = defaultdict(dict)
    for char in characteristics:
        for key, values in processed_data[char].items():
            if values:
                mean = np.mean(values)
                std = np.std(values)
                stats[char][key] = (mean, std)
    
    # Define colors for each (version, machine) combination
    colors = {}
    color_palette = plt.cm.tab10(np.linspace(0, 1, max(10, len(all_configs))))
    for i, config in enumerate(sorted(all_configs)):
        colors[config] = color_palette[i % len(color_palette)]
    
    # For each characteristic, create 3-subplot figure (log-x, log-log, linear)
    for char in characteristics:
        fig, axes = plt.subplots(1, 3, figsize=(18, 5))
        
        # Plot for each scale type
        scale_configs = [
            ('log', 'linear', 'Log-X Scale'),
            ('log', 'log', 'Log-Log Scale'),
            ('linear', 'linear', 'Linear Scale'),
        ]
        
        for ax_idx, (xscale, yscale, title_suffix) in enumerate(scale_configs):
            ax = axes[ax_idx]
            ax.set_xscale(xscale)
            ax.set_yscale(yscale)
            ax.xaxis.set_major_locator(FixedLocator(all_threads))
            ax.set_xticklabels([str(t) for t in all_threads])
            
            # Plot one line per (version, machine) config
            for version, machine in sorted(all_configs):
                config_key = (version, machine)
                
                # Collect data points for this config
                thread_values = {}
                for threads in all_threads:
                    key = (version, machine, threads)
                    if key in stats[char]:
                        mean, std = stats[char][key]
                        thread_values[threads] = (mean, std)
                
                if not thread_values:
                    continue
                
                threads_sorted = sorted(thread_values.keys())
                means = [thread_values[t][0] for t in threads_sorted]
                stds = [thread_values[t][1] for t in threads_sorted]
                
                # Plot line and shaded region
                color = colors[config_key]
                label = f'{version} ({machine})'
                ax.plot(threads_sorted, means, 'o-', color=color, label=label, linewidth=2, markersize=6)
                
                # Add shaded region for ±1 std dev
                means_arr = np.array(means)
                stds_arr = np.array(stds)
                ax.fill_between(threads_sorted, means_arr - stds_arr, means_arr + stds_arr, 
                               color=color, alpha=0.2)
            
            ax.set_xlabel('Threads')
            ax.set_ylabel(char)
            ax.set_title(f'{char} vs Threads ({title_suffix})')
            ax.legend(loc='best', fontsize=9)
            ax.grid(True, alpha=0.3)
        
        plt.tight_layout()
        output_file = graphs_dir / f"{char}_graphs.png"
        plt.savefig(output_file, dpi=100, bbox_inches='tight')
        print(f"Graphs for {char} saved to {output_file}")
        plt.close()
    
    # Create combined figure with all characteristics (3 x num_characteristics subplots)
    num_chars = len(characteristics)
    fig, axes = plt.subplots(3, num_chars, figsize=(6 * num_chars, 15))
    
    # Ensure axes is 2D even if num_chars == 1
    if num_chars == 1:
        axes = axes.reshape(-1, 1)
    
    scale_configs = [
        ('log', 'linear', 'Log-X Scale'),
        ('log', 'log', 'Log-Log Scale'),
        ('linear', 'linear', 'Linear Scale'),
    ]
    
    for row_idx, (xscale, yscale, title_suffix) in enumerate(scale_configs):
        for col_idx, char in enumerate(characteristics):
            ax = axes[row_idx, col_idx]
            ax.set_xscale(xscale)
            ax.set_yscale(yscale)
            ax.xaxis.set_major_locator(FixedLocator(all_threads))
            ax.set_xticklabels([str(t) for t in all_threads])
            
            # Plot one line per (version, machine) config
            for version, machine in sorted(all_configs):
                config_key = (version, machine)
                
                # Collect data points for this config
                thread_values = {}
                for threads in all_threads:
                    key = (version, machine, threads)
                    if key in stats[char]:
                        mean, std = stats[char][key]
                        thread_values[threads] = (mean, std)
                
                if not thread_values:
                    continue
                
                threads_sorted = sorted(thread_values.keys())
                means = [thread_values[t][0] for t in threads_sorted]
                stds = [thread_values[t][1] for t in threads_sorted]
                
                # Plot line and shaded region
                color = colors[config_key]
                label = f'{version} ({machine})'
                ax.plot(threads_sorted, means, 'o-', color=color, label=label, linewidth=2, markersize=6)
                
                # Add shaded region for ±1 std dev
                means_arr = np.array(means)
                stds_arr = np.array(stds)
                ax.fill_between(threads_sorted, means_arr - stds_arr, means_arr + stds_arr, 
                               color=color, alpha=0.2)
            
            ax.set_xlabel('Threads')
            ax.set_ylabel(char)
            ax.set_title(f'{char} ({title_suffix})')
            if row_idx == 0 and col_idx == 0:
                ax.legend(loc='best', fontsize=8)
            ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    output_file = graphs_dir / "all_characteristics_graphs.png"
    plt.savefig(output_file, dpi=100, bbox_inches='tight')
    print(f"Combined graphs saved to {output_file}")
    plt.close()
    
    print(f"Generated {len(characteristics)} characteristic graphs with 3 scale types each")
    print(f"Plus 1 combined graph with all characteristics")
