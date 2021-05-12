#
#   showcase on CoT
#
# --version 3.8.5

from modules.Cot import Signal
from modules.database import PowerPriceTable, PowerSupplyTable, PowerUsageTable, PowerTotalTable
from time import sleep



def production(prod_read):
    powerSupplyTable = PowerSupplyTable()


    if prod_read == 0:  # Now
        try:
            sendBack = powerSupplyTable.getProductionNow()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0

    elif prod_read == 1:  # Today
        try:
            sendBack = powerSupplyTable.getProductionToday()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0

    elif prod_read == 2:  # Last Week
        try:
            sendBack = powerSupplyTable.getProductionLastWeak()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0

    elif prod_read == 3:  # Last Month
        try:
            sendBack = powerSupplyTable.getProductionLastMonth()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0

    elif prod_read == 4:  # This Year
        try:
            sendBack = powerSupplyTable.getProductionThisYear()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    else:
        sendBack = 0

    return sendBack


def consumption(con_read):

    powerUsageTable = PowerUsageTable()

    if con_read == 0:  # Now
        try:
            sendBack = powerUsageTable.getUsageNow()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    elif con_read == 1:  # Today
        try:
            sendBack = powerUsageTable.getUsageLastWeak()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    elif con_read == 2:  # Last Week
        try:
            sendBack = powerUsageTable.getUsageLastWeak()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    elif con_read == 3:  # Last Month
        try:
            sendBack = powerUsageTable.getUsageLastMonth()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    elif con_read == 4:  # This Year
        try:
            sendBack = powerUsageTable.getUsageThisYear()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    else:
        sendBack = 0

    return sendBack


def net(net_read):
    powerUsageTable = PowerUsageTable()
    powerSupplyTable = PowerSupplyTable()
    if net_read == 0:  # now
        try:
            sendBack = powerSupplyTable.getProductionNow() - powerUsageTable.getUsageNow()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    elif net_read == 1:  # today
        try:
            sendBack = powerSupplyTable.getProductionToday() - powerUsageTable.getUsageToday()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    elif net_read == 2:  # last weak
        try:
            sendBack = powerSupplyTable.getProductionLastWeak() - powerUsageTable.getUsageLastWeak()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    elif net_read == 3:  # last mothn
        try:
            sendBack = powerSupplyTable.getProductionLastMonth() - powerUsageTable.getUsageLastMonth()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    elif net_read == 4:  # this year
        try:
            sendBack = powerSupplyTable.getProductionThisYear() - powerUsageTable.getUsageThisYear()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    else:
        sendBack = 0

    return sendBack


def price(price_read):
    priceTable = PowerPriceTable()

    if price_read == 0:  # Now
        try:
            sendBack = priceTable.getPriceNow()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    elif price_read == 1:  # Today
        try:
            sendBack = priceTable.getPriceToday()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    elif price_read == 2:  # Average Last Week
        try:
            sendBack = priceTable.getLastWeak()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    elif price_read == 3:  # Average Last Month
        try:
            sendBack = priceTable.getLastMonth()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    elif price_read == 4:  # Average This Year
        try:
            sendBack = priceTable.getThisYear()
        except Exception as e:
            print(f"Error {e} set CoT signal to 0")
            return 0
    else:
        sendBack = 0

    return sendBack


# Production signal
button_prod = Signal("20914", Signal.smartHouseToken)
showcaseProd = Signal("24696", Signal.smartHouseToken)
lastSignalRead_prod = button_prod.read(False)
print(f"initialising production button")
sleep(2)

# Consumption signal
button_con = Signal("15419", Signal.smartHouseToken)
showcaseCon = Signal("27795", Signal.smartHouseToken)
lastSignalRead_con = button_con.read(False)
sleep(2)
print(f"initialising consumption button ")
# Price  signal
button_price = Signal("25770", Signal.smartHouseToken)
showcasePrice = Signal("27421", Signal.smartHouseToken)
lastSignalRead_price = button_price.read(False)
sleep(2)
print(f"initialising button: ")
# Net signal
button_net = Signal("31009", Signal.smartHouseToken)
showcaseNet = Signal("23130", Signal.smartHouseToken)
lastSignalRead_net = button_net.read(False)
sleep(2)
print(f"initialising net button ")


print(f"all sett up: \n")

# Read and Write button modules
while True:
    buttonSignal_prod = button_prod.read(False)
    if buttonSignal_prod != lastSignalRead_prod:
        valueProd = production(buttonSignal_prod)
        showcaseProd.write(valueProd)
    sleep(1)
    lastSignalRead_prod = buttonSignal_prod

    buttonSignal_con = button_con.read(False)
    if buttonSignal_con != lastSignalRead_con:
        valueCon = consumption(buttonSignal_con)
        showcaseCon.write(valueCon)
    sleep(1)
    lastSignalRead_con = buttonSignal_con

    buttonSignal_net = button_net.read(False)
    if buttonSignal_net != lastSignalRead_net:

        valueNet = net(buttonSignal_net)
        showcaseNet.write(valueNet)
    sleep(1)
    lastSignalRead_net = buttonSignal_net

    buttonSignal_price = button_price.read(False)
    if buttonSignal_price != lastSignalRead_price:

        valuePrice = price(buttonSignal_price)
        showcasePrice.write(valuePrice)
    sleep(5)
    lastSignalRead_price = buttonSignal_price



