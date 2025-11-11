#!/usr/bin/env python3
"""
Parallel Performance Testing Script for SANA

Simple CLI for parallel SANA performance testing with multiple modes.

Commands:
  ./ppt.py compute [N] - Submit jobs and wait for completion (N runs per config, default 1)
  ./ppt.py extract     - Parse results from output files into CSV
  ./ppt.py graph       - Visualize results with performance graphs
  ./ppt.py clear       - Empty output directory
"""

import os
import re
import sys
import csv
import textwrap
import subprocess
from pathlib import Path
from typing import Dict, List, Tuple, Optional

os.environ['PYTHONDONTWRITEBYTECODE'] = '1'

# Global verbose flag for logging
VERBOSE = False

# ============================================================================
# CONFIGURATION: Edit these to customize test parameters
# ============================================================================

# SANA executables: Map from version string to executable path
SANA_EXECUTABLES = {
    "2.1": "./sana2.1",
    "3.3": "./sana3.3",
}

# SANA command arguments: Map from version string to command template
SANA_COMMANDS = {
    "2.1": "SANA_THREADS=\"{threads}\" /usr/bin/time -f 'TIMING: real=%e user=%U sys=%S' {executable} -g1 yeast -g2 human -ec 1 -tolerance 0 -it 100000000 -tinitial 0.01 -tdecay 6",
    "3.3": "SANA_THREADS=\"{threads}\" /usr/bin/time -f 'TIMING: real=%e user=%U sys=%S' {executable} -g1 yeast -g2 human -ec 1 -tolerance 0 -it 100000000 -tinitial 0.01 -tdecay 6 -maxthreads {threads}",
}   

# SLURM failure patterns: Regex patterns that tell us a job won't progress.
SANA_FAILURE_PATTERNS = [
    r"\(QOSMaxCpuPerNode\)",
    r"\(QOSMaxMemoryPerNode\)",
    r"\(ReqNodeNotAvail,\s*UnavailableNodes:[^)]+\)",
]

# Test configuration specification: Define versions, cores, and machines
TEST_CONFIGS_SPEC = {
    "versions": ["2.1", "3.3"],
    "cores": [1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20],
    "machines": ["odin", "cirnicus-11"]
}

# Generate TEST_CONFIGS from spec (version outer, cores middle, machines inner)
TEST_CONFIGS = [
    (version, cores, machine)
    for version in TEST_CONFIGS_SPEC["versions"]
    for cores in TEST_CONFIGS_SPEC["cores"]
    for machine in TEST_CONFIGS_SPEC["machines"]
]

# Detection patterns for extracting statistics from output files
DETECTION_ORDER = ["RealTime", "UserTime", "SystemTime", "FinalScore"]
DETECTIONS = {
    "RealTime": r"TIMING: real=([0-9.]+)",
    "UserTime": r"TIMING:.*user=([0-9.]+)",
    "SystemTime": r"TIMING:.*sys=([0-9.]+)",
    "FinalScore": r"100%.*score = ([0-9.e+-]+)",
}

# ============================================================================


def log_message(message: str, fp: Optional = None) -> None:
    """
    Log a message if VERBOSE is enabled.
    
    Args:
        message: Message to log
        fp: Optional file pointer to write to. If None, message is not logged.
    """
    global VERBOSE
    if VERBOSE and fp:
        fp.write(message + "\n")
        fp.flush()


