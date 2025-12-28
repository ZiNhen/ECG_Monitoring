import serial

ser = serial.Serial("COM7", 115200)
f = open("ecg.csv", "w")

    line = ser.readline().decode().strip()
    f.write(line + "\n")
