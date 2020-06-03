from ast import literal_eval as make_tuple
import subprocess
import sys
import re
import os
from pprint import pprint

def msgString(msgType : str, line : str, firstWord : str) -> str:
  return msgType + " on line " + line + ": " + firstWord


def runTest(expected : list, actual : list) -> bool:
  for i in range(len(actual)):
    if not errOutput[i].startswith(expected[i]):
      return False

  return True


if __name__ == "__main__":
  os.chdir(os.getcwd() + "/error")

  mainExe = sys.argv[1]
  prezFile = sys.argv[2]
  asmFile = sys.argv[3]

  proc = subprocess.run((mainExe, prezFile, asmFile), stderr=subprocess.PIPE)
  errOutput = proc.stderr.decode()

  # https://stackoverflow.com/questions/14693701/how-can-i-remove-the-ansi-escape-sequences-from-a-string-in-python
  ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
  errOutput = ansi_escape.sub('', errOutput).split('\n')
  errOutput = list(filter(
      lambda line:
          line.startswith("Error")
          or line.startswith("Warning")
          or line.startswith("Note"),
      errOutput))

  expected = []
  for check in sys.argv[4].split():
    (msgType, line, firstWord) = make_tuple(check)
    expected.append(msgString(msgType, line, firstWord))

  # Errors don't have to appear in exact order
  errOutput.sort()
  expected.sort()

  if len(errOutput) != len(expected) or not runTest(expected, errOutput):
    print("Failure...")
    print("Expected:")
    pprint(expected)
    print()
    print("Got:")
    pprint(errOutput)
  else:
    print("Pass!")
