from requests_html import HTMLSession
import requests
import datetime

session = HTMLSession()

n = 1

while n <= 365:
    date = datetime.datetime(2018, 1, 1) + datetime.timedelta(n - 1)
    url = 'https://www.gocomics.com/calvinandhobbes/2018/{}/{}'.format(date.month, date.day)
    r = session.get(url)
    comic = r.html.find('div.comic', first=True)
    name = comic.attrs['data-url']
    print('{}'.format(name))
    x = 'comics/{}.jpg'.format(name.replace('https://', '').replace('/', '-'))
    with open(x, "wb") as f:
        c = requests.get(comic.attrs['data-image'])
        f.write(c.content)
    n += 1