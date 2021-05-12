# smartHouse

packages: 
pip3 install mariadb flask flask_restful requests pytz python-dateutil schedule

please run the app in a virtual envirement

for the app to work you need to have a local database, 
to create the needed tables please copy all the code in conifg/db_setup.txt
and run it inside your database consol.

to sart server please make sure you are in smartHouse folder and run following commands in termnal:
# Linux and mac:

export FLASK_RUN_HOST=0.0.0.0 

flask run

# Windows:
set FLASK_RUN_HOST=0.0.0.0

flask run





