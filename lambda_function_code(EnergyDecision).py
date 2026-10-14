import json
import boto3
from datetime import datetime, timedelta

sns = boto3.client('sns', region_name='ap-south-1')
dynamodb = boto3.resource('dynamodb', region_name='ap-south-1')
table = dynamodb.Table('EnergyReadings')

TOPIC_ARN = "arn:aws:sns:ap-south-1:207066930794:Energyalert"

# -------- TARIFF SCHEDULE (IST) --------
# ON-PEAK  : 7:00 AM - 11:00 AM  and  6:00 PM - 10:00 PM   -> Rs 8/unit
# OFF-PEAK : 11:00 AM - 6:00 PM  and  10:00 PM - 7:00 AM   -> Rs 5/unit
PEAK_RATE = 8
OFFPEAK_RATE = 5

PEAK_TEXT = "7:00 AM - 11:00 AM, 6:00 PM - 10:00 PM"
OFFPEAK_TEXT = "11:00 AM - 6:00 PM, 10:00 PM - 7:00 AM"

# -------- LIMITS --------
PEAK_USAGE_LIMIT = 100    # W  - above this in peak hours -> status email


def is_peak_hour(hour):
    return (7 <= hour < 11) or (18 <= hour < 22)


def shift_recommendation(hour):
    if 7 <= hour < 11:
        return "Shift non-essential loads to off-peak hours: 11:00 AM - 6:00 PM"
    return "Shift non-essential loads to off-peak hours: 10:00 PM - 7:00 AM"


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
        hour = int(event.get('test_hour', ist_time.hour))   # test_hour is only for testing

        print("IST Hour:", hour)

        # -------- PEAK / OFF-PEAK --------
        is_peak = is_peak_hour(hour)

        current_rate = PEAK_RATE if is_peak else OFFPEAK_RATE
        cost = units * current_rate

        # -------- MONTHLY BILL ESTIMATION --------
        monthly_bill = cost * 30

        # -------- ALERT CONDITIONS --------
        peak_alert = is_peak and power > PEAK_USAGE_LIMIT    # high usage in peak hours

        alert_type = 'peak' if peak_alert else 'normal'

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
            'alert_type': alert_type
        })

        # -------- MESSAGE --------
        subject = None

        # PEAK HOUR STATUS
        if peak_alert:
            message = f"""Status: ON-PEAK
Voltage: {voltage} V
Current: {current} A
Units: {units} kWh
Power: {power} W
Current Price: Rs {current_rate}/unit
Estimated Monthly Bill: Rs {round(monthly_bill, 2)}

On-peak hours  (Rs {PEAK_RATE}/unit): {PEAK_TEXT}
Off-peak hours (Rs {OFFPEAK_RATE}/unit): {OFFPEAK_TEXT}

Recommendation: {shift_recommendation(hour)}"""
            subject = "Peak Time Energy Status"

        # NO EMAIL
        else:
            message = f"""NORMAL - no email sent
Status: {'ON-PEAK' if is_peak else 'OFF-PEAK'}
Voltage: {voltage} V
Power: {power} W"""

        # -------- SEND EMAIL --------
        if subject:
            sns.publish(
                TopicArn=TOPIC_ARN,
                Message=message,
                Subject=subject
            )

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
