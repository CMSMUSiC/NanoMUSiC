#!/usr/bin/env python

import argparse
import os
import sys

from PyQt4.QtGui import QApplication

import gridlib.se
import gridlib.util

from gridbrowser import GridBrowser

def parse_args():
    cernusername = gridlib.util.get_username_cern()
    default_path = os.path.join("/store/user", cernusername)

    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--root-directory", default=default_path)
    parser.add_argument("-s", "--site", default="T2_DE_RWTH")
    args = parser.parse_args()
    return args

def main():
    args = parse_args()

    app = QApplication(sys.argv)


    #se = gridlib.se.get_se_instance(args.site)
    se = gridlib.se.StorageElement(args.site)

    browser = GridBrowser(storage_element=se, root_path=args.root_directory)
    browser.resize(1024, 768)
    browser.move(100, 100)
    browser.show()

    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
