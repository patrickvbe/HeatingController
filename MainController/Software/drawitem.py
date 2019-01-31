
import colors

class DrawItem:
  tekst = ""
  bgcolor = colors.BG
  fgcolor = colors.FG

  def __repr__(self):
    return "{{tekst=\"{}\", bgcolor=0x{:04X}, fgcolor=0x{:04X}}}".format(self.tekst, self.bgcolor, self.fgcolor)