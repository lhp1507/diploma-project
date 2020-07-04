import paho.mqtt.client as mqtt
import MyConn
# import FileInsert
# import FileHandle

# Public code scope
# Define on_publish event function
def on_publish(client, userdata, mid):
    print("Message Published...")

#hàm thông báo khi cơ sở dữ liệu có sự thay đổi
def changeData(count):
    # Initiate MQTT Client
    mqttc = mqtt.Client()
    # Register publish callback function
    mqttc.on_publish = on_publish
    # Connect with MQTT Broker
    mqttc.connect("localhost", 1883, 60)
    # Publish message to MQTT Broker
    mqttc.publish("changeData", "y")
    # Publish message to MQTT Broker
    mqttc.publish("Reminds", "C" + str(count))
    # Disconnect from MQTT_Broker
    mqttc.disconnect()

# hàm thông báo của báo thức
def notif(count):
    # print("Hello baby")
    # Initiate MQTT Client
    mqttc = mqtt.Client()
    # Register publish callback function
    mqttc.on_publish = on_publish
    # Connect with MQTT Broker
    mqttc.connect("localhost", 1883, 60)
    # Publish message to MQTT Broker
    mqttc.publish("Reminds", "Alarm")
    # Publish message to MQTT Broker
    mqttc.publish("Reminds", "C" + str(count))
    # Disconnect from MQTT_Broker
    mqttc.disconnect()

# Subcribe code scope
def insert_temp(temperature):  #insert nhiet do vao db
    connection = MyConn.getConnection()
    # print("Connect successful to temp!")
    try:
        cursor = connection.cursor()
        sql = "INSERT INTO temperature (temperature) VALUES (%s)"
        val = (temperature)
        cursor.execute(sql, val)
        connection.commit()
    finally:
        connection.close()
    return

def insert_humidity(humidity):  #insert do am vao db
    connection = MyConn.getConnection()
    # print("Connect successful to humidity!")
    try:
        cursor = connection.cursor()
        sql = "INSERT INTO humidity (humidity) VALUES (%s)"
        val = (humidity)
        cursor.execute(sql, val)
        connection.commit()
    finally:
        connection.close()
    return

def insert_times(seconds):
    connection = MyConn.getConnection()
    print("connect seccessfull to times")
    try:
        cursor = connection.cursor()
        sql = "INSERT INTO times (hour, minute, seconds) VALUES (%s, %s, %s)"
        val = (seconds//3600, seconds%3600//60 , seconds)
        cursor.execute(sql, val)
        connection.commit()
    finally:
        connection.close()
    return

def Temp(Data) :
    insert_temp(Data)
    return

def Hum(Data) :
    insert_humidity(Data)
    return

def times(Data) :
    insert_times(Data)
    return

def on_connect(mosq, obj, rc):  #2
    print("rc: " + str(rc))

def on_message(mosq, obj, msg): #1
    print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))
    topic = msg.topic
    message_payload = str(msg.payload)
    length = len(message_payload)
    check = message_payload[2:3]
    if check == "T" :
        Data = message_payload[3:length-1]
        # print("Temperature: "+Data)
        mqtt.Client().publish("temperature",Data,2,True)
        Temp(Data)
    elif check == "H" :
        Data = message_payload[3:length-1]
        # print("Humidity: "+Data)
        mqtt.Client().publish("humidity", Data, 2, True)
        Hum(Data)
    elif check == "G" :
        Data =message_payload[3:length-1]
        Data = int(Data)
        print("Seconds: ", Data//3600, Data%3600//60)
        mqtt.Client().publish("times", Data, 2, True)
        times(Data)

def on_subscribe(mosq, obj, mid, granted_qos):      #4          #nhận dữ liệu từ topic tên Topic
    print("Subscribed: " + str(mid) + " " + str(granted_qos))

def on_log(mosq, obj, level, string):
    print(string)

# mqttc = mqtt.Client() # Initiate MQTT Client
# # Assign event callbacks
# mqttc.on_message = on_message   #1
# mqttc.on_connect = on_connect   #2
#
# # Register publish callback function
# mqttc.on_subscribe = on_subscribe #4
#
# # Connect with MQTT Broker
# mqttc.connect("localhost", 1883, 60)
#
# mqttc.subscribe("Topic", 2 )
#
# mqttc.loop_forever()
