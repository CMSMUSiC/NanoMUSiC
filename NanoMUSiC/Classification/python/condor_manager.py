#!/usr/bin/env python3
from typing import Optional
from dataclasses import dataclass, field

import time
import os
import sys
import subprocess
import shlex
import random
from enum import IntEnum, auto
import htcondor

from rich.progress import (
    Progress,
    TimeElapsedColumn,
    SpinnerColumn,
    TextColumn,
    BarColumn,
    MofNCompleteColumn,
    TaskProgressColumn,
)

from rich.progress import track
from rich import print

import logging
from rich.logging import RichHandler

FORMAT = "%(message)s"
logging.basicConfig(
    level=logging.DEBUG,
    format="%(message)s",
    datefmt="%m/%d/%Y %I:%M:%S %p",
    handlers=[RichHandler()],
)
log = logging.getLogger("main")


class JobStatus(IntEnum):
    UNDEFINED = auto()
    BUILD = auto()
    SUBMITED = auto()
    IDLE = auto()
    RUNNING = auto()
    FAILED = auto()
    DONE = auto()
    MAX_RETRIES = auto()


condor_log_dir = "condor_logs"
if os.getenv("CONDOR_LOG_DIR"):
    condor_log_dir = os.getenv("CONDOR_LOG_DIR")


