from flask import Flask, render_template, send_from_directory
import firebase_admin
from firebase_admin import credentials, db

app = Flask(__name__)

# Firebase for server-side
cred = credentials.Certificate("serviceAccountKey.json")
firebase_admin.initialize_app(cred, {"databaseURL": "https://smart-parking-system-5376-default-rtdb.asia-southeast1.firebasedatabase.app"})
ref_entry = db.reference("entry")
ref_exit = db.reference("exit")

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/payment')
def get_payment_status():
    status = db.reference("paymentStatus/exit").get()
    return {"paid": bool(status)}

@app.route('/firebase-config.json')
def serve_config():
    return send_from_directory('.', 'firebase-config.json')

if __name__ == "__main__":
    app.run(debug=True, host='0.0.0.0', port=5000)