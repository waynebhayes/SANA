"""Compute mode: Submit and monitor SANA jobs."""

import sys
import time
import signal
import subprocess
import re
from pathlib import Path
from typing import Dict, Optional, TextIO


def run_compute(tester, num_runs: int = 1) -> None:
    """
    Submit jobs to SLURM and monitor their progress with table display.
    
    For each configuration, runs N iterations sequentially. As each job completes,
    the next run of that configuration is submitted. Shows progress in a table with
    progress bars.
    
    Args:
        tester: ParallelPerformanceTester instance
        num_runs: Number of times to run each configuration (default 1)
    """
    tester.ensure_directories()
    
    # Import VERBOSE flag and log helper
    import ppt
    
    # Open log file only if verbose
    log_fp = None
    if ppt.VERBOSE:
        log_file = tester.script_dir / "log.txt"
        log_fp = open(log_file, "w")
    
    # Helper to log messages
    def log_msg(msg: str) -> None:
        if log_fp:
            log_fp.write(msg + "\n" if not msg.endswith("\n") else msg)
            log_fp.flush()
    
    if num_runs < 1:
        print("Error: num_runs must be at least 1", file=sys.stderr)
        sys.exit(1)

    all_submitted_jobs = []
    
    # Signal handler to make sure a Ctrl+C doesn't leave any jobs running
    # Tip: Run `scancel -u $USER` to cancel all your jobs.
    def handle_sigint(signum, frame):
        print("\n\nCancelling all submitted jobs...")
        log_msg("User cancelled - running scancel on all jobs")
        
        for job_id in all_submitted_jobs:
            try:
                subprocess.run(["scancel", job_id], check=False)
                log_msg(f"Cancelled job {job_id}")
            except Exception:
                pass
        
        log_msg("Cancelled all jobs.")
        if log_fp:
            log_fp.close()
        print("All jobs cancelled. Exiting.")
        sys.exit(0)
    
    signal.signal(signal.SIGINT, handle_sigint)
    print("(Press Ctrl+C to cancel all jobs and exit)")
    
    files_removed = 0
    files_to_clear = []
    for directory in [tester.output_dir, tester.sbatch_scripts_dir]:
        for file_path in directory.glob("*"):
            if file_path.is_file():
                files_to_clear.append(file_path.name)
                try:
                    file_path.unlink()
                    files_removed += 1
                except Exception as e:
                    log_msg(f"Failed to delete {file_path.name}: {e}")
    
    print(f"Cleared {files_removed} existing file(s): {files_to_clear}")
    log_msg(f"Cleared {files_removed} files: {files_to_clear}")
    
    remaining = list(tester.output_dir.glob("*"))
    if remaining:
        log_msg(f"WARNING: Output directory still has files after clear: {[f.name for f in remaining]}")
    print()
    
    total_jobs = len(tester.test_configs) * num_runs
    print(f"Submitting {total_jobs} jobs ({len(tester.test_configs)} configs × {num_runs} runs)...")
    print()
    
    current_run: Dict[str, int] = {f"{v}-{t}-{m}": 0 for v, t, m in tester.test_configs}
    current_slurm_id: Dict[str, Optional[str]] = {f"{v}-{t}-{m}": None for v, t, m in tester.test_configs}
    current_output_file: Dict[str, Optional[Path]] = {f"{v}-{t}-{m}": None for v, t, m in tester.test_configs}
    current_percentage: Dict[str, Optional[float]] = {f"{v}-{t}-{m}": None for v, t, m in tester.test_configs}
    current_status: Dict[str, Optional[str]] = {f"{v}-{t}-{m}": None for v, t, m in tester.test_configs}
    
    def submit_config_job(sana_version: str, threads: int, machine: str, run_idx: int) -> Optional[Path]:
        """Submit a single job and return the output file path."""
        output_file = tester.output_dir / f"{sana_version}-{threads}-{machine}-run{run_idx}.out"
        key = f"{sana_version}-{threads}-{machine}"
        
        if output_file.exists():
            return output_file
        
        script = tester.create_sbatch_script(sana_version, threads, machine, output_file)
        
        try:
            result = subprocess.run(
                ["sbatch", str(script)],
                cwd=str(tester.sbatch_scripts_dir),
                capture_output=True,
                text=True,
                check=False
            )
            
            match = re.search(r"Submitted batch job (\d+)", result.stdout)
            if result.returncode == 0 and match:
                slurm_id = match.group(1)
                current_slurm_id[key] = slurm_id
                all_submitted_jobs.append(slurm_id)
                return output_file
            else:
                return None
        except Exception:
            return None
    
    for sana_version, threads, machine in tester.test_configs:
        key = f"{sana_version}-{threads}-{machine}"
        output_file = submit_config_job(sana_version, threads, machine, current_run[key])
        if output_file:
            current_output_file[key] = output_file
    
    render_table_header(tester)
    
    while any(current_run[f"{v}-{t}-{m}"] < num_runs for v, t, m in tester.test_configs):
        for sana_version, threads, machine in tester.test_configs:
            key = f"{sana_version}-{threads}-{machine}"
            
            if current_run[key] >= num_runs:
                current_percentage[key] = 100.0
                # Show FAILED if any run hit a failure pattern
                final_status = current_status[key] if current_status[key] == "FAILED" else "DONE"
                print(render_table_row(
                    sana_version, threads, machine,
                    num_runs, num_runs,
                    100.0, final_status, None
                ))
                continue
            
            output_file = current_output_file[key]
            slurm_id = current_slurm_id[key]
            
            if not output_file or not slurm_id:
                print(render_table_row(
                    sana_version, threads, machine,
                    current_run[key], num_runs,
                    None, None, None
                ))
                continue
            
            percentage = tester.extract_progress_percentage(output_file)
            current_percentage[key] = percentage
            
            status = tester.get_squeue_status(slurm_id)
            elapsed = tester.get_job_elapsed_time(slurm_id)
            
            is_complete, completion_reason = tester.is_job_complete(
                status,
                output_file,
                tester.detections,
                tester.sana_failure_patterns
            )
            
            if is_complete or (status is None and output_file.exists()):
                current_percentage[key] = 100.0
                
                # Set status based on completion reason
                if completion_reason and "Failure pattern matched" in completion_reason:
                    current_status[key] = "FAILED"
                    # Cancel the job if it hit a failure pattern
                    if slurm_id:
                        try:
                            subprocess.run(["scancel", slurm_id], timeout=5)
                            log_msg(f"[{key}] Cancelled job {slurm_id} (failure pattern matched)")
                        except Exception:
                            pass
                else:
                    current_status[key] = "DONE"
                
                if completion_reason:
                    log_msg(f"[{key}] Job completed: {completion_reason}")
                elif status is None and output_file.exists():
                    log_msg(f"[{key}] Job completed: No longer in squeue, output file exists")
                
                current_run[key] += 1
                
                if current_run[key] < num_runs:
                    next_output = submit_config_job(sana_version, threads, machine, current_run[key])
                    if next_output:
                        current_output_file[key] = next_output
                        current_status[key] = "PENDING"
                        current_percentage[key] = None
            else:
                current_status[key] = status if status else "RUNNING"
            
            print(render_table_row(
                sana_version, threads, machine,
                current_run[key], num_runs,
                percentage, current_status[key], elapsed
            ))
        
        if any(current_run[f"{v}-{t}-{m}"] < num_runs for v, t, m in tester.test_configs):
            time.sleep(3)
            num_rows = len(tester.test_configs)
            for _ in range(num_rows):
                print("\033[1A\033[K", end="")
    
    print()
    print("All jobs completed!")
    
    if log_fp:
        log_fp.close()