@dataclass
class CondorJob:
    """Class for keeping track of a Condor Job."""

    name: str
    actions: list[str]
    preamble: list[str] = field(default_factory=list)
    input_files: list[str] = field(default_factory=list)
    output_files: list[str] = field(default_factory=list)
    request_memory: int = 3
    max_retries: int = 5
    _num_retries: int = 0
    _submit_config: dict[str, str] = field(
        default_factory=lambda: {
            "universe": "vanilla",
            "getenv": "true",
            "use_x509userproxy": "true",
            # "executable": f"{condor_log_dir}/exec_{random.randint(0, 10000000)}.sh",
            # "log": f"{condor_log_dir}/condor-$(CLUSTER).log",
            # "output": f"{condor_log_dir}/condor-$(CLUSTER).out",
            # "error": f"{condor_log_dir}/condor-$(CLUSTER).err",
            "arguments": "$(CLUSTER) $(PROCESS)",
            "notification": "never",
            # "request_memory": "4G",
            "rank": "Memory",
            "should_transfer_files": "YES",
            "when_to_transfer_output": "ON_SUCCESS",
            # "StreamErr": "true",
            # "StreamOut": "true",
        }
    )
    _preamble: list[str] = field(
        default_factory=lambda: [
            r"#!/usr/bin/env bash",
            r"set -e",
            r"cd $_CONDOR_SCRATCH_DIR",
            r"pwd",
            r"date",
            r"hostname",
        ]
    )
    cluster_id: Optional[int] = None
    job_status: JobStatus = JobStatus.UNDEFINED
    executable_script: Optional[str] = None
    executable_script_path: Optional[str] = None
    log_file: Optional[str] = None
    stdout: Optional[str] = None
    stderr: Optional[str] = None

    def _build_executable(self) -> str:
        if len(self.actions) == 0:
            log.error("No actions were defined.")
            sys.exit(-1)
        return "\n".join(self._preamble + self.preamble + self.actions)

    def _add_input_files(self) -> None:
        if len(self.input_files):
            self._submit_config["transfer_input_files"] = ",".join(self.input_files)

    def _add_output_files(self) -> None:
        if len(self.output_files):
            self._submit_config["transfer_output_files"] = ",".join(self.output_files)

    def _add_request_memory(self) -> None:
        self._submit_config["request_memory"] = "{}G".format(self.request_memory)

    def _add_log_files(self) -> None:
        self._submit_config["log"] = "{}/{}.log".format(condor_log_dir, self.name)
        self._submit_config["output"] = "{}/{}.out".format(condor_log_dir, self.name)
        self._submit_config["error"] = "{}/{}.err".format(condor_log_dir, self.name)

    def _add_executable(self) -> None:
        self._submit_config["executable"] = "{}/{}.sh".format(condor_log_dir, self.name)

    def submit(self, schedd) -> int:
        self._add_input_files()
        self._add_output_files()
        self._add_request_memory()
        self._add_log_files()
        self._add_executable()
        self.executable_script = self._build_executable()
        self.job_status = JobStatus.BUILD

        self.executable_script_path = self._submit_config["executable"]
        try:
            with open(self.executable_script_path, "w") as exec_file:
                exec_file.write(self.executable_script)

            subprocess.run(shlex.split(f"chmod +x {self.executable_script_path}"))

            submit_result = schedd.submit(htcondor.Submit(self._submit_config))
            self.cluster_id = submit_result.cluster()
            self.log_file = submit_result.clusterad()["UserLog"]
            self.stdout = self.log_file.replace(".log", ".out")
            self.stderr = self.log_file.replace(".log", ".err")
        except RuntimeError as error:
            print("ERROR: Could not submit job. Scheduler message: {}".format(error))
            sys.exit(-1)
        else:
            self.job_status = JobStatus.SUBMITED
            return self.cluster_id

    def monitor(self, schedd, logger=log.warning) -> JobStatus:
        """Return job status."""

        # will only query jobs not DONE or exausted (reached MAX_RETRIES)
        if (
            self.job_status == JobStatus.DONE
            or self.job_status == JobStatus.MAX_RETRIES
        ):
            return self.job_status

        query_results = list(
            schedd.query(
                constraint=f"ClusterId == {self.cluster_id}",
                projection=["ClusterId", "ProcId", "JobStatus"],
            )
        )

        assert len(query_results) <= 1

        if len(query_results) == 0:
            if self.check_completion():
                self.job_status = JobStatus.DONE
                return self.job_status

            if self._num_retries <= self.max_retries:
                self.job_status = JobStatus.FAILED

                subprocess.run(
                    shlex.split(f"condor_rm {self.cluster_id}"),
                    capture_output=True,
                    text=True,
                )

                self.resubmit(schedd, logger)
                return self.job_status

            self.job_status = JobStatus.MAX_RETRIES
            return self.job_status

        query_result = query_results[0]
        if query_result["JobStatus"] == htcondor.JobStatus.IDLE:
            self.job_status = JobStatus.IDLE
            return self.job_status
        elif (
            query_result["JobStatus"] == htcondor.JobStatus.RUNNING
            or query_result["JobStatus"] == htcondor.JobStatus.TRANSFERRING_OUTPUT
            or query_result["JobStatus"] == htcondor.JobStatus.SUSPENDED
        ):
            self.job_status = JobStatus.RUNNING
            return self.job_status
        elif (
            query_result["JobStatus"] == htcondor.JobStatus.HELD
            or query_result["JobStatus"] == htcondor.JobStatus.REMOVED
        ):
            if self._num_retries <= self.max_retries:
                self.job_status = JobStatus.FAILED

                subprocess.run(
                    shlex.split(f"condor_rm {self.cluster_id}"),
                    capture_output=True,
                    text=True,
                )

                self.resubmit(schedd, logger)
                return self.job_status

            log.error(
                "There are jobs HELD or REMOVED, which reached MAX_RETRIES. Useless to go on like this... Stoping here."
            )
            sys.exit(-1)
        else:
            log.error("Could not get Job Status from Condor.")
            sys.exit(-1)

    def resubmit(self, schedd, logger=log.warning) -> None:
        logger(
            f"[yellow]Job failed: {self.name} - Cluster Id: {self.cluster_id}. Resubmiting..."
        )
        self.submit(schedd)
        self.job_status == JobStatus.SUBMITED
        self._num_retries += 1

    def check_completion(self) -> bool:
        try:
            with open(self.log_file, "r") as file:
                content = file.read()
                if (
                    "Job terminated of its own" in content
                    and "with exit-code 0" in content
                ):
                    with open(self.stderr, "r") as file:
                        content = file.read()
                        if "FATAL" not in content:
                            return True
                return False
        except FileNotFoundError:
            log.error("Log file not found.")
            return False


