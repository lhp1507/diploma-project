import MyConn

def insert_temp(temperature):  #insert nhiet do vao db
    connection = MyConn.getConnection()
    print("Connect successful to temp!")
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
    print("Connect successful to humidity!")
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