def render_table_header(tester) -> None:
    """Render the persistent table header."""
    header = (
        f"{'Version':<8} "
        f"{'Cores':<6} "
        f"{'Machine':<15} "
        f"{'Trials':<22} "
        f"{'Progress':<22} "
        f"{'Elapsed':<10} "
        f"{'Status':<25}"
    )
    print(header)
    print("-" * len(header))


def render_table_row(
    sana_version: str,
    threads: int,
    machine: str,
    current_run: int,
    total_runs: int,
    percentage: Optional[float],
    status: Optional[str],
    elapsed: Optional[str] = None
) -> str:
    """
    Render a single table row.
    
    Args:
        sana_version: SANA version
        threads: Number of threads
        machine: Machine name
        current_run: Current run number (0-indexed)
        total_runs: Total runs for this config
        percentage: Progress percentage (0-100) or None
        status: Job status string or None
        elapsed: Elapsed time string (HH:MM:SS) or None
        
    Returns:
        Formatted row string
    """
    # Trial progress: [bar] (x/N)
    trial_bar = _make_progress_bar(current_run, total_runs, width=10)
    trials_str = f"{trial_bar} ({current_run}/{total_runs})"
    
    # Program progress: [bar] xx.x%
    if percentage is not None:
        pct_str = f"{percentage:.1f}%"
        prog_bar = _make_progress_bar(int(percentage), 100, width=10)
        progress_str = f"{prog_bar} {pct_str}"
    else:
        progress_str = f"{_make_progress_bar(0, 100, width=10)} -"
    
    # Elapsed time
    elapsed_str = elapsed if elapsed else "-"
    
    # Status display
    if status is None:
        status_str = "PENDING"
    else:
        status_str = status if status else "UNKNOWN"
    
    row = (
        f"{sana_version:<8} "
        f"{str(threads):<6} "
        f"{machine:<15} "
        f"{trials_str:<22} "
        f"{progress_str:<22} "
        f"{elapsed_str:<10} "
        f"{status_str:<25}"
    )
    return row


def _make_progress_bar(current: int, total: int, width: int = 10) -> str:
    """Create a small fixed-width progress bar."""
    if total <= 0:
        filled = 0
    else:
        filled = int(width * current / total)
    filled = min(filled, width)
    empty = width - filled
    return f"[{'█' * filled}{'░' * empty}]"

