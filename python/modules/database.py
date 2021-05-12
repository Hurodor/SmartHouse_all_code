# --Author--: Vegar                          |
#                                            |
# This module is used to connect to database |
# Database class is general and can be used  |
# in all cases.                              |
# table classes defined under is specific to |
# smartHouse project                         |
# ___________________________________________|


import mariadb
import datetime

class Database:
    """
    change __config for loginInformation
    needs to connect() and close() when using to not keep the connection open
    can also use with connect() to automatically close after with statement
    """
    __config = {'user': 'hurodor', 'password': '123456', 'host': '127.0.0.1'}

    database = 'smartTest'

    acceptableTables = ['booking',
                        'powerprice',
                        'powersupply',
                        'powertotal',
                        'powerusage',
                        'roomlog',
                        'userlog',
                        'users',
                        'weather',
                        'entry',
                        'roomcontrol']

    def __init__(self, database=database, table=None):
        self.connection = None
        self.cursor = None
        self.database = database
        self.content = []
        self.table = table

    def connect(self, autocommit=True):  # todo: use right config
        """create connection to database"""
        try:
            self.connection = mariadb.connect(**self.__config, database=self.database, autocommit=autocommit)
            self.cursor = self.connection.cursor()
            print(f"connected at database: {self.database}")
        except mariadb.Error as e:
            print(f"Error: {e}")
        return self.cursor

    def getQuery(self, command, *args):
        """execute select -query :returns list of each row as dict"""

        self.content = []  # in case of old content from previous use

        # making dict
        try:
            self.cursor.execute(command, *args)
            try:
                headerName = [x[0] for x in self.cursor.description]
                for value in self.cursor.fetchall():
                    self.content.append(dict(zip(headerName, value)))

                self.content = self.__onlyValidDataTypes(self.content)
                return self.content

            # expects error if no return
            except mariadb.ProgrammingError:
                print(mariadb.ProgrammingError)
            except TypeError:
                print(TypeError)

        except mariadb.Error as e:
            print(f"Error: {e}")
        return

    @staticmethod
    def __onlyValidDataTypes(content: list):
        """this is used to control which datatypes to get data from DB
            takes inn: list of dict
            :returns copy of input with only valid data types"""

        validDataTypes = [str]  # this chooses witch data-types that data from db comes in

        index = 0
        for i in content:
            for key, value in i.items():

                if type(value) not in validDataTypes:
                    content[index][key] = str(value)
            index += 1
        return content

    def insertQuery(self, command, content=None):
        """execute insert into -query, content :type list, dict :returns None
            dict can only handle one row of info, list can handle many rows of info
        """
        if type(content) == dict:
            insertStatements = []
            for key, value in content.items():
                insertStatements.append(value)
            insertStatements = [insertStatements]

        elif type(content) == list:
            if type(content[0]) == list:
                insertStatements = content
            else:
                insertStatements = [content]
        else:
            raise TypeError("content has to be of type: dict or list")

        try:
            self.cursor.executemany(command, insertStatements)
        except mariadb.Error as e:
            print(f"Error: {e}")

    def insertDict(self, content: dict):
        """insert dict to database"""
        if self.table not in self.acceptableTables:
            raise AttributeError(f"{self.table} not a acceptable table")

        keys = []
        values = []

        for key, value in content.items():
            keys.append(key)
            values.append(value)

        keyString = ""
        for key in keys:
            keyString += (key + ", ")
        keyString = keyString[:-2]

        valueCount = "?, " * len(keys)
        valueCount = valueCount[:-2]

        queryString = f"insert into {self.database}.{self.table} " \
                      f"({keyString}) values ({valueCount})"
        self.connect()
        self.insertQuery(queryString, values)
        self.close()

    def query(self, command, *args):
        """execute query :return mariadb -cursor"""
        try:
            self.cursor.execute(command, *args)
        except mariadb.Error as e:
            print(f"Error: {e}")
        return self.cursor

    def close(self, commit=True):
        """close connection to server"""
        if not self.connection.autocommit:
            if commit:
                self.connection.commit()
                self.connection.close()
                return
        self.connection.close()
        return

#                   tables function
# __________________________________________________________
#                          ↓

