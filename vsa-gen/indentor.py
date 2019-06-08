class Indentor:
  def __init__(self, level):
    self.indent_level = 0
    self.tab_size = 2

  # def __init__(self, level):
  #   self.indent_level = level
  #   self.tab_size = 2

  def inc_by(self, num):
    self.indent_level += num

  def inc(self):
    self.indent_level += 1  
  
  def dec_by(self, num):
    self.indent_level -= num

  def dec(self):
    self.indent_level -= 1

  def to_str(self):
    ret_str = (' ' * self.tab_size) * self.indent_level
    return ret_str

if __name__ == "__main__":
  my_indentor = Indentor(0)
  my_indentor.inc()
  print('#' + my_indentor.to_str() + '#')
  my_indentor.inc_by(2)
  print('#' + my_indentor.to_str() + '#')