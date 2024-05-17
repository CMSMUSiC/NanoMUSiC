#!/usr/bin/env python3

from multiprocessing import Pool
import tqdm
from contextlib import contextmanager
import sys
import os
import argparse
import time
import json
import copy

from CRABAPI.RawCommand import crabCommand


html_loading = r"""
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta http-equiv="refresh" content="5" >
    <title>CRAB MUSiC</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-9ndCyUaIbzAi2FUVXJi0CjmCapSmO7SnpJef0486qhLnuZ2cdeRhO02iuK6FUUVM" crossorigin="anonymous">
  </head>
  <body>
    <div class="container my-2">
      <h1>CRAB MUSiC - Skimmer Monitor</h1><br/><br/>
      Input file: __INPUT_FILE__
      <div class="col-lg-20 px-0">
        <h2>Loading <div class="spinner-border" role="status">
            <span class="visually-hidden">Loading...</span>
        </div></h2>
      </div>
    </div>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js" integrity="sha384-geWF76RCwLtnZ8qwWowPQNguL3RmwHVBC9FhGdlKrxdiJJigb/j/68SIy3Te4Bkz" crossorigin="anonymous"></script>
  </body>
</html>
"""


html_body_upper = r"""
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="refresh" content="60" >
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>CRAB MUSiC</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-9ndCyUaIbzAi2FUVXJi0CjmCapSmO7SnpJef0486qhLnuZ2cdeRhO02iuK6FUUVM" crossorigin="anonymous">
    </head>
  <body>
    <div class="container my-2">
      <h1>CRAB MUSiC - Skimmer Monitor</h1><br/><br/>
      Input file: __INPUT_FILE__

      <div class="col-lg-20 px-0">
        <table class="table table-hover table-striped table-sm">
  <thead>
    <tr>
      <th scope="col">#</th>
      <th scope="col">Sample</th>
      <th scope="col">Year</th>
      <th scope="col">Status</th>
      <th scope="col"></th>
    </tr>
  </thead>
  <tbody>
"""


html_body_lower = r"""
  </tbody>
</table>
      </div>
    </div>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js" integrity="sha384-geWF76RCwLtnZ8qwWowPQNguL3RmwHVBC9FhGdlKrxdiJJigb/j/68SIy3Te4Bkz" crossorigin="anonymous"></script>
  </body>
</html>

"""


html_sample = r"""
    <tr>
      <th scope="row">__INDEX__</th>
      <td>__SAMPLE__</td>
      <td>__YEAR__</td>
      <td>__STATUS__</td>
      <td><a href="__MONITORING__" target="_blank">Monitoring</a></td>
    </tr>
"""


def parse_args():
    parser = argparse.ArgumentParser(description="Monitor CRAB MUSiC jobs.")

    parser.add_argument(
        "-l",
        "--loop",
        action="store_true",
        help="Will automaticaly resubmit failed tasks.",
        required=False,
    )

    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=30,
        help="number of parallel monitoring jobs.",
        required=False,
    )

    parser.add_argument(
        "-s",
        "--submited-samples",
        help="JSON file with the list of submited samples.",
        default="",
        required=False,
    )

    parser.add_argument(
        "--kill-server",
        help="Simply kill the HTTP server.",
        action="store_true",
        default=False,
    )

    parser.add_argument(
        "--kill-all-jobs",
        help="Kill all tasks.",
        action="store_true",
        default=False,
    )

    return parser.parse_args()


@contextmanager
def suppress_stdout():
    with open(os.devnull, "w") as devnull:
        old_stdout = sys.stdout
        sys.stdout = devnull
        try:
            yield
        finally:
            sys.stdout = old_stdout


def monitor(task):
    process_name, year = task

    dir = f"crab_nano_music_{process_name}_{year}/crab_{process_name}_{year}"

    try:
        with suppress_stdout():
            res = crabCommand("status", dir=dir)

        jobsPerStatus = res["jobsPerStatus"]
        proxiedWebDir = res["proxiedWebDir"].split("/")[-1]
        task_status = res["status"]
        monitoring_url = f"https://cmsweb.cern.ch/crabserver/ui/task/{proxiedWebDir}"

        status = ""
        if "failed" in jobsPerStatus:
            status = "Failed"
        elif task_status == "COMPLETED":
            status = "Completed"
        else:
            status = task_status

        retry_command = f"crab resubmit -d {dir}"
        kill_command = f"crab kill -d {dir}"

        if res["status"] == "FAILED":
            if "Crab resubmit will not work" in res["statusFailureMsg"]:
                status = "FATAL"

        return (process_name, year, status, monitoring_url, retry_command, kill_command)

    except:
        return (process_name, year, "Staled", "", "", "")