class BookingTable(Database):

    def __init__(self):
        super().__init__(table='booking')

    def checkBookingNew(self, bookRequest):
        """sends the booking-request to database and check for availability
            :return list of all bookings that crash with our booking
        """

        self.connect()
        print(bookRequest)
        self.getQuery("SELECT id, bookFrom, bookTo, roomID from smartTest.booking where "
                      "date = ? and roomID = ? and ? between bookFrom and bookTo "
                      "or date = ? and roomID = ? and ? between bookFrom and bookTo",
                      (bookRequest['date'], bookRequest['roomID'], bookRequest['bookFrom'],
                       bookRequest['date'], bookRequest['roomID'], bookRequest['bookTo']))

        self.close()
        return self.content

    def addBooking(self, booking):
        """bookingRequests has to be dict on format:
                {'bookFrom': HH:MM:SS, 'bookTo': HH:MM:SS, 'date': yyyy-mm-dd, 'userID': x, 'roomID': x}"""

        self.connect()
        self.insertQuery(
            "INSERT INTO smartTest.booking (bookFrom, bookTo, date, timestampOnBooking, userID, roomID) "
            "VALUES (?,?,?,CURRENT_TIMESTAMP,?,?)", [booking['bookFrom'], booking['bookTo'], booking['date'],
                                                     booking['userID'], booking['roomID']]
        )
        self.close()



    def getBooking(self, urlParameters: dict):
        """will give you next three bookings from desired time"""
        day = datetime.date.today() + datetime.timedelta(days=urlParameters['date'])
        self.connect()

        self.getQuery(
            "select bookFrom, bookTo from smartTest.booking where date = ? and roomID = ? and bookFrom > ? "
            "order by bookFrom limit 4",
            (day, urlParameters['roomID'], urlParameters['time'])
        )

        self.close()

        responseString = ""
        for rows in self.content:
            responseString += f"From {rows['bookFrom'][:-3]} to {rows['bookTo'][:-3]}|"

        return responseString[:-3]


class PowerPriceTable(Database):

    def __init__(self):
        super().__init__(table='powerprice')

    def addPowerPrice(self, content):
        self.connect()
        self.insertQuery(
            "INSERT INTO smartTest.powerprice (pricePerKwh, validFrom, validTo, date) VALUES (?,?,?,?)",
            content
        )
        self.close()

    def getPriceNow(self):
        """fetch current price from database"""

        # fetch current price
        self.connect()
        self.getQuery("select pricePerKwh as priceNow from smartTest.powerprice "
                      "where date = current_date "
                      "and validTo between subtime(current_time, '00:30:00') and addtime(current_time, '00:30:00')")

        self.close()
        try:
            return float(self.content[0]['priceNow'])
        except Exception as e:
            print(f"Error: {e} \n returning 0")
        return 0

    def getPriceToday(self):
        """fetch average price of today"""
        today = datetime.date.today()

        self.connect()
        self.getQuery("select avg(pricePerKwh) as priceToday from smartTest.powerprice where date = current_date")
        self.close()

        try:
            return float(self.content[0]['priceToday'])
        except Exception as e:
            print(f"Error: {e} \n returning 0")
        return 0

    def getLastWeak(self):
        # not tested
        self.connect()

        self.getQuery("SELECT avg(pricePerKwh) as priceLastWeak FROM smartTest.powerprice "
                      "WHERE WEEKOFYEAR(date)=WEEKOFYEAR(NOW())-1")

        self.close()

        try:
            return float(self.content[0]['priceLastWeak'])
        except Exception as e:
            print(f"Error: {e} \n returning 0")
        return 0

    def getLastMonth(self):
        self.connect()

        self.getQuery("select avg(pricePerKwh) as priceLastMonth from smartTest.powerprice "
                      "where year(date) = year(current_date - interval 1 month) "
                      "and month(date) = month(current_date - interval 1 month )")
        self.close()

        try:
            return float(self.content[0]['priceLastMonth'])
        except Exception as e:
            print(f"Error: {e} \n returning 0")
        return 0

    def getThisYear(self):
        return 0


