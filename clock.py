from math import pi
import datetime

import ephem
from flask import Flask, jsonify, request, send_file, send_from_directory

app = Flask(__name__)

ASTRAL_BODIES = {
        'sun': ephem.Sun,
        'venus': ephem.Venus,
        'mars': ephem.Mars,
        'jupiter': ephem.Jupiter,
        'moon': ephem.Moon
}

@app.route('/horizon', methods=['GET'])
def get_horizon():
    curr_time = datetime.datetime.utcnow()
    o = ephem.Observer()
    o.lat = request.args.get('lat')
    o.lon = request.args.get('lon')
    o.date = curr_time
    body = ephem.Sun()
    rising = o.next_rising(body).datetime() if curr_time > o.next_setting(body).datetime() else o.previous_rising(body).datetime()
    o.date = rising
    next_setting = o.next_setting(body).datetime()
    rise_angle = (rising.hour + rising.minute / 60.0 + rising.second / 3600.0) / 12 * pi
    set_angle = (next_setting.hour + next_setting.minute / 60.0 + next_setting.second / 3600.0) / 12 * pi
    return jsonify(rise_angle=rise_angle, set_angle=set_angle)

@app.route('/angle', methods=['GET'])
def get_angle():
    curr_time = datetime.datetime.utcnow()
    o = ephem.Observer()
    o.lat = request.args.get('lat')
    o.lon = request.args.get('lon')
    o.date = curr_time
    body = ASTRAL_BODIES[request.args.get('body')]()
    body.compute()
    rising = o.next_rising(body) if curr_time > o.next_setting(body).datetime() else o.previous_rising(body)
    o.date = rising
    next_setting = o.next_setting(body)
    if next_setting >= rising:
        angle = ((curr_time - rising.datetime()).total_seconds() /
                 (o.next_setting(body).datetime() - rising.datetime()).total_seconds()) * pi
    else:
        angle = ((curr_time - o.next_setting(body).datetime()).total_seconds() /
                 (rising.datetime() - o.next_setting(body).datetime()).total_seconds()) * 2 * pi
    return jsonify(name=body.name, angle=angle)


@app.route('/')
def serve_index():
    return send_from_directory('static', 'index.html')

@app.route('/js/<path:filename>')
def serve_js(filename):
    return send_from_directory('static/js', filename)

if __name__ == '__main__':
    app.run(debug=True)
