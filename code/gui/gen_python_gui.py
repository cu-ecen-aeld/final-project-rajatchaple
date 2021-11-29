#graph source https://stackoverflow.com/questions/56486710/animate-pyqtgraph-in-class
import sys
import threading
from PyQt5.QtWidgets import QApplication, QLabel, QPushButton, QTextEdit, QWidget, QHBoxLayout, QFrame, QSplitter, QComboBox#, QTextEdit
from PyQt5.QtCore import Qt
from serial import Serial
import serial
import queue
from threading import *
import time
from qtpy import QtWidgets
from qt_thread_updater.widgets.quick_text_edit import QuickTextEdit
from qt_thread_updater import get_updater
from enum import Enum
from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
from numpy import arange, sin, cos, pi
import pyqtgraph as pg
import glob

from serial.serialutil import PARITY_EVEN

global user_logged_in
user_logged_in = False

global module_loaded
module_loaded = False




def serial_ports():
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result


protocols = ["SPI0_CH0", "SPI0_CH1", "I2C", "UART"]
serial_ports_list = serial_ports()
baud_rates = ["115200","4800","9600","38400"]
data_length_list = ["8 bit", "7 bit"]
parity = ["none", "odd", "even"]
stop_bits = ["1","1.5"]

global ser

serial_read_queue = queue.Queue()
spi_data_label_queue = queue.Queue()
spi_data_graph_queue = queue.Queue()

global ser_char_read_mode
ser_char_read_mode = False



def read_from_port(ser):
    global ser_char_read_mode
    reading = ""
    while True:
        #print("serial_test")
        try:
            if(ser_char_read_mode == False):
                reading = ""
                reading = ser.readline().decode()
                #print(reading)
                serial_read_queue.put(reading)
                if(module_loaded == True):
                    if(reading[0].isdigit()):
                        reading = (int(reading[0:-2]) * 5 / 1024)
                        spi_data_label_queue.put(str(reading)[0:4] + "v")
                        spi_data_graph_queue.put(reading)
                        #print(int(reading))

            else:
                received_char = ser.read().decode()
                if(received_char == '#'):
                    ser_char_read_mode = False
                reading = reading + received_char
                if(user_logged_in == True):
                    serial_read_queue.put(reading)
                    reading = ""
                elif(received_char == ':'):
                    serial_read_queue.put(reading)
                    reading = ""
                #print(reading)

            #reading = ser.readline().decode()
            
        except:
            print("problem decoding a byte")
        time.sleep(0.010)
        #handle_data(reading)

global bottom_text_edit, spi_value_label, send_in_loop_button

def gui_update_text_box():
    global position_update_timer
    global bottom_text_edit
    global ser_char_read_mode
    line_read = ""
    lines_list = []
    bottom_text_edit.append("Welcome to the world")
    while(1):
        line_read = serial_read_queue.get()
        #get_updater().call_latest(spi_value_label.setText, spi_data_queue.get())
        #time.sleep(0.300)
        get_updater().call_latest(bottom_text_edit.append, line_read)
        #spi_value_label.setText()
        #bottom_text_edit.append(line_read)
        #lines_list.append(line_read)
        time.sleep(0.200)
        if("Welcome to AESD" in line_read):
            ser_char_read_mode = True
        #print(position_update_timer.remainingTime())

def gui_update_value_label():
    while(1):
        line_read = serial_read_queue.get()
        get_updater().call_latest(spi_value_label.setText, spi_data_label_queue.get())
        time.sleep(0.010)

global port_list,baud_list, data_list, parity_list, stop_bit_list
# use threading
def threading_serial(start_button):
    global ser
    ser = serial.Serial()
    if(start_button.isChecked()):
        start_button.setText("Disconnect")
        #starting serial
        if(parity_list.currentText() == "none"):
            s_parity = 'N'
        elif(parity_list.currentText() == "odd"):
            s_parity = 'O'
        else:
            s_parity = 'E'

        if(data_list.currentText() == "8 bit"):
            s_data_length = 8
        else:
            s_data_length = 7

        if(stop_bit_list.currentText() == "1"):
            s_stop_bit = 1
        else:
            s_stop_bit = 1.5

        ser = Serial(
                port=port_list.currentText(),\
                baudrate=int(baud_list.currentText()),\
                parity=s_parity,\
                stopbits=s_stop_bit,\
                bytesize=s_data_length)
        # Call work function
        t_serial_read=Thread(target=read_from_port, args=(ser,))
        t_serial_read.start()
    else:
        print("closing serial port")
        start_button.setText("Connect")
        ser.close()

