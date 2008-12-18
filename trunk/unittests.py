import sys
import os
from subprocess import *

unittest_dir = "unittests"
binary_dir = "tmp"
out_bin = os.path.join(binary_dir, "unitout")
compiler_dir = "bin"
compiler_bin = os.path.join(compiler_dir, "minnowc")

total_test_count = 0
failed_tests = []

def test_directory(d):
  global total_test_count

  print("Testing: " + d)
  unittests = os.listdir(d)
  for f in unittests:
    if os.path.isdir(os.path.join(d, f)):
      test_directory(os.path.join(d, f))
    elif f[-3:] == "tst":
      pass #print("Test found: " + f)
    else:
      sys.stdout.write(".")
      sys.stdout.flush()
      total_test_count += 1
      s = os.path.join(d, f)
      tst = s[:-3] + "tst"
      output = Popen([compiler_bin, s,  "-o", out_bin], stdout=PIPE, stderr=PIPE).communicate()
      if (len(output[1]) > 0):
        failed_tests.append( (s, output[1]) )
      else:
        if (os.path.isfile(tst)):
          tst_file = open(tst, 'r')

          tst_cmp = tst_file.read()
          bin_output = Popen([out_bin], stdout=PIPE, stderr=PIPE).communicate()
          if (len(bin_output[1]) > 0):
            failed_tests.append( (s, bin_output[1]) )
          elif (tst_cmp != bin_output[0]):
              failed_tests.append( (s, bin_output[0]) )
        else:
          failed_tests.append( (s, "No test file") )

test_directory(unittest_dir)
print("")
if (len(failed_tests) > 0):
  print("Failed tests: " + str(len(failed_tests)) + " of " + str(total_test_count))
  for i in failed_tests:
    print i
else:
  print("All tests pass")

#output = Popen(["ls", "-l"], stdout=PIPE).communicate()[0]
#print output
