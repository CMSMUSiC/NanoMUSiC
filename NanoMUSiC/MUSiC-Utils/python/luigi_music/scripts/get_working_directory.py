import argparse

import luigi
from luigi.task import flatten
from luigi.task_register import Register

import luigi_music.tasks

def main():
    args = parse_args()
    dir = get_working_directory(args.task)
    print(dir)

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("task")
    return parser.parse_args()

def get_working_directory( task_name ):
    task_cls = Register.get_task_cls( task_name )
    return task_cls().working_directory

if __name__ == "__main__":
    main()
