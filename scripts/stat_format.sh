#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <f4_json_file>"
    exit 1
fi

f4_json_file=$1
# python3 -m pip install prettytable jinja2
/opt/MESA/bin/fieldstat_exporter.py local -j $f4_json_file -l --clear-screen
