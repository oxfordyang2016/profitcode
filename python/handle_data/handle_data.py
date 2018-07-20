import urllib
import gzip

def get_data(contract_name, date, prefix='CTP', saved_filename=''):
    url = 'https://s3.amazonaws.com/wanlitech-data/polled/1s/' + date + '/' + prefix + '/' + contract_name + '.top.gz'
    print(url)
    zfile_name = ''
    if saved_filename == '':
        zfile_name = zfile_name + contract_name+'.gz'
    else:
        zfile_name = zfile_name + saved_filename +'.gz'
    print(zfile_name)
    urllib.urlretrieve(url, zfile_name)
    content = gzip.GzipFile(zfile_name).read()
    return content

def AddIndex(content, index):
    lines = content.split('\n') # split file content by line
    content_dict = {} # target to form this dict, use index can get a vector
    for i in index:
        content_dict[i] = [] # init this vector as null
    content_size = len(lines[0].split(' ')) # size of content

    if len(index) != content_size:
        print('index\' size is incorrect!')
        return content_dict

    for i in range(len(lines)-1):
        line_content = lines[i].split(' ')
        if len(line_content) != content_size: # exception: size incorrect
            print("error line in line", i, ", this line's size is", len(line_content), ", target size is ", content_size)
            continue
        for j in range(len(index)):
            content_dict[index[j]].append(line_content[j])

    return content_dict

# test_dict = content_dict(zip(b['time'], content_dict['last_price'])) # contract a dict indexing time
# sorted(test_dict.iteritems(), key=lambda d:d[0], reverse=True)[0] # sort in the dict using d[0]->time



def GenGridData(content):
    line_content = content.split('\n')

#c = get_data('AUAZ7', '20171114')
