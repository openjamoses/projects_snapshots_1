import json
import re
import shlex
from subprocess import Popen, PIPE


CQUERY_PATH = 'x64/Debug/cquery.exe'
CACHE_DIR = 'e2e_CACHE'

    # Content-Length: ...\r\n
    # \r\n
    # {
    #   "jsonrpc": "2.0",
    #   "id": 1,
    #   "method": "textDocument/didOpen",
    #   "params": {
    #     ...
    #   }
    # }

# We write test files in python. The test runner collects all python files in
# the directory and executes them. The test function just creates a test object
# which specifies expected stdin/stdout.
#
# Test functions are automatically discovered; they just need to be in the
# global environment and start with `Test_`.

class TestBuilder:
  def __init__(self):
    self.sent = []
    self.expected = []

  def IndexFile(self, path, contents):
    """
    Writes the file contents to disk so that the language server can access it.
    """
    self.Send({
      'method': '$cquery/indexFile',
      'params': {
        'path': path,
        'contents': contents,
        'args': [
          '-xc++',
          '-std=c++11',
          '-isystemC:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.10.25017/include',
          '-isystemC:/Program Files (x86)/Windows Kits/10/Include/10.0.15063.0/ucrt'
        ]
      }
    })
    return self

  def WaitForIdle(self):
    """
    Blocks the querydb thread until any active imports are complete.
    """
    self.Send({'method': '$cquery/queryDbWaitForIdleIndexer'})
    return self

  def Send(self, stdin):
    """
    Send the given message to the language server.
    """
    stdin['jsonrpc'] = '2.0'
    self.sent.append(stdin)
    return self

  def Expect(self, expected):
    """
    Expect a message from the language server.
    """
    expected['jsonrpc'] = '2.0'
    self.expected.append(expected)
    return self

  def SetupCommonInit(self):
    """
    Add initialize/initialized messages.
    """
    self.Send({
      'id': 0,
      'method': 'initialize',
      'params': {
        'processId': 123,
        'rootUri': 'cquery',
        'capabilities': {},
        'trace': 'off',
        'initializationOptions': {
          'cacheDirectory': CACHE_DIR,
          'clientVersion': -1 # Disables the check
        }
      }
    })
    self.Expect({
      'id': 0,
      'result': {
        'capabilities': {
          'textDocumentSync': 2,
          'hoverProvider': True,
          'completionProvider': {
            'resolveProvider': False,
            'triggerCharacters': [ '.', ':', '>', '#' ]
          },
          'signatureHelpProvider': {
            'triggerCharacters': [ '(', ',' ]
          },
          'definitionProvider': True,
          'referencesProvider': True,
          'documentHighlightProvider': True,
          'documentSymbolProvider': True,
          'workspaceSymbolProvider': True,
          'codeActionProvider': True,
          'codeLensProvider': {
            'resolveProvider': False
          },
          'documentFormattingProvider': False,
          'documentRangeFormattingProvider': False,
          'renameProvider': True,
          'documentLinkProvider': {
            'resolveProvider': False
          }
        }
      }
    })
    return self

def _ExecuteTest(name, func):
  """
  Executes a specific test.

  |func| must return a TestBuilder object.
  """
  test_builder = func()
  if not isinstance(test_builder, TestBuilder):
    raise Exception('%s does not return a TestBuilder instance' % name)

  # Add a final exit message.
  test_builder.Send({ 'method': '$cquery/exitWhenIdle' })

  # Convert messages to a stdin byte array.
  stdin = ''
  for message in test_builder.sent:
    payload = json.dumps(message)
    wrapped = 'Content-Length: %s\r\n\r\n%s' % (len(payload), payload)
    stdin += wrapped
  stdin_bytes = stdin.encode(encoding='UTF-8')

  # Finds all messages in |string| by parsing Content-Length headers.
  def GetMessages(string):
    messages = []
    for match in re.finditer('Content-Length: (\d+)\r\n\r\n', string):
      start = match.span()[1]
      length = int(match.groups()[0])
      message = string[start:start + length]
      messages.append(json.loads(message))
    return messages

  # Utility method to print a byte array.
  def PrintByteArray(bytes):
    for line in bytes.split(b'\r\n'):
      print(line.decode('utf8'))

  # Execute program.
  cmd = "%s --language-server" % CQUERY_PATH
  process = Popen(shlex.split(cmd), stdin=PIPE, stdout=PIPE, stderr=PIPE)
  (stdout, stderr) = process.communicate(stdin_bytes)
  exit_code = process.wait();

  # Check if test succeeded.
  actual = GetMessages(stdout.decode('utf8'))
  success = actual == test_builder.expected

  # Print failure messages.
  if success:
    print('== Passed %s with exit_code=%s ==' % (name, exit_code))
  else:
    print('== FAILED %s with exit_code=%s ==' % (name, exit_code))
    print('## STDIN:')
    for message in GetMessages(stdin):
      print(json.dumps(message, indent=True))
    if stdout:
      print('## STDOUT:')
      for message in GetMessages(stdout.decode('utf8')):
        print(json.dumps(message, indent=True))
    if stderr:
      print('## STDERR:')
      PrintByteArray(stderr)

    print('## Expected output')
    for message in test_builder.expected:
      print(message)
    print('## Actual output')
    for message in actual:
      print(message)
    print('## Difference')
    common_end = min(len(test_builder.expected), len(actual))
    for i in range(0, common_end):
      if test_builder.expected[i] != actual[i]:
        print('i=%s' % i)
        print('- Expected %s' % str(test_builder.expected[i]))
        print('- Actual %s' % str(actual[i]))
    for i in range(common_end, len(test_builder.expected)):
      print('Extra expected: %s' % str(test_builder.expected[i]))
    for i in range(common_end, len(actual)):
      print('Extra actual: %s' % str(actual[i]))


def _DiscoverTests():
  """
  Discover and return all tests.
  """
  for name, value in globals().items():
    if not callable(value):
      continue
    if not name.startswith('Test_'):
      continue
    yield (name, value)

def _RunTests():
  """
  Executes all tests.
  """
  for name, func in _DiscoverTests():
    _ExecuteTest(name, func)






#### EXAMPLE TESTS ####


class lsSymbolKind:
  Function = 1

def lsSymbolInfo(name, position, kind):
  return {
    'name': name,
    'position': position,
    'kind': kind
  }

def DISABLED_Test_Init():
  return (TestBuilder()
    .SetupCommonInit()
  )

def Test_Outline():
  return (TestBuilder()
    .SetupCommonInit()
    # .IndexFile("file:///C%3A/Users/jacob/Desktop/cquery/foo.cc",
    .IndexFile("foo.cc",
      """void foobar();""")
    .WaitForIdle()
    .Send({
      'id': 1,
      'method': 'textDocument/documentSymbol',
      'params': {
        'textDocument': {
          'uri': 'C:/Users/jacob/Desktop/cquery/foo.cc'
        }
      }
    })
    # .Expect({
    #   'jsonrpc': '2.0',
    #   'id': 1,
    #   'error': {'code': -32603, 'message': 'Unable to find file '}
    # }))
    .Expect({
      'id': 1,
      'result': [
        lsSymbolInfo('void main()', (1, 1), lsSymbolKind.Function)
      ]
    }))


if __name__ == '__main__':
  _RunTests()
