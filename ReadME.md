alias get_idf='. $HOME/esp/esp-idf/export.sh'

get_idf

cd ~/esp
cp -r $HOME/Documents/VL53L5CX_Sensor .

cd ~/esp/VL53L5CX_Sensor
idf.py set-target esp32
idf.py menuconfig


rm -rf ~/esp/VL53L5CX_Sensor
cd $HOME/Documents/VL53L5CX_Sensor


Run idf.py menuconfig. Go to Component Config -> ESP System settings and increase the Main task stack size to at least 7168.


idf.py build


cd ~/esp
rm -rf ~/esp/VL53L5CX_Sensor
cp -r $HOME/Documents/VL53L5CX_Sensor .
cd ~/esp/VL53L5CX_Sensor
idf.py set-target esp32
idf.py menuconfig