def main():
    args = parse_args()

    if args.kill_server:
        os.system("pkill -9 -f 'python3 -m http.server 8089'")
        exit(0)

    if args.submited_samples == "":
        raise Exception(
            "ERROR: Could not start monitor. The list of submited samples (--submited-samples) was not provided."
        )

    # load submited samples
    with open(args.submited_samples) as f:
        file_contents = f.read()
    submited_samples = json.loads(file_contents)

    monitoring_args = []
    for sample in submited_samples:
        monitoring_args.append((sample["process_name"], sample["year"]))

    # launch web server
    if not args.kill_all_jobs:
        os.system("pkill -9 -f 'python3 -m http.server 8089'")
        os.system("python3 -m http.server 8089 &")
    else:
        kill_list = []
        for task in monitoring_args:
            process_name, year = task
            kill_list.append(
                f"crab kill -d crab_nano_music_{process_name}_{year}/crab_{process_name}_{year}"
            )

        # killing all jobs
        if len(kill_list) > 0:
            with Pool(min(args.jobs, len(kill_list))) as p:
                monitoring_results = list(
                    tqdm.tqdm(
                        p.imap(os.system, kill_list),
                        total=len(kill_list),
                        desc=f"Killing all jobs [{min(args.jobs, len(kill_list))} jobs] ...",
                        unit=" jobs",
                    )
                )

        exit(0)

    with open("index.html", "w") as f:
        f.write(html_loading.replace("__INPUT_FILE__", args.submited_samples))

    while True:
        # submit monitoring tasks
        with Pool(min(args.jobs, len(monitoring_args))) as p:
            monitoring_results = list(
                tqdm.tqdm(
                    p.imap(monitor, monitoring_args),
                    total=len(monitoring_args),
                    desc=f"Monitoring tasks [{min(args.jobs, len(monitoring_args))} jobs] ...",
                    unit=" tasks",
                )
            )

        results = []
        completed = 0
        failed = 0
        others = 0
        fatal = 0
        for task in monitoring_results:
            sample, year, status, monitoring, retry_command, kill_command = task
            if status == "FATAL":
                print(
                    "FATAL: Task {} - {} failed completly. Will not resubmit autmatically. Better to resubmit all tasks.".format(
                        sample, year
                    )
                )
                fatal += 1

            results.append(
                {
                    "sample": sample,
                    "year": year,
                    "status": status,
                    "monitoring": monitoring,
                    "retry_command": retry_command,
                    "kill_command": kill_command,
                }
            )
            if status == "Completed":
                completed += 1
            elif status == "Failed":
                failed += 1
            else:
                others += 1

        if fatal>0:
            p.terminate()
            sys.exit(-1)

        results = sorted(results, key=lambda r: r["sample"])

        html_content = copy.copy(html_body_upper)
        html_content += f"<p><b>Completed: {completed}<br/>Failed: {failed}<br/>Others: {others}<br/>Total: {len(results)}</b></p>"
        if completed == len(results):
            html_content += "<h2><b>All done!</b></h2>"

        for idx_res, res in enumerate(results):
            if res["status"] == "Failed" or res["status"] == "Stalled":
                html_content += (
                    copy.copy(html_sample)
                    .replace("__INDEX__", str(idx_res + 1))
                    .replace("__SAMPLE__", res["sample"])
                    .replace("__YEAR__", res["year"])
                    .replace(
                        "__STATUS__",
                        f'<span class="badge bg-danger">{(lambda status : "FAILED" if status == "Failed"  else "STALLED")(res["status"])}</span>',
                    )
                    .replace("__MONITORING__", res["monitoring"])
                )

        for idx_res, res in enumerate(results):
            if res["status"] != "Failed" and res["status"] != "Completed":
                html_content += (
                    copy.copy(html_sample)
                    .replace("__INDEX__", str(idx_res + 1))
                    .replace("__SAMPLE__", res["sample"])
                    .replace("__YEAR__", res["year"])
                    .replace(
                        "__STATUS__",
                        f'<span class="badge bg-info">{res["status"]}</span>',
                    )
                    .replace("__MONITORING__", res["monitoring"])
                )

        for idx_res, res in enumerate(results):
            if res["status"] == "Completed":
                html_content += (
                    copy.copy(html_sample)
                    .replace("__INDEX__", str(idx_res + 1))
                    .replace("__SAMPLE__", res["sample"])
                    .replace("__YEAR__", res["year"])
                    .replace(
                        "__STATUS__", '<span class="badge bg-success">COMPLETED</span>'
                    )
                    .replace("__MONITORING__", res["monitoring"])
                )

        html_content += copy.copy(html_body_lower)

        with open("index.html", "w") as f:
            f.write(html_content.replace("__INPUT_FILE__", args.submited_samples))

        if args.loop:
            resubmition_list = []
            for idx_res, res in enumerate(results):
                if res["status"] == "Failed":
                    # os.system(res["retry_command"])
                    resubmition_list.append(res["retry_command"])

            # resubmitting failed tasks
            if len(resubmition_list) > 0:
                max_resub_jobs = 20
                with Pool(min(max_resub_jobs, len(resubmition_list))) as p:
                    _ = list(
                        tqdm.tqdm(
                            p.imap(os.system, resubmition_list),
                            total=len(resubmition_list),
                            desc=f"Resubmiting tasks [{min(max_resub_jobs, len(resubmition_list))} jobs] ...",
                            unit=" tasks",
                        )
                    )
            time.sleep(60)
        else:
            exit(0)


if __name__ == "__main__":
    main()
