import serial
import struct

ser = serial.Serial(port='COM5', baudrate=115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE,timeout = 2)
try:
    ser.isOpen()
    print("Porta Aberta")
except:
    print("error")
    exit()

if(ser.isOpen()):
     try:
         while(1):
             data=ser.read(2)
             data1=struct.unpack('!BB', data)
             
             uper=data1[0] << 8
             lower=data1[1]
             dado_pronto= uper | lower
             file=open('dados.txt', 'a+')
             file.write("%i\n" % dado_pronto)
             
             print(dado_pronto)
             
             
             
     except Exception:
         print("error")
else:
    print("nao foi possivel conectar")

    

