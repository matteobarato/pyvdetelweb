import pyvdetelweb.utils as utils
import hashlib

# 0000 DATA END '.'
# COMMAND PATH - SYNTAX - ARGUMENTS

class Vde2Mgmt:
    CMD_HISTORY = []  # commands history
    HELP_COMMANDS = []    # dictionary of commands, arguments, and help lines
    HISTORY = []
    HISTORY_HTML = []
    START_OPTIONAL_ARGUMENT = '[' # symbol for marker start of optional argument
    END_OPTIONAL_ARGUMENT = ']' # symbol for marker end of optional argument

    def __init__(self, socket, buffer_size, system_info):
        self.socket = socket # vde management socket
        self.BUFFER_SIZE = buffer_size
        self.SYS_INFO = system_info # configuration and vde options
        self.socket.recv(buffer_size)

    def is_required_arg(self, arg):
        ''' Check if argument is required '''
        if len(arg)>2 and arg[0]==self.START_OPTIONAL_ARGUMENT and arg[-1]==self.END_OPTIONAL_ARGUMENT:
            return False
        return True

    def send(self, cmd):
        mgmt_response = ''
        cmd = cmd.strip()
        try:
            self.socket.sendall(cmd)
            mgmt_response = utils.get_response(self.socket, self.BUFFER_SIZE)
        finally:
            if mgmt_response != '':
                self.add_history(cmd=cmd, res=mgmt_response['text'])
            return mgmt_response

    def showinfo(self):
        ''' send showinfo to mgmt socket and retrive the respone '''
        mgmt_response = ''
        try:
            self.socket.sendall('showinfo')
            mgmt_response = utils.get_response(self.socket, self.BUFFER_SIZE)
        finally:
            return mgmt_response


    def list_commands(self):
        ''' parse help command's response from mngm socket '''
        help_line = []
        self.HELP_COMMANDS= []
        column_index = []

        try:
            self.socket.sendall('help')
            start = False
            mgmt_response = utils.get_response(self.socket, self.BUFFER_SIZE)
            help_list = mgmt_response['text'].split('\n')

            for i in help_list:
                if start and len(i)>0:
                    if i.find('====') < 0:  # not find
                        help_line.append(i)

                if not start:
                    command_path_index = i.find('COMMAND PATH')
                    syntax_index = i.find('SYNTAX')
                    help_index = i.find('HELP')

                    if command_path_index > -1:
                        column_index.append(command_path_index)
                    if syntax_index > -1:
                        column_index.append(syntax_index)
                    if help_index > -1:
                        column_index.append(help_index)
                    if i.find('------------') > -1:
                        start = True

        finally:
            count_row = 0
            for i in help_line:
                command = {'__index': count_row,
                           'cmd': i[column_index[0]:column_index[1]].strip(),
                           'args': (i[column_index[1]:column_index[2]].strip()).split(),
                           'text_args': i[column_index[1]:column_index[2]].strip(),
                           'help': i[column_index[2]:].strip(),
                           'text': i
                           }
                count_row += 1
                self.HELP_COMMANDS.append(command)
            return self.HELP_COMMANDS

    def commands_tree(self):
        ''' parse help response to a nested hierarchy command tree '''
        if not self.HELP_COMMANDS: self.list_commands() #fill HELP_COMMANDS
        cmds_tree = {}
        utils.makeCommandsTree(cmds_tree, self.HELP_COMMANDS)
        return cmds_tree

    def get_history(self):
        ''' return command line's style history '''
        return self.HISTORY

    def get_HTMLhistory(self):
        ''' return HTML's style history '''
        return self.HISTORY_HTML

    def add_history(self, *args, **kwargs):
        ''' add command to `HISTORY` and `HISTORY_HTML` properly parsed'''
        cmd = kwargs.get('cmd', None)
        res = kwargs.get('res', None)
        if cmd:
            self.CMD_HISTORY.append('cmd')
            self.HISTORY_HTML.append('<tprfx>'+self.SYS_INFO['terminal_prefix']+'</tprfx>'+'<tcmd>'+cmd+'</tcmd>\n')
            self.HISTORY.append(self.SYS_INFO['terminal_prefix']+cmd)
        if res:
            self.HISTORY_HTML.append(res)
            self.HISTORY.append(res)

        return cmd + '\n' + res

    def authenticate(self, username, password):
        ''' check username and password for authentication '''
        if (self.SYS_INFO['config']['username'] == username and hashlib.sha1(password).hexdigest() == self.SYS_INFO['config']['password']):
                return True
        return False
