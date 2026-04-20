# ⚡ Smart Energy Meter — IoT Cloud Dashboard

A real-time energy monitoring system built with ESP32 and AWS cloud services. Monitors household voltage, current, and power consumption — sends instant alerts during peak hours and displays live data on a web dashboard.

---

## 🌐 Live Dashboard

Hosted on AWS S3:
**[energy-dashboard-achal.s3-website.ap-south-1.amazonaws.com](http://energy-dashboard-achal.s3-website.ap-south-1.amazonaws.com)**

---

## 📸 Screenshots



---

## 🏗️ Architecture

```
ESP32 Sensor
    │
    ▼
AWS IoT Core          ← MQTT over TLS
    │
    ▼
AWS Lambda            ← EnergyDecision (Python)
  ├── Peak/off-peak detection (IST timezone)
  ├── Dynamic pricing (₹8 peak / ₹5 off-peak)
  ├── Monthly bill estimation
  └── Alert condition evaluation
        │
        ├──▶ Amazon SNS      ← Email alert if high usage
        └──▶ Amazon DynamoDB ← Stores every reading
                    │
                    ▼
          AWS Lambda          ← EnergyDashboardAPI (Python)
                    │
                    ▼
          API Gateway (HTTP)  ← REST endpoint
                    │
                    ▼
          S3 Static Website   ← Live dashboard (HTML/JS)
```

---

## ✨ Features

- **Real-time monitoring** — voltage (V), current (A), power (W), energy units (kWh)
- **IST-aware peak hours** — morning peak 7–11 AM, evening peak 6–10 PM
- **Dynamic pricing engine** — ₹8/unit during peak, ₹5/unit off-peak
- **Monthly bill estimation** — calculated on every reading
- **Instant SNS alerts** — email notification when high load detected during peak hours
- **Live web dashboard** — auto-refreshes every 30 seconds
- **Historical data** — last 20 readings with charts and table
- **Peak hour map** — visual 24-hour IST timeline with current hour highlighted

---

## 🛠️ Tech Stack

| Layer | Technology |
|---|---|
| Hardware | ESP32 microcontroller |
| Connectivity | AWS IoT Core (MQTT) |
| Compute | AWS Lambda (Python 3.12) |
| Database | Amazon DynamoDB |
| Alerts | Amazon SNS (email) |
| API | Amazon API Gateway (HTTP API) |
| Frontend | HTML, CSS, JavaScript, Chart.js |
| Hosting | Amazon S3 static website |

---

## 📂 Project Structure

```
smart-energy-meter/
│
├── lambda/
│   ├── EnergyDecision.py        # Main Lambda — processes sensor data
│   └── EnergyDashboardAPI.py    # Read Lambda — serves data to dashboard
│
├── dashboard/
│   └── index.html               # Web dashboard (S3 hosted)
│
├── esp32/
│   └── energy_meter.ino         # Arduino IDE sketch for ESP32
│
└── README.md
```

---

## ⚙️ AWS Services Setup

### DynamoDB Table — `EnergyReadings`

| Field | Type | Description |
|---|---|---|
| `device_id` | String (PK) | Device identifier e.g. `esp32-001` |
| `timestamp` | String (SK) | IST datetime `YYYY-MM-DDTHH:MM:SS` |
| `voltage` | String | Voltage in volts |
| `current` | String | Current in amperes |
| `power` | String | Power in watts |
| `units` | String | Energy in kWh |
| `rate` | String | ₹/unit at time of reading |
| `cost` | String | Session cost in ₹ |
| `monthly_bill` | String | Estimated monthly bill in ₹ |
| `is_peak` | String | `"True"` or `"False"` |
| `alert_type` | String | `peak`, `offpeak`, or `normal` |

### Lambda — `EnergyDecision`

- Triggered by: **AWS IoT Core** + **API Gateway**
- Reads sensor values from event payload
- Converts UTC → IST, determines peak/off-peak
- Saves reading to DynamoDB
- Publishes SNS alert if threshold exceeded

**Alert thresholds:**
- Peak time: power > 100W or voltage > 250V → SNS alert
- Off-peak: power > 800W → SNS alert

### Lambda — `EnergyDashboardAPI`

- Triggered by: **API Gateway (HTTP GET)**
- Queries last 20 readings from DynamoDB
- Returns JSON with CORS headers enabled

---

## 🚀 Deployment Guide

### 1. Clone the repo

```bash
git clone https://github.com/yourusername/smart-energy-meter.git
cd smart-energy-meter
```

### 2. Create DynamoDB table

- Table name: `EnergyReadings`
- Partition key: `device_id` (String)
- Sort key: `timestamp` (String)
- Capacity: On-demand

### 3. Deploy Lambda functions

- Runtime: Python 3.12
- Region: `ap-south-1` (Mumbai)
- Attach `AmazonDynamoDBFullAccess` to `EnergyDecision`
- Attach `AmazonDynamoDBReadOnlyAccess` to `EnergyDashboardAPI`

### 4. Configure API Gateway

- Type: HTTP API
- Security: Open
- Enable CORS: Allow-Origin `*`, Allow-Methods `GET`

### 5. Host dashboard on S3

- Create S3 bucket with static website hosting enabled
- Upload `index.html`
- Set bucket policy for public read access
- Update `API_URL` in `index.html` with your API Gateway URL

### 6. Flash ESP32

- Install Arduino IDE
- Add ESP32 board support
- Install `PubSubClient` and `ArduinoJson` libraries
- Update WiFi credentials and AWS IoT endpoint in the sketch
- Flash and monitor via Serial

---

## 📊 Sample API Response

```json
{
  "device_id": "esp32-001",
  "count": 3,
  "readings": [
    {
      "timestamp": "2026-04-20T14:17:00",
      "voltage": 230.0,
      "current": 5.2,
      "power": 1196.0,
      "units": 1.5,
      "rate": 8.0,
      "cost": 12.0,
      "monthly_bill": 360.0,
      "is_peak": true,
      "alert_type": "peak"
    }
  ]
}
```

---

## 💰 AWS Cost Estimate

All services used stay within the **AWS Free Tier** for a typical home energy monitor:

| Service | Free Tier | Typical Usage |
|---|---|---|
| Lambda | 1M requests/month | < 50K/month |
| DynamoDB | 25 GB storage | < 1 MB/month |
| API Gateway | 1M calls/month | < 50K/month |
| SNS | 1000 emails/month | < 100/month |
| S3 | 5 GB + 20K requests | < 1 MB |

**Estimated monthly cost: ₹0 – ₹2**

---

## 🔮 Future Improvements

- [ ] Connect real ESP32 with CT sensor and ZMPT101B voltage sensor
- [ ] Add multiple device support (multiple ESP32s)
- [ ] Weekly and monthly usage reports via SNS email
- [ ] Appliance-level monitoring
- [ ] Mobile app (React Native)
- [ ] Predictive bill forecasting using ML

---

## 👨‍💻 Author

**Achal Chourasiya**
- AWS Account: `ap-south-1` (Mumbai)
- Device: `esp32-001`

---

## 📄 License

MIT License — feel free to use and modify for your own projects.

---

> Built with ☁️ AWS + ⚡ ESP32 + 🐍 Python
