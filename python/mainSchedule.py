#
# this script ueses schedule to organize the jobs to run
#
#__________________________________________________________



# python modules
import schedule
import time

# custom modules
from modules.database import PowerUsageTable, PowerTotalTable
from modules import externalData
from modules.Cot import Signal

# defining signal to use in CoT
billSignal = Signal("659", Signal.smartHouseToken)


def updateBill():
    powerTotalTable = PowerTotalTable()
    bill = powerTotalTable.getTotal(0) * 1.25
    billSignal.write(bill)


def generateAndAddPowerUsage():
    powerUsageTable = PowerUsageTable()
    power = powerUsageTable.generatePowerUsage()
    powerUsageTable.insertDict(power)


# fetch external data
# add solar panel production
schedule.every().day.at("00:16").do(externalData.getPowerProduction)

# add powerPrice
schedule.every().day.at("00:16").do(externalData.getPrice)

# add weather info
schedule.every().day.at("00:16").do(externalData.getWeather)

# update weather on CoT
schedule.every(15).minutes.do(externalData.weather_now)

# generate and add powerUsage
schedule.every().hour.do(generateAndAddPowerUsage)


# update totalTable to show the bill
schedule.every().monday.at("00:01").do(PowerTotalTable().addTotalPower)

schedule.every().monday.at("00:30").do(updateBill)



while True:
    schedule.run_pending()
    time.sleep(1)



