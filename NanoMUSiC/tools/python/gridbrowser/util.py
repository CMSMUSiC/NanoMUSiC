import re


def format_file_size(size):
    suffixes = ['bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB']

    index = 0
    while size > 1000.0:
        size /= 1000.0
        index += 1

    return '{:.4g} {}'.format(size, suffixes[index])

_remove_alphanumeric = re.compile('[\W_]+')
def natsort_key(text):
    text = _remove_alphanumeric.sub('', text)
    text = text.lower()
    return text
