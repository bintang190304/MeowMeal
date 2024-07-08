from flask import Flask, render_template
from flask_socketio import SocketIO, emit
import paho.mqtt.client as mqtt
import json
import threading

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)

# Konfigurasi MQTT
mqtt_broker = "broker.emqx.io"
mqtt_port = 1883
mqtt_topic = "sensor/data"
feedControlTopic = "feeding/data"  # Pastikan topik ini sesuai dengan yang di Arduino

# Callback ketika terhubung ke broker MQTT
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(mqtt_topic)

# Callback ketika pesan diterima dari broker MQTT
def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))
    data = json.loads(msg.payload.decode())
    socketio.emit('mqtt_message', data)

@socketio.on('feed_toggle')
def handle_feed_toggle(data):
    print(f"Feed toggle switched to {data['action']} at {data['timestamp']}")
    mqtt_client.publish(feedControlTopic, data['action'])
    if data['action'] == 'on':
        # Emit to client to update feeding history
        socketio.emit('feeding_history', {'status': 'Feeding Cat', 'time': data['timestamp']})

        # Schedule to send "off" message after 20 seconds
        def turn_off_motor():
            mqtt_client.publish(feedControlTopic, 'off')
            socketio.emit('feeding_history', {'status': 'Feeding Stopped', 'time': data['timestamp']})
        
        threading.Timer(20.0, turn_off_motor).start()

# Inisialisasi MQTT Client
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(mqtt_broker, mqtt_port, 60)

# Jalankan MQTT Client dalam thread terpisah
mqtt_client.loop_start()

# Route untuk halaman utama
@app.route('/')
def index():
    return render_template('index.html')

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)
