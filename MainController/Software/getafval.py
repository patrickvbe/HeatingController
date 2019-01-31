from drawitem import DrawItem
import colors
import time
import tinytime
try:
  import urequests as requests
except:
  import requests

def GetFromAvri():
  response = requests.get("http://dataservice.deafvalapp.nl/dataservice/DataServiceServlet?type=ANDROID&app=avri&avri=true&service=OPHAALSCHEMA&land=NL&postcode=5315AE&huisnr=36")
  lines = response.text.split()
  response.close()
  response = None
  return lines

def GetAfval():
  lines = GetFromAvri()

  # Zoek de eerstkomende dagen voor de verschillende afvalsoorten
  dagen = {}
  today = tinytime.trunctime(time.time())
  nextyear = today + tinytime.DURATION_YEAR
  for line in lines:
    elements = line.split(';')
    afvaltype = None
    for element in elements:
      if len(element) >= 2:
        if element[0].isdigit():
          newdate = tinytime.dmytoseconds(element)
          if newdate >= today and newdate < dagen[afvaltype]:
            dagen[afvaltype] = newdate
        else:
          afvaltype = element
          if not afvaltype in dagen:
            dagen[afvaltype] = nextyear

  # Sorteer op datum en maak een mooi leesbare tekst me een passend kleurtje.
  drawitems = []
  tomorrow = today + tinytime.DURATION_DAY
  volgendeweek = today + tinytime.DURATION_WEEK
  kleuren = {"PLASTIC" : colors.ORANGE, "PAPIER" : colors.BLUE, "GFT" : colors.GREEN, "REST" : colors.GREY}
  weekdagen = ["Maandag", "Dinsdag", "Woensdag", "Donderdag", "Vrijdag", "Zaterdag", "Zondag"]
  for afvaltype, dag in sorted(dagen.items(), key=lambda elem: elem[1]):
    if dag < nextyear:
      drawitem = DrawItem()
      kleur = kleuren.get(afvaltype, colors.FG)
      if dag == today:
        drawitem.tekst = "Vandaag"
      elif dag == tomorrow:
        drawitem.tekst = "Morgen"
      elif dag < volgendeweek:
        tm = time.localtime(dag)
        drawitem.tekst = weekdagen[tm[6]]
      else:
        drawitem.tekst = tinytime.secondstodmy(dag)
      drawitem.tekst = drawitem.tekst + ": " + afvaltype
      if dag <= tomorrow:
        drawitem.bgcolor = kleur
        drawitem.fgcolor = colors.BG
      else:
        drawitem.fgcolor = kleur
      drawitems.append(drawitem)
  return drawitems
