from xml.dom import minidom
import urllib
import re
from datetime import datetime, timedelta
import string


class blogEntry:
    pass

def getBlog(hours=72):
    xmldoc = minidom.parse(urllib.urlopen('https://grid-wiki.physik.rwth-aachen.de/blog?format=rss'))
    itemlist = xmldoc.getElementsByTagName('item')
    blog = []
    for item in itemlist:
        entry = blogEntry()
        entry.title = item.getElementsByTagName("title")[0].childNodes[0].data
        entry.creator = item.getElementsByTagName("dc:creator")[0].childNodes[0].data
        entry.pubDate = item.getElementsByTagName("pubDate")[0].childNodes[0].data
        entry.link = item.getElementsByTagName("link")[0].childNodes[0].data
        entry.description =  (re.sub('<[^<]+?>', '',   filter(lambda x: x in string.printable,item.getElementsByTagName("description")[0].childNodes[0].data)))
        entry.date = datetime.strptime(entry.pubDate, '%a, %d %b %Y %H:%M:%S %Z')
        if datetime.now() - entry.date < timedelta(hours=hours):
            blog.append(entry)
    return blog
