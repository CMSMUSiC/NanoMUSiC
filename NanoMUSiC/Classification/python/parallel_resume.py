import subprocess
import time
import os
import sys
from datetime import datetime
from typing import Tuple, Dict, Union, Optional


def parallel_resume_loop(
    joblog_path: str,
    parallel_command: str,
    max_attempts: int = 10,
    wait_between_attempts: int = 30,
    max_runtime_hours: float = 24,
    preamble: Optional[str] = None,
    epilog: Optional[str] = None,
) -> Tuple[str, Dict[str, Union[int, float]]]:
    """
    Run parallel --resume-failed in a loop until all jobs complete or conditions are met.
    Shows real-time output from parallel including ETA updates.

    Args:
        joblog_path: Path to the job log file
        parallel_command: String containing the parallel command
        max_attempts: Maximum number of resume attempts
        wait_between_attempts: Seconds to wait between attempts
        max_runtime_hours: Maximum total runtime in hours
        preamble: Optional shell command to execute before the first parallel run
        epilog: Optional shell command to execute after successful completion

    Returns:
        tuple: (reason_for_stopping, final_stats_dict)
    """
    start_time: datetime = datetime.now()

    def parse_joblog() -> Dict[str, Union[int, float]]:
        """Parse the job log to get statistics about job status."""
        if not os.path.exists(joblog_path):
            return {"total": 0, "completed": 0, "failed": 0, "success_rate": 0.0}

        try:
            with open(joblog_path, "r") as f:
                lines = f.readlines()

            # Skip header line
            job_lines: list[str] = [line.strip() for line in lines[1:] if line.strip()]

            total_jobs: int = len(job_lines)
            if total_jobs == 0:
                return {"total": 0, "completed": 0, "failed": 0, "success_rate": 0.0}

            # Count jobs by exit code (column 6, 0-indexed)
            completed_jobs: int = 0
            failed_jobs: int = 0

            for line in job_lines:
                parts: list[str] = line.split("\t")
                if len(parts) >= 7:
                    try:
                        exit_code: int = int(parts[6])
                        if exit_code == 0:
                            completed_jobs += 1
                        else:
                            failed_jobs += 1
                    except ValueError:
                        continue

            success_rate: float = (
                (completed_jobs / total_jobs) * 100 if total_jobs > 0 else 0.0
            )

            return {
                "total": total_jobs,
                "completed": completed_jobs,
                "failed": failed_jobs,
                "success_rate": success_rate,
            }
        except Exception as e:
            print(f"Error parsing job log: {e}")
            return {"total": 0, "completed": 0, "failed": 0, "success_rate": 0.0}

    def check_stopping_conditions(
        stats: Dict[str, Union[int, float]], attempt: int
    ) -> Tuple[bool, Optional[str]]:
        """Check if we should stop the loop."""
        runtime_hours: float = (datetime.now() - start_time).total_seconds() / 3600

        # Stop if all jobs completed
        if stats["failed"] == 0 and stats["total"] > 0:
            print(f"✅ All {stats['total']} jobs completed successfully!")
            return True, "all_completed"

        # Stop if max attempts reached
        if attempt >= max_attempts:
            print(f"❌ Maximum attempts ({max_attempts}) reached")
            return True, "max_attempts"

        # Stop if max runtime reached
        if runtime_hours >= max_runtime_hours:
            print(f"❌ Maximum runtime ({max_runtime_hours} hours) reached")
            return True, "max_runtime"

        # Stop if no failed jobs to retry
        if stats["total"] > 0 and stats["failed"] == 0:
            print("✅ No failed jobs to retry")
            return True, "no_failed_jobs"

        return False, None

    def run_parallel_resume() -> Tuple[bool, str, str]:
        """Execute parallel --resume-failed command with real-time output."""
        # cmd: str = f"parallel --resume-failed --joblog {joblog_path} {parallel_command.split('parallel', 1)[1] if parallel_command.startswith('parallel') else parallel_command}"
        cmd = parallel_command

        print(f"Running: {cmd}")

        try:
            parallel_proc: subprocess.Popen[str] = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                shell=True,
            )

            print()
            max_line_size: int = 0
            if parallel_proc.stdout:
                for line in iter(parallel_proc.stdout.readline, ""):
                    if line.startswith("ETA"):
                        line = line.replace("\n", "")
                        max_line_size = max(max_line_size, len(line))
                        print(" " * max_line_size, end="\r")
                        print(line.replace("\n", ""), end="\r")
                        sys.stdout.flush()  # Ensure it prints in real-time
                    else:
                        print(line, end="")
            print()

            if parallel_proc.stdout:
                parallel_proc.stdout.close()
            return_code: int = parallel_proc.wait()
            os.system("date")

            return return_code == 0, "", ""

        except subprocess.TimeoutExpired:
            print("⚠️  Command timed out")
            return False, "", "Command timed out"
        except Exception as e:
            print(f"⚠️  Error running command: {e}")
            return False, "", str(e)

    def execute_preamble() -> None:
        """Execute the preamble command if provided."""
        if preamble:
            print(f"🔧 Executing preamble: {preamble}")
            try:
                result: subprocess.CompletedProcess[str] = subprocess.run(
                    preamble,
                    shell=True,
                    text=True,
                    capture_output=True,
                    timeout=300,  # 5 minute timeout for preamble
                )
                if result.returncode == 0:
                    print("✅ Preamble executed successfully")
                    if result.stdout.strip():
                        print(f"Output: {result.stdout.strip()}")
                else:
                    print(f"⚠️  Preamble failed with exit code {result.returncode}")
                    if result.stderr.strip():
                        print(f"Error: {result.stderr.strip()}")
            except subprocess.TimeoutExpired:
                print("⚠️  Preamble timed out after 5 minutes")
            except Exception as e:
                print(f"⚠️  Error executing preamble: {e}")

    def execute_epilog() -> None:
        """Execute the epilog command if provided."""
        if epilog:
            print(f"🎉 Executing epilog: {epilog}")
            try:
                result: subprocess.CompletedProcess[str] = subprocess.run(
                    epilog,
                    shell=True,
                    text=True,
                    capture_output=True,
                    timeout=600,  # 10 minute timeout for epilog
                )
                if result.returncode == 0:
                    print("✅ Epilog executed successfully")
                    if result.stdout.strip():
                        print(f"Output: {result.stdout.strip()}")
                else:
                    print(f"⚠️  Epilog failed with exit code {result.returncode}")
                    if result.stderr.strip():
                        print(f"Error: {result.stderr.strip()}")
            except subprocess.TimeoutExpired:
                print("⚠️  Epilog timed out after 10 minutes")
            except Exception as e:
                print(f"⚠️  Error executing epilog: {e}")

    # Main loop
    print(f"Starting parallel resume loop at {start_time}")
    print(f"Job log: {joblog_path}")
    print(f"Max attempts: {max_attempts}")
    print(f"Wait between attempts: {wait_between_attempts}s")
    print(f"Max runtime: {max_runtime_hours}h")
    if preamble:
        print(f"Preamble: {preamble}")
    if epilog:
        print(f"Epilog: {epilog}")
    print("-" * 60)

    # Execute preamble once before starting the loop
    execute_preamble()

    attempt: int = 0

    while True:
        attempt += 1
        print(f"\n🔄 Attempt {attempt}/{max_attempts}")

        # Parse current job status
        stats: Dict[str, Union[int, float]] = parse_joblog()
        print(
            f"📊 Status: {stats['completed']}/{stats['total']} completed "
            f"({stats['success_rate']:.1f}% success rate), "
            f"{stats['failed']} failed"
        )

        # Check stopping conditions
        should_stop: bool
        reason: Optional[str]
        should_stop, reason = check_stopping_conditions(stats, attempt)
        if should_stop:
            print(f"\n🛑 Stopping: {reason}")

            # Execute epilog only on successful completion
            if reason == "all_completed":
                execute_epilog()

            return reason, stats

        # Run parallel --resume-failed
        success: bool
        stdout: str
        stderr: str
        success, stdout, stderr = run_parallel_resume()

        if success:
            print("✅ Parallel command completed successfully")
        else:
            print("❌ Parallel command failed")
            if stderr:
                print(f"Error: {stderr}")

        # Wait before next attempt (except on last iteration)
        if attempt < max_attempts:
            print(f"⏳ Waiting {wait_between_attempts} seconds before next attempt...")
            time.sleep(wait_between_attempts)

    return "max_attempts", stats


