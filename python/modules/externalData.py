
# python modules
import datetime
import requests
import json
import pytz, dateutil.parser

# custom modules
from modules.database import PowerSupplyTable, PowerPriceTable, WeatherTable
from modules.Cot import Signal


# used for fetcing powerPrice
day = str(datetime.date.today() + datetime.timedelta(days=0))

class ExternalData:

    """APi parameters defined as private variables"""
    __powerPriceParameters = {
        'url': 'https://norway-power.ffail.win/',
        'payload': {'zone': 'NO3', 'date': day}
    }

    __solarPowerGainParameters = {
        'url': 'https://api.solcast.com.au//world_pv_power/forecasts',
        'payload': {'api_key': 'IPNi2QK0q-5z-3CowTkQ-m55HhWoAbEj',
                    'latitude': '63.418901',
                    'longitude': '10.40266',
                    'capacity': "32.3",
                    'tilt': '20',
                    'azimuth': '180',
                    'hours': '24',  # todo: this has to be according to time interval between requests
                    'format': 'json'
                    }
    }

    __weatherParameters = {
        'url': 'http://pro.openweathermap.org/data/2.5/forecast/hourly',
        'payload': {
            'id': '3133881',
            'lang': 'no',
            'cnt': 24,
            'appid': 'c26f85065fd4fd77bf8aef8c4f98e656',
            'units': 'metric'
        }
    }

    __weatherParameters_now = {
        'url': 'http://api.openweathermap.org/data/2.5/weather',
        'payload': {
            'id': '3133880',
            'lang': 'no',
            'appid': 'c26f85065fd4fd77bf8aef8c4f98e656',
            'units': 'metric'
        }
    }

    # specify witch api to initialise
    def __init__(self, typeOfData):
        """typeOfData: powerPrice, solarPower, or weather :type str"""
        self.content = None

        if typeOfData == 'powerPrice':
            self.parameters = self.__powerPriceParameters

        elif typeOfData == 'solarPower':
            self.parameters = self.__solarPowerGainParameters

        elif typeOfData == 'weather':
            self.parameters = self.__weatherParameters

        elif typeOfData == 'weather_now':
            self.parameters = self.__weatherParameters_now
        else:
            raise NameError("parameter: typeOfData must be powerPrice, solarPower, or weather :type str")

    def getData(self):
        """fetch data from api"""
        url = self.parameters['url']
        payload = self.parameters['payload']

        response = requests.get(url, payload)
        if response.status_code != 200:
            print(f"Status code: {response.status_code}")

        self.content = json.loads(response.content)
        return self.content


def isoToLocal(timeStr):
    """ used to get right tim ein powerProduction"""
    utcTime = dateutil.parser.parse(timeStr)
    localTime = utcTime.astimezone(pytz.timezone("Europe/Oslo"))
    return str(localTime)[:-6]


def getPowerProduction():
    # connect to database: table-solarPowerGain
    solarPowerGain = ExternalData('solarPower')
    solarPowerGain.getData()

    databasePackage = []
    for i in solarPowerGain.content['forecasts']:
        period_end = isoToLocal(i['period_end'])

        # cant insert 0 value to database
        if i['pv_estimate'] == 0:
            pv_estimate = None
        else:
            pv_estimate = i['pv_estimate']

        period = i['period']
        databasePackage.append([pv_estimate, period_end])

    powerSupplyTable = PowerSupplyTable()
    powerSupplyTable.addPowerSupply(databasePackage)


def getPrice():
    # get data from Api
    powerPrice = ExternalData('powerPrice')
    content = powerPrice.getData()

    # make ready for depart to database
    databasePackage = []
    for values in content:
        price = content[values]['NOK_per_kWh']
        validFrom = content[values]['valid_from'][11:19]
        validTo = content[values]['valid_to'][11:19]
        date = values[:10]

        # will make a list of list
        databasePackage.append([price, validFrom, validTo, date])


    # make connection to database
    powerPriceTable = PowerPriceTable()
    # add content to database
    powerPriceTable.addPowerPrice(databasePackage)


def getWeather():
    weather = ExternalData("weather")
    weather.getData()
    # make data ready to insert to database
    databasePackage = []
    for value in weather.content['list']:
        timestamp = datetime.datetime.fromtimestamp(value['dt'])
        temp = value['main']['temp']
        pressure = value['main']['pressure']
        humidity = value['main']['humidity']
        description = value['weather'][0]['description']
        cloudiness = value['clouds']['all']
        windspeed = float(value['wind']['speed'])

        databasePackage.append([timestamp,
                                temp,
                                pressure,
                                humidity,
                                description,
                                cloudiness,
                                windspeed])


    # # insert into database
    weather = WeatherTable()
    weather.addWeatherContent(databasePackage)


def weather_now():
    weather_Ole = ExternalData('weather_now')
    data = weather_Ole.getData()

    city = data['name']
    description = data['weather'][0]['description']
    temp = data['main']['temp']
    wind_speed = data['wind']['speed']
    pressure = data['main']['pressure']
    humidity = data['main']['humidity']
    cloudiness = data['clouds']['all']

    UNIXsunrise = data['sys']['sunrise']
    UNIXsunset = data['sys']['sunset']
    sunrise = datetime.datetime.fromtimestamp(UNIXsunrise)
    sunset = datetime.datetime.fromtimestamp(UNIXsunset)

    tempSignal = Signal("25168", Signal.smartHouseToken)
    tempSignal.write(temp)

    wind_speedSignal = Signal("24618", Signal.smartHouseToken)
    wind_speedSignal.write(wind_speed)

    pressureSignal = Signal("8771", Signal.smartHouseToken)
    pressureSignal.write(pressure)

    humiditySignal = Signal("144", Signal.smartHouseToken)
    humiditySignal.write(humidity)

    cloudinessSignal = Signal("5086", Signal.smartHouseToken)
    cloudinessSignal.write(cloudiness)

    sunriseSignal = Signal("26836", Signal.smartHouseToken)
    sunriseSignal.write(UNIXsunrise)

    sunsetSignal = Signal("2796", Signal.smartHouseToken)
    sunsetSignal.write(UNIXsunset)


# this will not run if the file is imported
if __name__ == '__main__':
    getPowerProduction()
    getPrice()
    getWeather()
    weather_now()







