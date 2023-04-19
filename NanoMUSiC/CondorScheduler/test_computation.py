#!/usr/bin/env python3

import argparse


def summer(a, b):
    return a + b


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("b", type=int)
    args = parser.parse_args()
    print(summer(args.b, 2))


if __name__ == "__main__":
    main()
