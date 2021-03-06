from ast import literal_eval
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
  mainExe = sys.argv[1]
  prezFile = sys.argv[2]

  proc = subprocess.run((mainExe, prezFile, "file.asm"), stderr=subprocess.PIPE)
  if proc.returncode < -1:
    print(f"Exited with error code {-proc.returncode}")
    exit(0)

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
  for (msgType, line, firstWord) in [literal_eval(sys.argv[i]) for i in range(3, len(sys.argv))]:
    expected.append(msgString(msgType, line, firstWord))

  if len(errOutput) != len(expected) or not runTest(expected, errOutput):
    print("Failure...")
    print("Expected:")
    pprint(expected)
    print()
    print("Got:")
    pprint(errOutput)
    exit(1)
  else:
    print("Pass!")
    exit(0)
