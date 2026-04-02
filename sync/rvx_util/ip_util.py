import requests

def check_internet_connection(url='http://www.google.com', timeout=5):
  result = False
  try:
    response = requests.get(url, timeout=timeout)
    if response.status_code == 200:
      result = True
  except requests.ConnectionError:
    pass
  return result

def get_external_ip():
  try:
    response = requests.get('https://api64.ipify.org?format=json')
    external_ip = response.json()['ip']
  except requests.RequestException as e:
    external_ip = None
    print(f"Error: {e}")
  return external_ip