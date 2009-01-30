#!/usr/bin/env python
import sys
import os
import time
from subprocess import *

if (len(sys.argv) > 1):
  perftest_dir = sys.argv[1]
else:
  perftest_dir = "perftests"

binary_dir = "tmp"
out_bin = os.path.join(binary_dir, "perfout")
compiler_dir = "bin"
compiler_bin = os.path.join(compiler_dir, "minnowc")

if (not os.path.isdir(binary_dir)):
  os.mkdir(binary_dir)

total_test_count = 0
total_time = 0
failed_tests = []

def tick_good():
  sys.stdout.write(".")
  sys.stdout.flush()

def tick_bad():
  sys.stdout.write("X")
  sys.stdout.flush()

def test_directory(d):
  global total_test_count, total_time

  print("Testing: " + d)
  perftests = os.listdir(d)
  for f in perftests:
    if os.path.isdir(os.path.join(d, f)):
      if (f == ".svn"):
        pass
      else:
        test_directory(os.path.join(d, f))
        print("")
    elif f[-3:] == "arg":
      pass 
    else:
      total_test_count += 1
      s = os.path.join(d, f)
      args = s[:-3] + "arg"

      tim_cmp = ""

      if (os.path.isfile(args)):
        args_file = open(args, 'r')
        args_contents = args_file.read()

      output = Popen([compiler_bin, s, "-O3", "-o", out_bin], stdout=PIPE, stderr=PIPE).communicate()

      if (len(output[1]) > 0):
        failed_tests.append( (s, output[1]) )
        tick_bad()
      else:
        args_array = args_contents.split(" ")
        c = time.time()
        #print(["time", out_bin] + args_array)
        Popen([out_bin] + args_array, stdout=PIPE, stderr=PIPE).communicate()
        c2 = time.time()
        print("%s: %.3f secs" % (s, c2-c))
        total_time = total_time + (c2-c)
        #print(bin_output)

test_directory(perftest_dir)
print("")
if (len(failed_tests) > 0):
  print("Failed tests: " + str(len(failed_tests)) + " of " + str(total_test_count))
  for i in failed_tests:
    print(i)
else:
  print("All tests (" + str(total_test_count) + " of " + str(total_test_count) + ") pass.")
  print("Time: %.3f secs" % total_time)

#output = Popen(["ls", "-l"], stdout=PIPE).communicate()[0]
#print output