class PowerSupplyTable(Database):

    def __init__(self):
        super().__init__(table='powersupply')

    def addPowerSupply(self, content):

        self.connect()
        self.insertQuery("insert into smartTest.powersupply (powerInn, timeStamp) values (?,?)",
                         content)
        self.close()

    def getProductionNow(self):
        """fetch expected powerSupply between +- 15 minutes from now"""
        self.connect()

        self.getQuery("select powerinn as productionNow from smartTest.powersupply "
                      "where timeStamp between subtime(current_timestamp, '00:15:00') "
                      "and addtime(current_timestamp, '00:15:00')")  
        self.close()

        try:
            return float(self.content[0]['productionNow'])
        except Exception as e:
            print(f"Error {e} \n returning 0")
        return 0

    def getProductionToday(self):
        """fetch production today until now"""
        self.connect()
        #
        self.getQuery("SELECT sum(powerinn) AS productionToday FROM smartTest.powersupply "
                      "WHERE DATE(timeStamp) = CURRENT_DATE AND TIME(timeStamp) < CURRENT_TIME")
        self.close()

        # Obs getQuery returns list of dict
        try:
            return float(self.content[0]['productionToday'])
        except Exception as e:
            print(f"Error {e} \n returning 0")
        return 0

        # "SELECT avg(pricePerKwh) as priceLastWeak FROM smartTest.powerprice "
        # "WHERE WEEKOFYEAR(date)=WEEKOFYEAR(NOW())-1"

    def getProductionLastWeak(self):
        """not tested"""
        self.connect()
        self.getQuery(
            "select sum(powerinn) as productionLastWeak from smartTest.powersupply "
            "where weekofyear(timeStamp) = weekofyear(now()) - 1"
        )
        self.close()

        try:
            return float(self.content[0]['productionLastWeak'])
        except Exception as e:
            print(f"Error {e} \n returning 0")
        return 0

    def getProductionLastMonth(self):
        """not tested"""
        self.connect()
        self.getQuery("select sum(powerinn) as productionLastMonth from smartTest.powersupply where "
                      "year(timestamp) = year(current_date - interval 1 month) "
                      "and month(timeStamp) = month(current_date - interval 1 month)")

        self.close()

        try:
            return float(self.content[0]['productionLastMonth'])
        except Exception as e:
            print(f"Error {e} \n returning 0")
        return 0

    def getProductionThisYear(self):
        # not tested
        self.connect()

        self.getQuery("select sum(powerinn) as productionThisYear from smartTest.powersupply where "
                      "year(timeStamp) = year(current_timestamp) and timeStamp < current_timestamp")
        self.close()

        try:
            return float(self.content[0]['productionThisYear'])
        except Exception as e:
            print(f"Error {e} \n returning 0")
        return 0


