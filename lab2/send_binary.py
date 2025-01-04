import argparse
import serial
from pwn import *

# https://docs.python.org/zh-tw/3/howto/argparse.html
parser = argparse.ArgumentParser()
parser.add_argument("-f", "--filename", help="The file to be sent.")
parser.add_argument("-d", "--device"  , help="Device to send data on.")
parser.add_argument("-b", "--baudrate", help="Baudrate of sending data.")

def show_arguments(parser):
    print(f"Send {parser.filename} on {parser.device}")

show_arguments(parser)

# https://pyserial.readthedocs.io/en/latest/pyserial_api.html
# https://pyserial.readthedocs.io/en/latest/shortintro.html
with serial.Serial(parser.device, baudrate=parser.baudrate) as tty:
    with open(parser.filename, "rb") as f:
        kernel_image = f.read()
        kernel_image_len = len(kernel_image)
        print("Kernel image size: 0x", hex(kernel_image_len))
    
    # send kernel size
    for i in range(4):
        send_bytes = p32(kernel_image_len)[i:i+1]
        print(send_bytes)
        tty.write(send_bytes)
        tty.flush()
    
    # send kernel
    print("Start sending kernel image by uart ...")
    for i in range(kernel_image_len):
        tty.write(kernel_image[i : i+1])
        tty.flush()
    
    print("Sending completed!")