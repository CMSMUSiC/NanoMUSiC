import argparse

import luigi
from luigi.task import flatten
from luigi.task_register import Register

# load all tasks in the Register
import luigi_music.tasks

from luigi_music.tasks import dump_setenv

def main():
    args = parse_args()
    dump_setenv_script(args.task, args.filename)

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("task")
    parser.add_argument("filename")
    return parser.parse_args()

def dump_setenv_script( task_name, filename ):
    task_cls = Register.get_task_cls( task_name )
    instance = task_cls()
    dump_setenv( instance, filename )

if __name__ == "__main__":
    main()
