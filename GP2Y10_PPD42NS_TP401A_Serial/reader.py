#!/usr/bin/env python3
# encoding: utf-8

# https://github.com/kravietz/air-quality/

import configparser

config = configparser.RawConfigParser()
config.read('reader.ini')

# config file should look like this:
# [dust]
# api_key=KEY
# feed_id=ID

api_key = config.get('xively', 'api_key')
feed_id = config.getint('xively', 'feed_id')

analog_voltage = config.getfloat('arduino', 'analog_voltage')
serial_device = config.get('arduino', 'device')

print(api_key, feed_id, analog_voltage)

import xively

api = xively.XivelyAPIClient(api_key)
feed = api.feeds.get(feed_id)
print(feed)

import serial

s = serial.Serial(serial_device, 9600)
print(s)

import datetime
import requests

q = []

import csv
import time
import os

fout = open('{0}.csv'.format(time.time()), 'wt')
cw = csv.writer(fout)
header = ['Time', 'GP2Y10', 'PPD42NS', 'TP401A']
try:
    cw.writerow(header)
except AttributeError:
    cw.writerow(header)

while(True):
        line = s.readline().strip().decode('ascii')
        now = datetime.datetime.utcnow()

        if(line.startswith('#')):
            print(line)
            continue

        try:
            gp,tp,pp,rh,tc = line.split(',')
            gp,tp,pp,rh,tc = list(map(float, (gp,tp,pp)))
        except ValueError:
            print("Invalid line", line)
            continue

        gp_voltage = gp*analog_voltage;	
        gp_tsp = gp_voltage*200
        gp_tsp2 = (gp_voltage-0.5)*200

        pp_tsp = (1.1*pow(pp, 3) - 3.8*pow(pp, 2) + (520*pp) + 0.62)*0.033

        rh_proc = rh*100;

        print(u"{8} Raw: {0} *** GP2Y10={1} {2}V {3}µg/m3 {4}µg/m3 *** PPD42NS={5} {6}µg/m3 *** TP401A={7} *** DHT22 {9}% {10}°C".format(line, gp, gp_voltage, gp_tsp, gp_tsp2, pp, pp_tsp, tp, now, rh_proc, tc))
        # add current results to queue
        q.append((now, gp, pp, tp, rh_proc, tc))
        cw.writerow([now, gp, pp, tp, rh_proc, tc])

        fout.flush()
        os.fsync(fout.fileno())

        try:
            for n,g,p,t,h,c in q:
                feed.datastreams = [
                    xively.Datastream(id='GP2Y10_dust',  current_value=g, at=n),
                    xively.Datastream(id='PPD42NS_dust', current_value=p, at=n),
                    xively.Datastream(id='TP401A_gas',   current_value=t, at=n),
                    xively.Datastream(id='DHT22_humid',  current_value=h, at=n),
                    xively.Datastream(id='DHT22_temp',   current_value=c, at=n),
                ]
                feed.update()
            q = []
        except (requests.exceptions.HTTPError, requests.exceptions.ConnectionError) as e:
            print(e)


