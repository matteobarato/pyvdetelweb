import os
def get_response(socket, buffer_size):
    buffer = socket.recv(buffer_size)
    buffering = True
    end_char = False
    is_end = False

    help_list = []
    text = ''
    status = ''
    raw = ''
    text_status = ''
    while buffering:
        if "\n" in buffer:
            (line, buffer) = buffer.split("\n", 1)
            raw += line +'\n'

            if (len(line) == 0 and not is_end):
                text += '\n'

            elif (line[0]=='0' and line.find('0000 DATA END WITH') > -1):
                end_char = line.split("'")[1]; #get string into ' ' of 0000 DATA END WTIH '*' line

            elif (line[0]>'0' and line<='9'
                and line[1]>='0' and line[1]<='9'
                and line[2]>='0' and line[2]<='9'
                and line[3]>='0' and line[3]<='9'): #find end string 1000 Success or similar
                status = line[0:4]
                text_status = line[4:]
                buffering = False

            elif line == end_char:
                is_end = True

            elif not is_end:
                text += line+'\n'
        else:
            more = socket.recv(buffer_size)
            if not more:
                buffering = False
            else:
                buffer += more
    return {'raw':raw, 'text': text, 'status': status, 'text_status':text_status}

def makeCommandsTree(cmds_tree, cmds_dict):
    for i in cmds_dict:
        cmds = i['cmd'].split('/')
        tree_level = cmds_tree
        for node in cmds:
            if node not in tree_level:
                tree_level[node] = {}
            if node == cmds[-1]:
                tree_level[node] = i

            tree_level = tree_level[node]
    return cmds_tree

def parseConfig(fconfig_path):
    config = {
    'username':'admin',
    'ip': '10.40.0.55',
    'netmask': '255.255.255.0',
    'stack': 'picotcp'
    } #default configuration
    f = open(fconfig_path, 'r')
    count_line = 0
    try:
        fconfig_text = f.readlines()
        for line in fconfig_text:
            line = line.strip()
            if not (len(line)==0 or line[0]=='#'):
                chunck = line.split('=', 1)
                if (len(chunck)==2):
                    config[chunck[0].strip()] = chunck[1].strip()
                else:
                    log.error('ERROR: Error in cofiguration file at line '+str(count_line))
        count_line = count_line+1

    except IOError:
        log.error('ERROR: Configuration file not found!')
    finally:
        print config ##########################
        f.close()
        return config
