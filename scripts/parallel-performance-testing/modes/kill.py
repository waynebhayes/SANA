"""Kill mode: Cancel orchestration jobs."""

import sys
import subprocess


def run_kill(tester) -> None:
    """
    Find and cancel all orchestration jobs (ppt-orch) by name.
    
    Args:
        tester: ParallelPerformanceTester instance (unused, kept for consistency)
    """
    try:
        # Get list of jobs with name "ppt-orch"
        result = subprocess.run(
            ["squeue", "-h", "-n", "ppt-orch", "-o", "%i"],
            capture_output=True,
            text=True,
            timeout=10
        )
        
        job_ids = result.stdout.strip().split('\n')
        job_ids = [jid for jid in job_ids if jid.strip()]
        
        if not job_ids:
            print("No orchestration jobs (ppt-orch) found")
            return
        
        # Cancel each job
        cancelled = 0
        for job_id in job_ids:
            try:
                subprocess.run(["scancel", job_id], timeout=5)
                print(f"Cancelled job {job_id}")
                cancelled += 1
            except Exception:
                pass
        
        print(f"Cancelled {cancelled} orchestration job(s)")
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)