class ParallelPerformanceTester:
    """Core configuration and helper methods for SANA performance testing."""

    def __init__(self):
        """Initialize paths and use global configurations."""
        self.script_dir = Path(__file__).parent.absolute()
        self.sana_dir = self.script_dir.parent.parent
        self.output_dir = self.script_dir / "output"
        self.results_dir = self.script_dir / "results"
        self.sbatch_scripts_dir = self.script_dir / "sbatch_scripts"
        self.modes_dir = self.script_dir / "modes"
        
        # Use global configuration
        self.sana_executables = SANA_EXECUTABLES
        self.sana_commands = SANA_COMMANDS
        self.sana_failure_patterns = SANA_FAILURE_PATTERNS
        self.test_configs = TEST_CONFIGS
        self.detection_order = DETECTION_ORDER
        self.detections = DETECTIONS

    def ensure_directories(self) -> None:
        """Create necessary directories if they don't exist."""
        for directory in [self.output_dir, self.results_dir, self.sbatch_scripts_dir]:
            directory.mkdir(parents=True, exist_ok=True)

    def create_sbatch_script(self, sana_version: str, threads: int, machine: str, output_file: Path) -> Path:
        """
        Create an SBATCH script for a specific configuration.

        Args:
            sana_version: Version of SANA (e.g., "2.1", "3.3")
            threads: Number of threads to use
            machine: Machine to run on (e.g., "odin", "tristram")
            output_file: Path where output will be written

        Returns:
            Path to the created SBATCH script
        """
        script_path = self.sbatch_scripts_dir / f"sbatch_{sana_version}_{threads}_{machine}.sh"
        
        sana_exe = self.sana_executables.get(sana_version)
        if not sana_exe:
            raise ValueError(f"Unknown SANA version: {sana_version}")
        
        cmd_template = self.sana_commands.get(sana_version)
        if not cmd_template:
            raise ValueError(f"No command template for SANA version: {sana_version}")
        
        sana_cmd = cmd_template.format(executable=sana_exe, threads=threads)
        
        script_content = textwrap.dedent(f"""\
            #!/bin/bash
            #SBATCH --job-name=sana-PPT-{sana_version}-{threads}
            #SBATCH --nodes=1
            #SBATCH --ntasks-per-node=1
            #SBATCH --cpus-per-task={threads}
            #SBATCH --nodelist={machine}
            #SBATCH --output={output_file}
            #SBATCH --error={output_file}

            cd {self.sana_dir}
            {sana_cmd}
            """)
        
        script_path.write_text(script_content)
        script_path.chmod(0o755)
        return script_path

    @staticmethod
    def extract_progress_percentage(output_file: Path) -> Optional[float]:
        """
        Extract the last printed progress percentage from an output file.
        
        Args:
            output_file: Path to the output file
            
        Returns:
            Progress percentage (0-100) or None if not found
        """
        if not output_file.exists():
            return None
        
        try:
            content = output_file.read_text()
        except Exception:
            return None
        
        # Match percentages like (0%), (10%), (100%), (25.5%), etc. or just 0%, 10%, 100%, 25.5%, etc.
        matches = re.findall(r'\(?(\d{1,3}(?:\.\d+)?)%\)?', content)
        
        if matches:
            last_percentage = float(matches[-1])
            return max(0.0, min(100.0, last_percentage))
        
        return None

    @staticmethod
    def make_progress_bar(current: int, total: int, width: int = 10) -> str:
        """Create a small fixed-width progress bar."""
        if total <= 0:
            filled = 0
        else:
            filled = int(width * current / total)
        filled = min(filled, width)
        empty = width - filled
        return f"[{'█' * filled}{'░' * empty}]"

    @staticmethod
    def get_squeue_status(job_id: str) -> Optional[str]:
        """Get job status from squeue (NODELIST(REASON) column)."""
        try:
            result = subprocess.run(
                ["squeue", "-j", job_id, "-h", "-o", "%R"],
                capture_output=True,
                text=True,
                timeout=5
            )
            status = result.stdout.strip()
            if status:
                return status[:40] if len(status) > 40 else status
            return None
        except Exception:
            return None

    @staticmethod
    def get_job_elapsed_time(job_id: str) -> Optional[str]:
        """Get job elapsed time from squeue (%M format: HH:MM:SS)."""
        try:
            result = subprocess.run(
                ["squeue", "-j", job_id, "-h", "-o", "%M"],
                capture_output=True,
                text=True,
                timeout=5
            )
            elapsed = result.stdout.strip()
            if elapsed:
                return elapsed
            return None
        except Exception:
            return None

    @staticmethod
    def is_job_complete(
        status: Optional[str],
        output_file: Path,
        detection_patterns: Dict[str, str],
        failure_patterns: List[str]
    ) -> Tuple[bool, Optional[str]]:
        """
        Check if job is complete by checking status and output file.
        
        Args:
            status: SLURM status string from squeue NODELIST(REASON)
            output_file: Path to job output file
            detection_patterns: Dict of regex patterns for completion detection
            failure_patterns: List of regex patterns that indicate job failure/stopping conditions
            
        Returns:
            Tuple of (is_complete, reason)
            - is_complete: True if job is complete
            - reason: String explaining why (e.g., "Failure pattern matched: (QOSLimited)", "FinalScore found", etc.)
        """
        if status:
            for pattern in failure_patterns:
                if re.search(pattern, status):
                    return True, f"Failure pattern matched: {status}"
        
        if output_file.exists():
            try:
                content = output_file.read_text()
                if "FinalScore" in detection_patterns:
                    pattern = detection_patterns["FinalScore"]
                    if re.search(pattern, content):
                        return True, "FinalScore found in output (successful completion)"
            except Exception:
                pass
        
        return False, None