# Example usage
if __name__ == "__main__":
    # Your parallel command as a string (without --resume-failed --joblog)
    parallel_cmd: str = """parallel --halt now,fail=5 --eta --progress --noswap --retries 2 --memfree 1G 'process_file {}' ::: file1 file2 file3 file4 file5"""

    # Optional preamble to run before first attempt only
    preamble_cmd: str = "echo 'Initializing batch processing...' && free -h && df -h /tmp && mkdir -p output_dir"

    # Optional epilog to run after successful completion
    epilog_cmd: str = (
        "echo 'All jobs completed successfully!' && date && ls -la output_dir/ | wc -l"
    )

    try:
        reason: str
        final_stats: Dict[str, Union[int, float]]
        reason, final_stats = parallel_resume_loop(
            joblog_path="job.log",
            parallel_command=parallel_cmd,
            max_attempts=20,
            wait_between_attempts=60,
            max_runtime_hours=12,
            preamble=preamble_cmd,
            epilog=epilog_cmd,
        )

        print("\n" + "=" * 60)
        print("FINAL RESULTS:")
        print(f"Reason for stopping: {reason}")
        print(f"Total jobs: {final_stats['total']}")
        print(f"Completed: {final_stats['completed']}")
        print(f"Failed: {final_stats['failed']}")
        print(f"Success rate: {final_stats['success_rate']:.1f}%")

        # Exit based on results
        if final_stats["failed"] == 0 and final_stats["total"] > 0:
            sys.exit(0)  # Success
        else:
            print("ERROR: Some jobs still failed.", file=sys.stderr)
            sys.exit(-1)  # Some jobs still failed

    except KeyboardInterrupt:
        print("\n\n⚠️  Interrupted by user (Ctrl+C)")
        sys.exit(130)
