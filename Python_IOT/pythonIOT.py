import paho.mqtt.client as mqtt
import mysql.connector

con = mysql.connector.connect(host='localhost', database='Iot', user='root', password='senha1234')

if con.is_connected():
    db_info = con.get_server_info()
    print("Conentado com sucesso ao Servidor ", db_info)
    cursor = con.cursor()
    cursor.execute("select database();")
    linha = cursor.fetchone()
    print("Conectado ao DB", linha)

    createTable = """
                CREATE TABLE dadosIoT (id INT AUTO_INCREMENT, 
                                       mensagem TEXT(512), 
                                       PRIMARY KEY (id));
            """
    try:
        cursor.execute(createTable)
    except:
        print("Tabela dadosIoT já existe.")
        pass

def print_hi(name):
    # mensagem inicial
    print(f'Hi, {name}')

def on_connect(client, userdata, flags, rc):
    print("Connectado com codigo "+str(rc))

    client.subscribe("topico_sensor_fotorresistor_teste-1230")
    client.subscribe("topico_sensor_chuva_teste-1230")
    client.subscribe("topico_sensor_presenca_teste-1230")

def on_message(client, userdata, msg):
    print("Mensagem recebida no tópico: " + msg.topic)
    print("Mensagem: "+ str(msg.payload.decode()) + "º")

    cursor = con.cursor()
    cursor.execute("INSERT INTO dadosIoT (mensagem) VALUES ('{}')".format(str(msg.payload.decode())))
    con.commit()

    cursor.execute("SELECT * FROM dadosIoT")
    myresult = cursor.fetchall()
    print(myresult)

    if str(msg.payload.decode().strip()) == "termina":
        print("Recebeu comando termina.")
        if con.is_connected():
            cursor.close()
            con.close()
            print("Fim da conexão com o Banco dadosIoT")

    if str(msg.payload.decode().strip())  == "delete":
        cursor.execute("TRUNCATE TABLE dadosIoT")


if __name__ == '__main__':
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect("test.mosquitto.org", 1883, 60)
    client.loop_forever()