def _submit_compute_to_slurm(tester: "ParallelPerformanceTester", num_runs: int) -> None:
    """
    Submit the compute command itself to SLURM, allowing it to run detached from SSH.
    
    Args:
        tester: ParallelPerformanceTester instance
        num_runs: Number of runs per configuration
    """
    # First clear outputs
    tester.ensure_directories()
    files_removed = 0
    for directory in [tester.output_dir, tester.sbatch_scripts_dir]:
        for file_path in directory.glob("*"):
            if file_path.is_file():
                file_path.unlink()
                files_removed += 1
    print(f"Cleared {files_removed} file(s)")
    
    # Get the script path (ppt.py)
    script_path = Path(__file__).resolve()
    
    # Submit to SLURM via command line
    try:
        result = subprocess.run(
            [
                "sbatch",
                "-J", "ppt-orch",
                "-N", "1",
                "--ntasks-per-node=1",
                "-c", "1",
                "-o", "/dev/null",
                "-e", "/dev/null",
                "--wrap", f"cd {tester.sana_dir} && {script_path} compute {num_runs}"
            ],
            capture_output=True,
            text=True,
            timeout=10
        )
        if result.returncode == 0:
            print(f"Submitted compute job to SLURM: {result.stdout.strip()}")
            print("Exiting...")
            sys.exit(0)
        else:
            print(f"Error submitting job: {result.stderr}", file=sys.stderr)
            sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


def print_help() -> None:
    """Print usage information."""
    print("Usage: ppt.py {compute [N] [--slurm]|extract|graph|kill|clear} [-v]", file=sys.stderr)
    print()
    print("Commands:")
    print("  compute [N] [--slurm] - Submit jobs (N runs per config, default 1)")
    print("                          Use --slurm to submit the compute job itself to SLURM")
    print("  extract               - Parse results from output files into CSV")
    print("  graph                 - Visualize results with performance graphs")
    print("  kill                  - Cancel all orchestration jobs (ppt-orch)")
    print("  clear                 - Empty output directory")
    print()
    print("Flags:")
    print("  -v                    - Verbose logging (for compute command)")
    print()
    print("Examples:")
    print("  ./ppt.py compute              # Run each config once (waits for completion)")
    print("  ./ppt.py compute 3            # Run each config 3 times")
    print("  ./ppt.py compute 3 --slurm    # Submit compute job to SLURM (detached)")
    print("  ./ppt.py compute -v           # Run with verbose logging")
    print("  ./ppt.py kill                 # Cancel all running orchestration jobs")
    print("  ./ppt.py extract              # Extract statistics")
    print("  ./ppt.py graph                # Generate graphs")


def main() -> None:
    """Entry point for the script."""
    global VERBOSE
    
    if len(sys.argv) < 2:
        print_help()
        sys.exit(1)
    
    task = sys.argv[1]
    additional_args = sys.argv[2:] if len(sys.argv) > 2 else ()
    
    if "-v" in additional_args:
        VERBOSE = True
        additional_args = tuple(arg for arg in additional_args if arg != "-v")
    
    # Check for --slurm flag in compute command
    use_slurm = False
    if task == "compute" and "--slurm" in additional_args:
        use_slurm = True
        additional_args = tuple(arg for arg in additional_args if arg != "--slurm")
    
    tester = ParallelPerformanceTester()
    
    try:
        if task == "compute":
            from modes.compute import run_compute
            num_runs = 1
            if additional_args:
                try:
                    num_runs = int(additional_args[0])
                except ValueError:
                    print(f"Error: invalid number of runs '{additional_args[0]}'", file=sys.stderr)
                    sys.exit(1)
            
            # If --slurm flag is set, submit this command to SLURM instead of running directly
            if use_slurm:
                _submit_compute_to_slurm(tester, num_runs)
            else:
                run_compute(tester, num_runs)
        
        elif task == "extract":
            from modes.extract import run_extract
            run_extract(tester)
        
        elif task == "graph":
            from modes.graph import run_graph
            run_graph(tester)
        
        elif task == "kill":
            from modes.kill import run_kill
            run_kill(tester)
        
        elif task == "clear":
            tester.ensure_directories()
            files_removed = 0
            for directory in [tester.output_dir, tester.sbatch_scripts_dir]:
                for file_path in directory.glob("*"):
                    if file_path.is_file():
                        file_path.unlink()
                        files_removed += 1
            print(f"Cleared {files_removed} file(s)")
        
        else:
            print_help()
            sys.exit(1)
    
    except ImportError as e:
        print(f"Error: Failed to import mode. Make sure the mode is in the correct directory - {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
