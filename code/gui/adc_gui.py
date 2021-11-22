#!/usr/bin/python3

import tkinter as tk
import serial
from tkinter import * # note that module name has changed from Tkinter in Python 2 to tkinter in Python 3
from serial import Serial
import sys, traceback
import time
import threading
import queue



serial_start = False
initial_wait_complete = False
global ser
global voltage
voltage = 0

serial_read_queue = queue.Queue()


ser = Serial(
        port='COM15',\
        baudrate=115200,\
        parity=serial.PARITY_NONE,\
        stopbits=serial.STOPBITS_ONE,\
        bytesize=serial.EIGHTBITS)
#print("connected to: " + ser.portstr)
ser.close()

def serial_init():
    ser.open()
    


def serial_operation():
    global serial_start
    serial_start = True

def serial_ops():
    str = ''
    global ser, initial_wait_complete, serial_start
    ok_to_send_next_command = True
    while(1):
        time.sleep(0.010)
        if serial_start == True:
            global voltage
            if ok_to_send_next_command == True:
                ok_to_send_next_command = False
                ser.write(bytes("cat /sys/class/spiclient/spi0/read0\r\n",'ascii'))
            received_char = ''
            received_char = ser.read().decode("ascii")
            
            if(received_char == '\n'):
                if(str[0] in {'0','1','2','3','4','5','6','7','8','9'}):
                    print(str)
                    voltage = int(str) * 5.0 / 1024.0
                    time.sleep(0.200)
                serial_read_queue.put(str + '\n')
                str = ''
                ok_to_send_next_command = True
            else:
                if(received_char != ''):
                    str = str + received_char

            # read_line = str(received_line)
            # if(read_line[0].isdigit):
            #     #print(read_line[0])
            #     ##print(str(received_line))
            time.sleep(0.005)
            


def gui_update():
    global serial_start, value_label, voltage
    voltage_str = str(voltage)
    while(1):
        if serial_start == True:
            log_text_box.insert(tk.END, serial_read_queue.get())
            ##print(serial_read_queue.get())
            value_text_box.delete('1.0', END)
            value_text_box.insert(tk.END,voltage)
        time.sleep(0.01)
            
        
    



def display_gui():
    gui_height = 400
    gui_width = 600
    top_frame_height = 200
    bottom_frame_height = gui_height - top_frame_height

    root = tk.Tk()

    root.title("SPI over ADC")
    root.geometry(str(gui_width)+ "x" + str(gui_height))

    top_frame = Frame(root, bg='cyan', width=gui_width, height=top_frame_height, pady=3)
    btm_frame = Frame(root, bg='white', width=gui_width, height=bottom_frame_height, pady=3)
    settings_frame = Frame(top_frame, bg='yellow', width=(gui_width / 2), height=top_frame_height, pady=3)
    #graph_frame = Frame(top_frame, bg='green', width=(gui_width / 3), height=top_frame_height, pady=3)
    value_frame = Frame(top_frame, bg='green', width=(gui_width / 2), height=top_frame_height, pady=3)
    scrollbar = Scrollbar(btm_frame)
    


    top_frame.grid_propagate(False) 
    btm_frame.grid_propagate(False) 
    settings_frame.grid_propagate(False) 
    #graph_frame.grid_propagate(False) 
    value_frame.grid_propagate(False)


    top_frame.grid(row=0, sticky="ew")
    btm_frame.grid(row=1, sticky="ew")
    settings_frame.grid(row=0, column=0, sticky = "ew")
    #graph_frame.grid(row=0, column=1, sticky = "ew")
    value_frame.grid(row=0, column=1, sticky = "ew")
    scrollbar.grid(row=0, column=1)
    
    

    


    #Settings frame
    start_button  = tk.Button(settings_frame, 
                        #width=int(gui_width / 3),
                    text="START", 
                    fg="red",
                    command=serial_operation)
    start_button.grid(row=0, column=0)

    stop_button  = tk.Button(settings_frame, 
                        #width=int(gui_width / 3),
                    text="STOP", 
                    fg="red",
                    command=serial_operation)

    stop_button.grid(row=1, column=0)
    root.grid_rowconfigure(1, weight=1)
    root.grid_columnconfigure(0,weight=1)
    top_frame.grid(row=0, sticky="ew")
    btm_frame.grid(row=1, sticky="ew")


    #Log frame
    global log_text_box
    log_text_box = Text(btm_frame, height = bottom_frame_height, width=gui_width, yscrollcommand = scrollbar.set)
    log_text_box.grid(row=0,column=0)

    scrollbar.config( command = log_text_box.yview )


    #value frame
    global value_text_box
    value_label = Label(value_frame, text="Value", font=("Arial", 25))
    value_label.grid(row=0,column=0)
    value_text_box = Text(value_frame, height = 20, width=13)
    value_text_box.grid(row=1,column=0)

    root.mainloop()

    




def main():
    
    try:
        print("inside main function")
        # global ser
        # ser.close()
        #serial_init()
        #display_gui()
        
        #serial_operation()
        #serial_operation()
    except KeyboardInterrupt:
        #print("Shutdown requested...exiting")
        # global ser
        ser.close()
    except Exception:
        traceback.print_exc(file=sys.stdout)
        ser.close()
    sys.exit(0)

if __name__ == "__main__":
    serial_init()
    # creating thread
    
    gui_display_thread = threading.Thread(target=display_gui)
    serial_thread = threading.Thread(target=serial_ops)
    gui_update_thread = threading.Thread(target=gui_update)
    
    # starting thread 1
    serial_thread.start()
    # starting thread 2
    gui_update_thread.start()
    gui_display_thread.start()
  
    main()
    # wait until thread 1 is completely executed
    serial_thread.join()
    # wait until thread 2 is completely executed
    gui_update_thread.join()
    

