#!/usr/bin/env python
import sys
import os
from subprocess import *

if (len(sys.argv) > 1):
  unittest_dir = sys.argv[1]
else:
  unittest_dir = "unittests"

binary_dir = "tmp"
out_bin = os.path.join(binary_dir, "unitout")
compiler_dir = "bin"
compiler_bin = os.path.join(compiler_dir, "minnowc")

if (not os.path.isdir(binary_dir)):
  os.mkdir(binary_dir)

total_test_count = 0
failed_tests = []

def tick_good():
  sys.stdout.write(".")
  sys.stdout.flush()

def tick_bad():
  sys.stdout.write("X")
  sys.stdout.flush()

def test_directory(d):
  global total_test_count

  print("Testing: " + d)
  unittests = os.listdir(d)
  for f in unittests:
    if os.path.isdir(os.path.join(d, f)):
      if (f == ".svn"):
        pass
      else:
        test_directory(os.path.join(d, f))
        print("")
    elif f[-3:] == "err":
      pass 
    elif f[-3:] == "tst":
      pass 
    else:
      total_test_count += 1
      s = os.path.join(d, f)
      tst = s[:-3] + "tst"
      err = s[:-3] + "err"

      tst_cmp = ""
      err_cmp = ""

      if (os.path.isfile(tst)):
        tst_file = open(tst, 'rU')
        tst_cmp = tst_file.read()

      if (os.path.isfile(err)):
        err_file = open(err, 'rU')
        err_cmp = err_file.read()

      output = Popen([compiler_bin, s,  "-o", out_bin], stdout=PIPE, stderr=PIPE).communicate()

      if (len(output[1]) > 0):
        output = (output[0].replace('\r\n', '\n'), output[1].replace('\r\n', '\n'))
        output = (output[0].replace("\\", "/"), output[1].replace("\\", "/"))
        if (err_cmp != output[1]):
          failed_tests.append( (s, output[1]) )
          tick_bad()
        else:
          tick_good()
      else:
        if (os.path.isfile(tst)):
          tst_file = open(tst, 'rU')
          tst_cmp = tst_file.read()

          bin_output = Popen([out_bin], stdout=PIPE, stderr=PIPE).communicate()
          bin_output = (bin_output[0].replace('\r\n', '\n'), bin_output[1].replace('\r\n', '\n'))
          bin_output = (bin_output[0].replace("\\", "/"), bin_output[1].replace("\\", "/"))
          if (len(bin_output[1]) > 0):
            if (err_cmp != output[1]):
              failed_tests.append( (s, bin_output[1]) )
              tick_bad()
            else:
              tick_good()
          elif (tst_cmp != bin_output[0]):
              failed_tests.append( (s, bin_output[0]) )
              tick_bad()
          else:
              tick_good()
        else:
          failed_tests.append( (s, "No test file") )
          tick_bad()

test_directory(unittest_dir)
print("")
if (len(failed_tests) > 0):
  print("Failed tests: " + str(len(failed_tests)) + " of " + str(total_test_count))
  for i in failed_tests:
    print(i)
else:
  print("All tests (" + str(total_test_count) + " of " + str(total_test_count) + ") pass")

#output = Popen(["ls", "-l"], stdout=PIPE).communicate()[0]
#print output