class PowerUsageTable(Database):
    # kWm
    constantPowerUsageKitchen = 0.03
    constantPowerUsageLivingRoom = 0.002
    constantPowerUsageBathRoom = 0.25
    constantPowerUsage = {'8': constantPowerUsageKitchen,
                          '9': constantPowerUsageLivingRoom,
                          '7': constantPowerUsageBathRoom}

    # kWh each element in list represent daily hour
    BasePower = [1.5, 1, 0.9, 1.2, 1.3, 1.1, 1.2, 3.1, 3.2, 2.9, 2.7, 3.1,
                 3, 2.9, 3.1, 3.2, 3, 3.2, 3.4, 3.1, 3, 2.9, 2.6, 1.9]

    def __init__(self):
        super().__init__(table='powerusage')

    # works
    def generatePowerUsage(self):
        """ this needs to run each hour"""
        now = datetime.datetime.now()
        now = now.strftime("%H")

        print(f"we are in hour {now}")
        self.connect()
        print(f"fetching bookings from db")
        self.getQuery("select minute(timediff(bookTo, bookFrom)) as minDiff, "
                      "hour(timediff(bookTo, bookFrom)) as hourDiff, "
                      "roomID from smartTest.booking "
                      "where date = current_date "
                      "and bookTO between current_time and addtime(current_time, '01:00:00')")
        self.close()

        content = self.content

        print(f"db content is {content}")

        thisHourPowerUsage = {'7': 0, '8': 0, '9': 0}
        peopleCount = {'7': 0, '8': 0, '9': 0}
        for rows in content:
            try:
                # finds interval in minutes
                interval = int(rows['minDiff']) + (int(rows['hourDiff']) * 60)
                print(f"interval = {interval}")

                thisHourPowerUsage[rows['roomID']] += self.constantPowerUsage[rows['roomID']] * interval
                peopleCount[rows['roomID']] += 1

                print(f"current powerUSage {thisHourPowerUsage[rows['roomID']]} \n "
                      f"added for room {rows['roomID']}")
                print(f"current peopleCount = {peopleCount}")

            except Exception as e:
                print(f"Error {e} skipping this calculation")
                continue

        thisHourPowerUsageTotal = 0
        if peopleCount['7']:
            thisHourPowerUsageTotal += thisHourPowerUsage['7'] / peopleCount['7']
        print(f"room 7 registerd current value ins {thisHourPowerUsageTotal}")

        if peopleCount['8']:
            thisHourPowerUsageTotal += thisHourPowerUsage['8'] / peopleCount['8']
        print(f"room 8 registerd current value ins {thisHourPowerUsageTotal}")

        if peopleCount['9']:
            thisHourPowerUsageTotal += thisHourPowerUsage['9'] / peopleCount['9']
        print(f"room 9 registerd current value ins {thisHourPowerUsageTotal}")

        thisHourPowerUsageTotal += self.BasePower[int(now)]
        print(f"calculating done! powerUsage is {thisHourPowerUsageTotal}")

        finalInsertDict = {'powerUse': thisHourPowerUsageTotal,
                           'room7': thisHourPowerUsage['7'],
                           'room8': thisHourPowerUsage['8'],
                           'room9': thisHourPowerUsage['9']}

        return finalInsertDict

    def getUsageNow(self):
        self.connect()
        self.getQuery(
            """
            select powerUse as powerUsageNow from smartTest.powerusage where date(timestamp) = current_date
            and time(timestamp) between subtime(current_time, '00:30:00') 
            and addtime(current_time, '00:30:00')
            """
        )
        self.close()

        try:
            return float(self.content[0]['powerUsageNow'])
        except Exception as e:
            print(f"Error {e} \n returning 0")
        return 0

    def getUsageToday(self):
        self.connect()

        self.getQuery(
            "select sum(powerUse) as powerUsageToday from smartTest.powerusage "
            "where date(timestamp) = current_date and time(timestamp) < current_time"
        )
        self.close()

        try:
            return float(self.content[0]['powerUsageToday'])
        except Exception as e:
            print(f"Error {e} \n returning 0")
        return 0

    def getUsageLastWeak(self):

        self.connect()
        self.getQuery(
            "select sum(powerUse) as powerUsageLastWeak from smartTest.powerusage "
            "where weekofyear(timeStamp) = weekofyear(now()) - 1"
        )

        self.close()
        if self.content[0]['powerUsageLastWeak'] == "None":
            return 0

        try:
            return float(self.content[0]['powerUsageLastWeak'])
        except Exception as e:
            print(f"Error {e} \n returning 0")
        return 0

    def getUsageLastMonth(self):
        self.connect()
        self.getQuery("select sum(powerUse) as powerUsageLastMonth from smartTest.powerusage where "
                      "year(timestamp) = year(current_date - interval 1 month) "
                      "and month(timeStamp) = month(current_date - interval 1 month)")

        self.close()

        try:
            return float(self.content[0]['powerUsageLastMonth'])
        except Exception as e:
            print(f"Error {e} \n returning 0")
        return 0

    def getUsageThisYear(self):

        self.connect()

        self.getQuery("select sum(powerUse) as powerUsageThisYear from smartTest.powerusage where "
                      "year(timeStamp) = year(current_timestamp) and timeStamp < current_timestamp")
        self.close()

        try:
            return float(self.content[0]['powerUsageThisYear'])
        except Exception as e:
            print(f"Error {e} \n returning 0")
        return 0


class PowerTotalTable(Database):

    def __init__(self):
        super().__init__(table='powertotal')

    def addTotalPower(self):
        powerUsage = PowerUsageTable()
        powerPrice = PowerPriceTable()
        powerSupply = PowerSupplyTable()

        usage = powerUsage.getUsageLastWeak()
        price = powerPrice.getLastWeak()
        supply = powerSupply.getProductionLastWeak()

        inserStatments = {'powerUsage': usage,
                          'powerSupply': supply,
                          'total': supply - usage,
                          'price': 0,
                          }

        inserStatments['price'] = price * inserStatments['total']

        self.connect()
        self.insertQuery(
            "insert into smartTest.powertotal (powerUsage, powerSupply, total, price, payDate) "
            "values (?, ?, ?, ?, week(current_date))",
            inserStatments
        )

    def getTotal(self, weeksInPast):
        self.connect()
        self.getQuery("select price from smartTest.powertotal where payDate = week(current_date) - ?",
                      [weeksInPast, ])
        try:
            bill = float(self.content[0]['price'])
            return bill
        except Exception as e:
            print(f"Error {e} \nno data, returning 0")
            return 0


