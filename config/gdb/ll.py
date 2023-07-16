# Extension for gdb, that adds command 'll'. This command prints 4 lines before
# and 16 lines after your current position in file
import gdb

class HelloWorld (gdb.Command):
  """Greet the whole world."""
  def __init__ (self):
      super (HelloWorld, self).__init__ ("ll", gdb.COMMAND_USER)

  def invoke (self, arg, from_tty):
      print ("Hello, World!")

print("Hola")