def send_text(text_to_be_sent):
    global ser
    text_to_be_sent += "\r\n"
    try:
        ser.write(text_to_be_sent.encode())
    except:
        print("Incorrect serial port configuration")
    time.sleep(0.10)

global stop_thread_t_serial_cont_write
def send_text_thread_work(text_to_be_sent):
    while(1):
        send_text(text_to_be_sent)
        time.sleep(0.8)
        if(stop_thread_t_serial_cont_write == True):
            break

def init_continuous_write(protocol, send_in_loop_button):
    global module_loaded, stop_thread_t_serial_cont_write
    stop_thread_t_serial_cont_write = False
    if(send_in_loop_button.isChecked()):
        if(protocol == "SPI0_CH0"):
            send_text("insmod /lib/modules/4.19.79/extra/adc_spi_char.ko")
            send_text("insmod /lib/modules/4.19.79/extra/adc_spi_low_level.ko")
            module_loaded = True
            t_serial_cont_write = Thread(target=send_text_thread_work, args=("cat /sys/class/spiclient/spi0/read0",))
            t_serial_cont_write.start()
        elif(protocol == "SPI0_CH1"):
            send_text("insmod /lib/modules/4.19.79/extra/adc_spi_char.ko")
            send_text("insmod /lib/modules/4.19.79/extra/adc_spi_low_level.ko")
            module_loaded = True
            t_serial_cont_write = Thread(target=send_text_thread_work, args=("cat /sys/class/spiclient/spi0/read1",))
            t_serial_cont_write.start()
        else:
            print("No Devices found")
    else:
        stop_thread_t_serial_cont_write = True



def send_command(send_serial_text_edit):
    entered_text = send_serial_text_edit.toPlainText()
    entered_text_list = entered_text.split("\n")
    for text in entered_text_list:
        send_text(text)


def create_text_edit_widget(text):
    widget = QuickTextEdit()    #QTextEdit()    #QTextEdit crashes as 
    widget.setText(text)
    return widget


def create_qframe():
    frame = QFrame()
    frame.setFrameShape(QFrame.StyledPanel)
    frame.setMinimumSize(100, 200)
    return frame


def create_layout(layout_type=QHBoxLayout, orientation=Qt.Horizontal):
    if(layout_type == QHBoxLayout):
        layout = QHBoxLayout()
    elif(layout_type == QSplitter):
        layout = QSplitter(orientation)
    return layout


def add_to_layout(layout, widget):
    layout.addWidget(widget)
    return layout

FREQUENCY = .4
LEFT_X = -10
RIGHT_X = 0
X_Axis = np.arange(LEFT_X, RIGHT_X, FREQUENCY)
buffer = int((abs(LEFT_X) + abs(RIGHT_X))/FREQUENCY)
data = []

global graph
def plot_updater():
    global graph
    if(spi_data_graph_queue.empty() == False):
        dataPoint = spi_data_graph_queue.get()
        if len(data) >= buffer:
            del data[:1]
        data.append(dataPoint)
        graph.setData(X_Axis[len(X_Axis) - len(data):], data)

def protocol_list_text_changed(protocol_list):
    global stop_thread_t_serial_cont_write
    send_in_loop_button.setText(protocol_list.currentText())
    stop_thread_t_serial_cont_write = True

global position_update_timer
def start_timer():
    global position_update_timer
    position_update_timer = QtCore.QTimer()
    position_update_timer.timeout.connect(plot_updater)#(lambda: plot_updater(graph))
    position_update_timer.start(500)
    print("Timer started")

