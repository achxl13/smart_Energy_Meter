import json
import boto3
from datetime import datetime, timedelta

sns = boto3.client('sns', region_name='ap-south-1')
dynamodb = boto3.resource('dynamodb', region_name='ap-south-1')
table = dynamodb.Table('EnergyReadings')

TOPIC_ARN = "arn:aws:sns:ap-south-1:207066930794:Energyalert"

def lambda_handler(event, context):

    try:
        # -------- INPUT --------
        voltage = float(event.get('voltage', 0))
        current = float(event.get('current', 0))
        power = float(event.get('power', 0))
        units = float(event.get('units', 0))

        # -------- TIME CONVERSION --------
        utc_time = datetime.utcnow()
        ist_time = utc_time + timedelta(hours=5, minutes=30)
        hour = ist_time.hour

        print("IST Hour:", hour)

        # -------- PEAK HOURS --------
        morning_peak = 7 <= hour <= 11
        evening_peak = 18 <= hour <= 22
        is_peak = morning_peak or evening_peak

        # -------- PRICE SYSTEM --------
        normal_rate = 5
        peak_rate = 8

        current_rate = peak_rate if is_peak else normal_rate
        cost = units * current_rate

        # -------- MONTHLY BILL ESTIMATION --------
        monthly_bill = cost * 30

        # -------- SAVE TO DYNAMODB --------
        table.put_item(Item={
            'device_id': 'esp32-001',
            'timestamp': ist_time.strftime('%Y-%m-%dT%H:%M:%S'),
            'voltage': str(voltage),
            'current': str(current),
            'power': str(power),
            'units': str(units),
            'rate': str(current_rate),
            'cost': str(round(cost, 2)),
            'monthly_bill': str(round(monthly_bill, 2)),
            'is_peak': str(is_peak),
            'alert_type': 'peak' if is_peak and (power > 100 or voltage > 250) else ('offpeak' if (not is_peak) and power > 800 else 'normal')
        })

        # -------- ALERT CONDITIONS --------
        peak_high_voltage_alert = is_peak and (power > 100 or voltage > 250)

        offpeak_high_power_alert = (not is_peak) and power > 800

        message = ""

        # 🔥 PEAK TIME ALERT
        if peak_high_voltage_alert:
            message = f"""⚡ PEAK TIME HIGH USAGE ALERT ⚡

Voltage: {voltage} V
Current: {current} A
Power: {power} W
Units: {units} kWh

💰 Current Price: ₹{current_rate}/unit
📊 Estimated Monthly Bill: ₹{round(monthly_bill,2)}

⚠️ You are using high load during PEAK hours!
👉 Reduce usage to save electricity cost."""

            sns.publish(
                TopicArn=TOPIC_ARN,
                Message=message,
                Subject="⚡ Peak Time Energy Alert"
            )

        # 🌙 OFF-PEAK HIGH POWER ALERT
        elif offpeak_high_power_alert:
            message = f"""🌙 OFF-PEAK HIGH POWER ALERT 🌙

Voltage: {voltage} V
Power: {power} W
Units: {units} kWh

💰 Current Price: ₹{current_rate}/unit
📊 Estimated Monthly Bill: ₹{round(monthly_bill,2)}

⚠️ High power usage detected in OFF-PEAK time!
👉 Please check heavy appliances."""

            sns.publish(
                TopicArn=TOPIC_ARN,
                Message=message,
                Subject="🌙 Off-Peak Power Alert"
            )

        # ✅ NORMAL
        else:
            message = f"""✅ NORMAL USAGE

Voltage: {voltage} V
Power: {power} W
Units: {units} kWh

💰 Current Price: ₹{current_rate}/unit
📊 Estimated Monthly Bill: ₹{round(monthly_bill,2)}"""

        print(message)

        return {
            'statusCode': 200,
            'body': json.dumps(message)
        }

    except Exception as e:
        print("ERROR:", str(e))
        return {
            'statusCode': 500,
            'body': json.dumps("Error occurred")
        }