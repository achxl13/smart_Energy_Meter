import json
import boto3
from boto3.dynamodb.conditions import Key
from datetime import datetime, timedelta

dynamodb = boto3.resource('dynamodb', region_name='ap-south-1')
table = dynamodb.Table('EnergyReadings')

def lambda_handler(event, context):
    try:
        response = table.query(
            KeyConditionExpression=Key('device_id').eq('esp32-001'),
            ScanIndexForward=False,
            Limit=20
        )

        items = response.get('Items', [])

        readings = []
        for item in items:
            readings.append({
                'timestamp':   item.get('timestamp', ''),
                'voltage':     float(item.get('voltage', 0)),
                'current':     float(item.get('current', 0)),
                'power':       float(item.get('power', 0)),
                'units':       float(item.get('units', 0)),
                'rate':        float(item.get('rate', 0)),
                'cost':        float(item.get('cost', 0)),
                'monthly_bill':float(item.get('monthly_bill', 0)),
                'is_peak':     item.get('is_peak', 'False') == 'True',
                'alert_type':  item.get('alert_type', 'normal'),
            })

        return {
            'statusCode': 200,
            'headers': {
                'Content-Type': 'application/json',
                'Access-Control-Allow-Origin': '*'
            },
            'body': json.dumps({
                'device_id': 'esp32-001',
                'count': len(readings),
                'readings': readings
            })
        }

    except Exception as e:
        print("ERROR:", str(e))
        return {
            'statusCode': 500,
            'headers': {'Access-Control-Allow-Origin': '*'},
            'body': json.dumps({'error': str(e)})
        }