def main():
    global bottom_text_edit,spi_value_label, graph, send_in_loop_button
    global port_list,baud_list, data_list, parity_list, stop_bit_list

    app = QApplication(sys.argv)
    window = QWidget()

    
    top_left_qframe = create_qframe()
    port_label = QLabel("Port:", top_left_qframe)
    port_label.move(40, 30)
    port_list = QComboBox(top_left_qframe)
    port_list.addItems([e for e in serial_ports_list])
    port_list.move(130, 30)
    port_list.setFixedWidth(60)


    Baud_label = QLabel("Baud_rate:", top_left_qframe)
    Baud_label.move(40, 60)
    baud_list = QComboBox(top_left_qframe)
    baud_list.addItems(baud_rates)
    baud_list.move(130, 60)
    baud_list.setFixedWidth(60)

    Data_label = QLabel("Data:", top_left_qframe)
    Data_label.move(40, 90)
    data_list = QComboBox(top_left_qframe)
    data_list.addItems(data_length_list)
    data_list.move(130, 90)
    data_list.setFixedWidth(60)


    Parity = QLabel("Parity:", top_left_qframe)
    Parity.move(40, 120)
    parity_list = QComboBox(top_left_qframe)
    parity_list.addItems(parity)
    parity_list.move(130, 120)
    parity_list.setFixedWidth(60)

    Stop_bit = QLabel("Stop:", top_left_qframe)
    Stop_bit.move(40, 150)
    stop_bit_list = QComboBox(top_left_qframe)
    stop_bit_list.addItems(stop_bits)
    stop_bit_list.move(130, 150)
    stop_bit_list.setFixedWidth(60)

    start_button = QPushButton("CONNECT", top_left_qframe)
    start_button.clicked.connect(lambda: threading_serial(start_button))
    start_button.move(230, 70)
    start_button.setFixedSize(70,50)
    start_button.setCheckable(True)
    # start_button.setStyleSheet("QPushButton {border-radius: 5px}"
    #                   "QPushButton:pressed {border-radius: 5px}" ) # background-color: blue; 


    #top_left_qframe = add_to_layout(top_left_qframe, QPushButton("START"))
    top_center_qframe = create_qframe()
    graph_widget = pg.PlotWidget(parent=top_center_qframe)
    graph_widget.plotItem.setMouseEnabled(x=False, y=False)
    #p1.plotItem.vb.setLimits(xMin=a, xMax=b, yMin=c, yMax=d)
    graph_widget.setXRange(LEFT_X, RIGHT_X)
    graph_widget.setTitle('Voltage')
    graph_widget.setLabel('left', 'Value')
    graph_widget.setLabel('bottom', 'Time (s)')
    graph = graph_widget.plot()
    graph_widget.setMinimumSize(340, 200)
    #graph.setPen(197,235,255)
    graph_widget.setYRange(0,5, padding=0)
    

    top_right_qframe = create_qframe()
    spi_value_label = QLabel("0v", top_right_qframe, )
    spi_value_label.move(90, 50)
    
    spi_value_label.setStyleSheet(''' font-size: 40px; ''')
    spi_value_label.resize(150,90)
    spi_value_label.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
    # send_serial_qframe = create_qframe
    # ()
    # send_serial_text_edit = QTextEdit("", send_serial_qframe)
    # send_serial_button = QPushButton("SEND", send_serial_qframe)
    
    bottom_text_edit = create_text_edit_widget("")

    layout = create_layout(QHBoxLayout)

    horizontal_splitter = create_layout(QSplitter, Qt.Horizontal)
    horizontal_splitter.addWidget(top_left_qframe)
    horizontal_splitter.addWidget(top_center_qframe)
    horizontal_splitter.addWidget(top_right_qframe)
    horizontal_splitter.setSizes([100, 100, 100])

    send_serial_layout = create_layout(QSplitter, Qt.Horizontal)
    protocol_list = QComboBox()
    protocol_list.addItems(protocols)#(["SPI CH0", "SPI CH1", "I2C", "UART"])
    protocol_list.currentIndexChanged.connect(lambda: protocol_list_text_changed(protocol_list))
    send_serial_layout.addWidget(protocol_list)
    send_serial_text_edit = QTextEdit("")
    send_serial_text_edit.setFixedHeight(25)
    send_serial_layout.addWidget(send_serial_text_edit)
    send_serial_button = QPushButton("SEND")
    send_serial_button.clicked.connect(lambda: send_command(send_serial_text_edit))
    send_serial_layout.addWidget(send_serial_button)
    send_in_loop_button = QPushButton(protocol_list.currentText())
    send_in_loop_button.clicked.connect(lambda: init_continuous_write(protocol_list.currentText(), send_in_loop_button))
    send_in_loop_button.setCheckable(True)
    send_serial_layout.addWidget(send_in_loop_button)
    
    

    vertical_splitter = create_layout(QSplitter, Qt.Vertical)
    vertical_splitter = add_to_layout(vertical_splitter, horizontal_splitter)
    vertical_splitter = add_to_layout(vertical_splitter, send_serial_layout)
    vertical_splitter = add_to_layout(vertical_splitter, bottom_text_edit)

    
    layout = add_to_layout(layout, vertical_splitter)
    

    window.setLayout(layout)
    window.setGeometry(300, 300, 1000, 550)
    window.setWindowTitle("AESD : READ DATA")

    #t_gui = Thread(target=gui_update, args=(bottom_text_edit,))
    t_gui_text_box = Thread(target=gui_update_text_box)
    t_gui_text_box.start()

    t_gui_value_label = Thread(target=gui_update_value_label)
    t_gui_value_label.start()

    #t_gui_graph = Thread(target=plot_updater, args = (graph,))
    #t_gui_graph.start()

    start_timer()
    
    window.show()
    app.exec_()
    print("exited")
    #sys.exit(app.exec_())


main()
