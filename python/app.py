#   @copyright -   Vegar & Co
#
#   this is the main rest_api script
#   running python 3.8.5
#
#   _______________________________________


from flask import Flask
from flask_restful import Api

# custom modules
from modules.restApi import *


app = Flask(__name__)
api = Api(app)



# adds functions in first argument in endpoint that is defined as str in second argument
api.add_resource(Booking, "/booking", endpoint="booking")
api.add_resource(Entry, "/entry", endpoint="entry")
api.add_resource(RoomControl, "/roomcontrol", endpoint="booking")


# host 0.0.0.0 means the server is open, and not locally on computer
if __name__ == '__main__':
    app.run(host='0.0.0.0')