@dataclass
class MonitoringCounter:
    old_done: int = 0
    old_idle: int = 0
    old_max_retries: int = 0
    old_running: int = 0
    done: int = 0
    idle: int = 0
    max_retries: int = 0
    running: int = 0
    total: int = 0

    def update(self, statuses: list[JobStatus]) -> None:
        self.total = len(statuses)

        self.old_done = self.done
        self.done = sum([s == JobStatus.DONE for s in statuses])

        self.old_idle = self.idle
        self.idle = sum([s == JobStatus.IDLE for s in statuses])

        self.old_max_retries = self.max_retries
        self.max_retries = sum([s == JobStatus.MAX_RETRIES for s in statuses])

        self.old_running = self.running
        self.running = sum([s == JobStatus.RUNNING for s in statuses])


@dataclass
class CondorManager:
    """Manager for many Condor jobs."""

    jobs: list[CondorJob]
    verbose_monitoring_threshold: int = 10
    schedd = htcondor.Schedd()
    counter: MonitoringCounter = field(default_factory=MonitoringCounter)
    jobs_ids: dict[str, int] = field(default_factory=dict)

    def submit(self):
        log.info("Cleaning log files ...")
        os.system(f"mkdir -p {condor_log_dir}")
        os.system(f"rm -rf {condor_log_dir}/*")
        log.info("Starting job submition...")
        for job in track(self.jobs, description=f"Will submit {len(self.jobs)} jobs: "):
            self.jobs_ids[job.name] = job.submit(self.schedd)
        log.info("... done.")

    def nanny(self) -> None:
        log.info("Starting Nanny loop...")
        jobs_statuses = [j.monitor(self.schedd) for j in self.jobs]
        self.counter.update(jobs_statuses)
        jobs_max_retries = []

        with Progress(
            SpinnerColumn(),
            TextColumn("[progress.description]{task.description}"),
            BarColumn(),
            MofNCompleteColumn(),
            TaskProgressColumn(),
            TimeElapsedColumn(),
            transient=True,
            refresh_per_second=10,
        ) as progress:
            max_retries_tasks = progress.add_task(
                "[red]Reached max retries...",
                total=len(jobs_statuses),
            )
            done_tasks = progress.add_task(
                "[green]Done...",
                total=len(jobs_statuses),
            )
            total_tasks = progress.add_task(
                "[cyan]Total...",
                total=len(jobs_statuses),
            )

            while not all(
                [
                    s == JobStatus.DONE or s == JobStatus.MAX_RETRIES
                    for s in jobs_statuses
                ]
            ):
                proc = subprocess.run(
                    shlex.split(r"condor_q"),
                    capture_output=True,
                    text=True,
                )
                if proc.returncode != 0:
                    log.error("Could not query the Condor scheduler.")
                    log.error(proc.stderr)
                    sys.exit(-1)

                progress.console.log(
                    f"condor_q summary: {proc.stdout.splitlines()[-3:-2]}"
                )
                if (
                    len(jobs_statuses)
                    - sum(
                        [
                            s == JobStatus.DONE or s == JobStatus.MAX_RETRIES
                            for s in jobs_statuses
                        ]
                    )
                    <= self.verbose_monitoring_threshold
                ):
                    jobs_running = []
                    jobs_idle = []
                    for j in self.jobs:
                        _monitoring_result = j.monitor(
                            schedd=self.schedd,
                            logger=progress.console.log,
                        )
                        if _monitoring_result == JobStatus.RUNNING:
                            jobs_running.append(j.name)
                        if _monitoring_result == JobStatus.IDLE:
                            jobs_idle.append(j.name)
                    progress.console.log(
                        f"RUNNING - {self.counter.running}/{self.counter.total}: {', '.join(jobs_running)}"
                    )
                    progress.console.log(
                        f"IDLE - {self.counter.idle}/{self.counter.total}: {', '.join(jobs_idle)}"
                    )

                jobs_max_retries = []
                for j in self.jobs:
                    if (
                        j.monitor(schedd=self.schedd, logger=progress.console.log)
                        == JobStatus.MAX_RETRIES
                    ):
                        jobs_max_retries.append((j.name, str(j.cluster_id)))
                progress.console.log(
                    f"Reached MAX_RETRIES - {self.counter.max_retries}/{self.counter.total}: {', '.join([j[0]+' ('+j[1]+')' for j in jobs_max_retries])}"
                )
                progress.console.log(f"DONE: {self.counter.done}/{self.counter.total}")

                # update progress bars
                progress.advance(
                    max_retries_tasks,
                    advance=self.counter.max_retries - self.counter.old_max_retries,
                )
                progress.advance(
                    done_tasks, advance=self.counter.done - self.counter.old_done
                )
                progress.advance(
                    total_tasks,
                    advance=(self.counter.max_retries - self.counter.old_max_retries)
                    + (self.counter.done - self.counter.old_done),
                )

                time.sleep(5)
                jobs_statuses = [
                    j.monitor(schedd=self.schedd, logger=progress.console.log)
                    for j in self.jobs
                ]
                self.counter.update(jobs_statuses)

        jobs_max_retries = []
        for j in self.jobs:
            if j.monitor(self.schedd) == JobStatus.MAX_RETRIES:
                jobs_max_retries.append(f"{j.name} ({j.cluster_id})")
        print("[bold]\nSummary:")
        print(
            f"[bold]Reached MAX_RETRIES - {self.counter.max_retries}/{self.counter.total}: {', '.join(jobs_max_retries)}"
        )
        print(f"[bold]DONE: {self.counter.done}/{self.counter.total}")
        print()

        log.info("Done. Nanny goes home...")


