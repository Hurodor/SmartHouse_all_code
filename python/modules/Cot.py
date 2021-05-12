#
# klass for communication with CoT
#

import requests
import json



class Signal:
    testToken = "eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI1MTk1In0.t7suGraBvinvSMNzoAiT2ufFl_a99HXEZXd3UyLHqCM"
    testKey = '31553'

    smartHouseToken = 'eyJhbGciOiJIUzI1NiJ9.eyJqdGkiOiI0ODg3In0.Tpcl8yMLKCTpcQtz5U-4Eny2OuspzWQHKk_cTKTxgZI'

    def __init__(self, key, token, info="NA"):
        self.token = token
        self.key = key
        self.info = info

    def write(self, value):
        """Writes a value to signal"""
        data = {'Key': self.key, 'Value': value, 'Token': self.token}

        response = requests.put("https://circusofthings.com/WriteValue",
                                data=json.dumps(data),
                                headers={"Content-Type": 'application/json'})

        if response.status_code != 200:
            print('error {}'.format(response.status_code))



    def read(self, decimal=True):
        """Reads the value of a signal"""
        data = {'Key': self.key, 'Token': self.token}

        response = requests.get("https://circusofthings.com/ReadValue",
                                params=data,
                                headers={"Content-Type": 'application/json'})

        jsonResponse = json.loads(response.content)

        print("Reading Value form signal {}".format(self.key))
        if response.status_code != 200:
            # print("Success")
            print('error {}'.format(response.status_code))


        if decimal:
            return jsonResponse['Value']
        return int(jsonResponse['Value'])




