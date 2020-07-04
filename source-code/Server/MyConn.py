import pymysql.cursors

# Hàm trả về một connection.
def getConnection():
    # Bạn có thể thay đổi các thông số kết nối.
    connection = pymysql.connect(host='localhost',
                                 user='root',
                                 password='',
                                 db='day',
                                 charset='utf8')

    return connection
