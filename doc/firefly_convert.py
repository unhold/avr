#!/usr/bin/env python2

import csv

filename = "firefly_data.csv"
rownumber = 2

values = []
with open(filename, "rb") as f:
	reader = csv.reader(f)
	for row in reader:
		values.append(row[rownumber])

rowname = values[0]
values = values[1:]

with open("firefly.h", "w") as f:
	head = """\
#ifndef FIREFLY_H
#define FIREFLY_H

#include <avr/pgmspace.h>

// %s[%d:%s]
prog_uint8_t const firefly[] = {
"""
	foot = """\
};

#endif // FIREFLY_H
"""
	f.write(head % (filename, rownumber, rowname))
	for d in values:
		f.write("    %3s, // " % (d,) + int(d)*68/256*"#" + "\n")
	f.write(foot)