class WeatherTable(Database):

    def __init__(self):
        super().__init__(table='weather')

    def addWeatherContent(self, content: list):
        """content is list of list"""
        self.connect()
        self.insertQuery("insert into smartTest.weather (timestamp, "
                         "temperature, "
                         "pressure, "
                         "humidity, "
                         "description, "
                         "cloudiness, "
                         "windspeed) values "
                         "(?,?,?,?,?,?,?)", content)
        self.close()


class EntryTable(Database):

    def __init__(self):
        super().__init__(table='entry')

    def getStatus(self):
        """fetch door status"""
        # info from ESP: doorOpen [x] doorLoced [x]  current peopleAllowed [xx]

        self.connect()

        self.getQuery("SELECT doorOpen, doorLocked, peopleAllowed FROM smartTest.entry ORDER BY ID DESC LIMIT 1")
        self.close()

        # fetch each value in list
        sendBackSignalList = []
        for key, value in self.content[0].items():
            sendBackSignalList.append(value)

        # if database returns "None" we need to change that to prevent error on datatypes:
        # todo: check format
        if sendBackSignalList[-1] == "None":
            sendBackSignalList[-1] = "10"

        # needs to be two siffer
        elif len(sendBackSignalList[-1]) != 2:
            sendBackSignalList[-1] = "0" + sendBackSignalList[-1]

        # joins to signalstr: "x x xx"
        sendBackSignal = "".join(sendBackSignalList)
        return sendBackSignal

    def getLastPeople(self):
        """fetch last input of how many people in apartment"""
        self.connect()
        self.getQuery("select people from smartTest.entry ORDER BY ID DESC LIMIT 1")
        self.close()

        # returning None value will cause Error
        lastPeople = self.content[0]['people']
        if lastPeople == "None":
            return 0

        return lastPeople


class RoomControlTable(Database):
    fullbodyMal = ['people', 'lightVal', 'tempVal', 'windowOpen', 'fanSpeed', 'roomID']

    def __init__(self):
        super().__init__(table="roomcontrol")

    def getStatus(self, roomID):
        """this fetches status for desired room"""
        self.connect()
        self.getQuery("select lightVal, tempVal, windowOpen, fanSpeed from smartTest.roomcontrol where roomID = ? "
                      "order by id desc limit 1", (roomID,))
        self.close()

        # if no return there is no remote changes done
        try:
            content = self.content[0]
        except IndexError:
            return "0000000000"

        # check format and configure it to be right
        if len(content['lightVal']) < 3:
            content['lightVal'] = "0" * (3 - len(content['lightVal'])) + content['lightVal']

        if len(content['tempVal']) < 2:
            content['tempVal'] = "0" + content['tempVal']

        if len(content['windowOpen']) < 2:
            content['windowOpen'] = "0" + content['windowOpen']

        if len(content['fanSpeed']) < 3:
            content['fanSpeed'] = "0" * (3 - len(content['fanSpeed'])) + content['fanSpeed']

        # making the signal to deliver to ESP
        sendBackList = []
        for key, value in content.items():
            sendBackList.append(value)

        sendBackStr = "".join(sendBackList)

        return sendBackStr

    def updateRoom(self, body):
        """ insert new information that is given"""
        self.connect()

        # get last value for that room
        self.getQuery("select people, roomID, lightVal, tempVal, windowOpen, fanSpeed from smartTest.roomcontrol "
                      "where roomID = ? order by id desc limit 1",
                      (body['roomID'],))

        # change old values with the new given keep old values where no new info given
        try:
            dbContent = self.content[0]
            for key, value in body.items():
                dbContent[key] = value
            print("ok")
            self.insertDict(dbContent)

        # if no return from database insert only zeros for that roomID
        except IndexError:
            self.query("insert into smartTest.roomcontrol "
                       "(people, lightVal, tempVal, windowOpen, fanSpeed, roomID) "
                       "values (0, 0, 0, 0, 0, ?)", (body['roomID'],))
            self.close()
            print("inserted zeroed value for that roomID for")
