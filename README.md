# 1. ESP32Garden

## 1.1. Index

- [1. ESP32Garden](#1-esp32garden)
  - [1.1. Index](#11-index)
  - [1.2. Infrastructure](#12-infrastructure)
    - [1.2.1. Pages of interest](#121-pages-of-interest)
  - [1.3. Links](#13-links)

## 1.2. Infrastructure

We are going to create a free infrastructure to be able to support data, monitor and manage the system remotely. We are gonna use 3 webs. Is recommendable that you learn a bit about them:

- [Blynk.io]: With this, we can interact with the system enabling remote pins, and active pumps...
- [InfluxDB]: Here we are going to send the data compiled by our system. Moisture, temp, humidity...
- [Grafana]: Cloud version. Create dashboards with InfluxDB data to being able to monitor how our system is working over time

### 1.2.1. Pages of interest

How to connect [InfluxDB Cloud] and [Grafana]. You have 2 options about how [Grafana] will query the InfluxDB data. We are going to use Flux method <https://docs.influxdata.com/influxdb/cloud/tools/grafana/>
When you create the InfluxDB token which [Grafana] will use, in permissions only select read from your designed bucket

## 1.3. Links

[Blynk.io]: (https://blynk.io/)
[InfluxDB]: (https://cloud2.influxdata.com/signup)
[Grafana]: (https://grafana.com/products/cloud/)