import requests

url = "http://127.0.0.1:8080"
params = {"text": "Tell me about Alpacas"}

response = requests.get(url, params=params)

print(response.text)