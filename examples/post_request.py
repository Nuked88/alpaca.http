import requests

url = "http://127.0.0.1:8080"
text = "Tell me about Alpacas"

response = requests.post(url, data=text.encode("utf-8"))

print(response.text)
