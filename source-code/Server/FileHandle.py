# Import package
#import socket
import paho.mqtt.client as mqtt
import MyConn
import ConnectTopic
import datetime
import time
import threading
# Declare global variable
n = 0

# Hàm đếm có bao nhiêu báo thức tới hạn
def checkPosi():
    connection = MyConn.getConnection()
    try:
        cursor = connection.cursor()
        sql = "SELECT seconds FROM times ORDER BY seconds"
        cursor.execute(sql)
        myresult = cursor.fetchall()
        connection.commit()  # để thực hiện commit data khi có thay đổi
    finally:
        # print("Connect successful to database!")
        connection.close()
        myresult = [item for t in myresult for item in t]  # chuyển sang kiểu int để có thể trừ trong kiểu list
        now = datetime.datetime.now()
        secondsOfNow = now.hour * 3600 + now.minute * 60 + now.second
        for i in range(len(myresult)):
            myresult[i] = myresult[i] - secondsOfNow
# Tính toán lại số phần tử dương còn lại trong danh sách thời gian
    countOfPosi = 0
    for i in myresult:
        if(i > 0):
            countOfPosi += 1
        # else:
        #     countOfPosi = 0
    return countOfPosi

# Hàm dùng để tìm thời gian vừa cập nhập gần với thời gian hiện tại nhất và báo thức
def Timer():
    connection = MyConn.getConnection()
    try:
        cursor = connection.cursor()
        sql = "SELECT seconds FROM times ORDER BY seconds"
        cursor.execute(sql)
        myresult = cursor.fetchall()
        connection.commit()  # để thực hiện commit data khi có thay đổi
    finally:
        # print("Connect successful to database!")
        connection.close()
        myresult = [item for t in myresult for item in t] #chuyển sang kiểu int để có thể trừ trong kiểu list
        now = datetime.datetime.now()
        secondsOfNow = now.hour * 3600 + now.minute * 60 + now.second
        for i in range(len(myresult)):
            myresult[i] = myresult[i] - secondsOfNow
        # print("myresult:", myresult)1
        value, position = min(((b, a) for a, b in enumerate(myresult) if b > 0), default=(None, None))
        print("Số giây còn lại: ", value)
        if(value != None):
            print("Timer was activated")
            for i in range(value):
                time.sleep(1)
                if (checker() == 1 and value >= 30):
                    print("**************************************************************************************")
                    ConnectTopic.changeData(checkPosi())
                    return 0
            ConnectTopic.notif(checkPosi())
            ConnectTopic.changeData(checkPosi())
        else:
            print("None timer")

def checker():
    global n # nếu không khải báo là biến toàn cục thì sau khi gán n trong hàm sẽ xảy ra lỗi UnboundLocalErr
    # while 1:
    connection = MyConn.getConnection()
    try:
        cursor = connection.cursor()
        cursor.execute("SELECT id FROM times")
        id = cursor.fetchall()
        # Lựa chọn một hàng  # str(''.join(map(str, myresult))) đẻ Chuyển kiểu dữ liệu của myresult sang kiểu str nếu muốn thêm khoảng trống thì bỏ space vào trong dấu nháy đơn
        connection.commit()  # để thực hiện commit data khi có thay đổi
    finally:
        connection.close()
    if ((n != len(id))):
        n = len(id)
        return 1
    elif (checkPosi() != 0):
        return 2
    # else:
    #     return 0


def worker():
    while 1:
        # print(" * ")
        if checker() == 1:
            print("was different!")
            ConnectTopic.changeData(checkPosi()) #Thông báo khi cơ sở dữ liệu thay đổi
            Timer()
        elif checker() == 2:
            print("Positive values!")
            # ConnectTopic.changeData(checkPosi())
            Timer()
        time.sleep(1)

def waiter():
    print("_")
    time.sleep(1)

if __name__ == '__main__':
    #print("Your Computer IP Address is:" + socket.gethostbyname(socket.gethostname()))
    a = threading.Thread(target = worker)
    b = threading.Thread(target = waiter)
    print("worker was started")
    a.start()
    print("waiter was started")
    b.start()

    # Initiate MQTT Client
    mqttc = mqtt.Client()
    # Assign event callbacks
    mqttc.on_message = ConnectTopic.on_message  # 1
    mqttc.on_connect = ConnectTopic.on_connect  # 2

    # Register publish callback function
    mqttc.on_subscribe = ConnectTopic.on_subscribe  # 4

    # Connect with MQTT Broker
    mqttc.connect("localhost", 1883, 60)

    mqttc.subscribe("Topic", 2)

    mqttc.loop_forever()






