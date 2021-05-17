Each folder contains the specific codes used for each circuit.
The request-class is used in Room, Entry and Console, however
to avoid duplicated files, it's saved in the ESP32_Arduino folder
instead of inside each specific folder where it is used. The setup
in this project will therefore not work, as the include path for
"Request.h" is not valid. In the project, when uploading code, all
files have been within the same folder.
