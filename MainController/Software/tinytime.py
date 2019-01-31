import time

DURATION_DAY = 86400
DURATION_WEEK = DURATION_DAY * 7
DURATION_YEAR = DURATION_DAY * 365
TIMEZONE = 3600

def dmytoseconds(ymdstr):
  ''' Conversion from "dd-mm-yyyy" to seconds '''
  try:
    return time.mktime((int(ymdstr[6:10]),int(ymdstr[3:5]),int(ymdstr[0:2]),0,0,0,0,0,0))
  except ValueError:
    return 0

def ymdtoseconds(ymdstr):
  ''' Conversion from "yyyy-mm-dd" to seconds '''
  try:
    return time.mktime((int(ymdstr[0:4]),int(ymdstr[5:7]),int(ymdstr[8:10]),0,0,0,0,0,0))
  except ValueError:
    return 0

def secondstodmy(seconds):
  ''' Conversion from seconds to d-m-y h:mm'''
  tm = time.localtime(seconds)
  return str(tm[2]) + "-" + str(tm[1]) + "-" + str(tm[0])

def secondstohm(seconds):
  ''' Conversion from seconds to d-m-y h:mm'''
  tm = time.localtime(seconds)
  return str(tm[3]) + ":" + ("0" if tm[4] <= 9 else "") + str(tm[4])

def secondstodmyhm(seconds):
  ''' Conversion from seconds to d-m-y h:mm'''
  return secondstodmy(seconds) + " " + secondstohm(seconds)

def trunctime(seconds):
  return ((seconds + TIMEZONE) // DURATION_DAY) * DURATION_DAY - TIMEZONE
