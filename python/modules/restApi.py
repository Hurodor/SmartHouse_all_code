from flask import request
from flask_restful import Resource, reqparse

# custom modules
from modules.database import *


class Booking(Resource):
    """booking api"""
    # bathroom = 7, kitchen = 8, livingroom = 9
    __roomRules = {7: 1, 8: 3, 9: 5}

    @classmethod
    def get(cls):
        """returns bookings that are not avalible"""
        bookingParser = reqparse.RequestParser()
        bookingParser.add_argument('roomID', type=int)  # add roomID as a url parameter
        bookingParser.add_argument('time', type=str)  # add time as url parameter
        bookingParser.add_argument('date', type=int)  # add day as a url parameter

        args = bookingParser.parse_args()  # get url paramters
        # python: args {'roomID': x, 'time': xx:xx:xx', 'date': 0}
        # ESP:  booking?roomID=7&time=17:00:00&date=0

        bookingTable = BookingTable()
        sendBack = bookingTable.getBooking(args)

        return sendBack
        # return args  # this should return next 4 bookings from desired timestamp

    def post(self):
        """take a bookingRequest and check database returns bookings that crash with requested booking"""
        body = request.get_json()

        # change body['date'] to current date + how many days in the future
        body['date'] = str(datetime.date.today() + datetime.timedelta(days=int(body['date'])))

        # check database
        bookingTable = BookingTable()
        t = bookingTable.checkBookingNew(body)

        # roomRules specified as private global variables
        roomID = int(body['roomID'])
        if len(t) >= self.__roomRules[roomID]:
            return "there is no space left for you at your desired time", 406

        bookingTable.addBooking(body)
        returnString = f"{body['bookFrom'][:-3]} to {body['bookTo'][:-3]}"

        return returnString, 201


class Entry(Resource):
    """valid data is always last row of databaseTable: entry"""

    allowedPeople = 11

    @classmethod
    def get(cls):

        # initialise EntryTable class
        entryTable = EntryTable()

        # fetch status from database
        statusNow = entryTable.getStatus()

        return statusNow, 200  # 200 means ok request

    @classmethod
    def post(cls):
        """change door state in db"""

        # {'doorOpen': 'x', 'doorLocked': 'x', 'peopleAllowed': 'x'}
        body = request.get_json()

        # connect to databaseTable entry
        entryTable = EntryTable()

        # set people count as last value if no input
        if "people" not in body.keys():
            body['people'] = entryTable.getLastPeople()

        # this sets Allowed people in house
        body['peopleAllowed'] = cls.allowedPeople

        entryTable.insertDict(body)

        return "", 201

    def put(self):
        """for remote accses"""
        body = request.get_json()

        print(body)

        # connect to databaseTable entry
        entryTable = EntryTable()

        # set people count as last value if no input
        if "people" not in body.keys():
            body['people'] = entryTable.getLastPeople()

        # this sets Allowed people in house
        body['peopleAllowed'] = self.allowedPeople

        entryTable.insertDict(body)


# should work
class RoomControl(Resource):

    # choose limit on max people for each room
    maxPeopleAllowed = {1: '02',
                        2: '02',
                        3: '02',
                        4: '02',
                        5: '02',
                        6: '02',
                        7: '01',
                        8: '03',
                        9: '05', }

    def get(self):
        """this will fetch status for roomID specified"""

        # adding url parameter
        roomControlParser = reqparse.RequestParser()
        roomControlParser.add_argument('roomID', type=int, required=True,
                                       help="need to specify roomID")
        roomID = roomControlParser.parse_args()['roomID']  # url parameter

        # get signal from database
        roomControlTable = RoomControlTable()
        # sendBackString = roomControlTable.getStatus(roomID) + self.maxPeopleAllowed

        # todo: mark out for changing people allowed
        sendBackString = roomControlTable.getStatus(roomID) + self.maxPeopleAllowed[roomID]
        print(sendBackString)

        return sendBackString, 200

    def post(self):
        """insert when change in people from esp"""

        # body = {'people': x, 'roomID': x}
        body = request.get_json()

        # todo: set roomID as 1 if Gleen forget to send info
        if 'roomID' not in body.keys():
            body['roomID'] = 1
            print("not in keys")

        roomControlTable = RoomControlTable()

        # updated methode
        roomControlTable.updateRoom(body)

        return body, 201

    def put(self):
        """this comes from remote console esp, updates """
        # {"lightVal": 000, "tempVal": 00, "windowOpen": 0, "fanSpeed": 000, "roomID"}
        body = request.get_json()
        print(body)

        roomControlTable = RoomControlTable()

        roomControlTable.updateRoom(body)

        return body, 201