if __name__ == "__main__":
    my_preamble = [
        r"source /cvmfs/sft.cern.ch/lcg/views/LCG_105a/x86_64-el9-gcc12-opt/setup.sh"
    ]
    my_jobs = [
        CondorJob(
            "job_1",
            actions=["sleep 3", "root -b -q"],
            preamble=my_preamble,
        ),
        CondorJob(
            "job_2",
            actions=["sleep 3", "root -b -q"],
            preamble=my_preamble,
        ),
        CondorJob(
            "job_3",
            actions=["sleep 3", "root -b -q"],
            preamble=my_preamble,
        ),
        CondorJob(
            "job_4",
            actions=["sleep 3", "root -b -q"],
            preamble=my_preamble,
        ),
        CondorJob(
            "job_5",
            actions=["sleep 3", "root -b -q"],
            preamble=my_preamble,
        ),
        CondorJob(
            "job_6",
            actions=["sleep 3", "root -b -q"],
            preamble=my_preamble,
        ),
        CondorJob(
            "job_7",
            actions=["sleep 3", "root -b -q"],
            preamble=my_preamble,
        ),
        CondorJob(
            "job_8",
            actions=["sleep 3", "root -b -q"],
            preamble=my_preamble,
        ),
        CondorJob(
            "job_9",
            actions=["sleep 3", "root -b -q"],
            preamble=my_preamble,
        ),
        CondorJob(
            "job_10",
            actions=["sleep 3", "root -b -q"],
            preamble=my_preamble,
        ),
        CondorJob(
            "job_11",
            actions=["sleep 3", "root -b -q"],
            preamble=my_preamble,
        ),
        CondorJob(
            "job_12",
            actions=["sleep 3", "root -b -q"],
            preamble=my_preamble,
        ),
        CondorJob(
            "different_job",
            actions=["ls"],
            preamble=[],
        ),
        CondorJob(
            "this_job_will_fail_after_3_retries",
            actions=["foo"],
            preamble=[],
            max_retries=3,
        ),
        CondorJob(
            "this_job_will_fail_after_1_retry",
            actions=["foo"],
            preamble=[],
            max_retries=1,
        ),
        CondorJob(
            "if_fail_will_not_retry",
            actions=["ls"],
            preamble=[],
            max_retries=0,
        ),
        CondorJob(
            "has_input_files",
            actions=["ls my_input_file_1", "ls my_input_file_2"],
            preamble=[],
            input_files=["my_input_file_1", "my_input_file_2"],
            max_retries=0,
        ),
    ]

    # create dummy input files
    os.system("touch my_input_file_1")
    os.system("touch my_input_file_2")

    manager = CondorManager(my_jobs)
    manager.submit()
    manager.nanny()
