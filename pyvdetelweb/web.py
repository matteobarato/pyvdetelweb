import sys
import os
import pycotcp.socket as pycosocket

# override 'classic' socket library with pycotcp.scoket library
sys.modules['socket'] = pycosocket

from flask import Flask, render_template, request, jsonify, Response
from flask.views import View
from functools import wraps

from werkzeug.serving import make_ssl_devcert

tmpl_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'assets/templates')
stct_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'assets/static')

app = Flask(__name__, template_folder=tmpl_dir, static_folder=stct_dir)

mgmt = {}

def requires_auth(f):
    # check for session authentication HTTP Basic
    @wraps(f)
    def decorated(*args, **kwargs):
        global mgmt
        auth = request.authorization
        if not auth or not mgmt.authenticate(auth.username, auth.password):
            return Response(
            'Could not verify your access level for that URL.\n'
            'You have to login with proper credentials', 401,
            {'WWW-Authenticate': 'Basic realm="Login Required"'})
        return f(*args, **kwargs)
    return decorated

def create_commands_urls(mgmt, commands_dict_tree, path):
    ''' dinaically create paths url of commands respect RESTful approach building it from nested tree of commands '''
    for k, v in commands_dict_tree.items():
        if isinstance(v, dict):
            new_path = path+'/'+k
            if '__index' not in v:
                create_commands_urls(mgmt, v, path+'/'+k)
            else:
                # create REST API route
                app.add_url_rule('/api'+new_path, new_path, send_command(k, v, mgmt), methods=['POST'])
            # create html pages form
            app.add_url_rule(new_path, 'menu_'+new_path, cmds_form(k, v), methods=['GET'])
            app.add_url_rule('/api'+new_path, 'api_'+new_path, cmds_api(k, v), methods=['GET'])


def send_command(cmd_string, cmd_dict, mgmt):
    # assign route for recive command POST request
    @requires_auth
    def f():
        mgmt_response = ''
        if request.method == 'POST':
                send_command= cmd_dict['cmd']
                cmd_is_correct = True
                for i in cmd_dict['args']:
                    if request.form[i] :
                        send_command += ' ' + request.form[i]
                    elif mgmt.is_required_arg(i) :
                        cmd_is_correct = False
                        mgmt_response = {'text':'Invalid: argument '+i+' is required!'}

                if cmd_is_correct:
                    mgmt_response = mgmt.send(send_command)

        json_response = {
            'command':cmd_dict['cmd'],
            'arguments':request.form,
            'response': mgmt_response,
            'terminal_prefix': mgmt.SYS_INFO['terminal_prefix']
        }

        return jsonify(json_response)
    return f

def cmds_form(cmd_string, cmd_dict):
    # render commands page templates
    @requires_auth
    def f():
        if '__index' in cmd_dict:
            return render_template('cmd_form.html', cmd_path_menu = cmd_string, cmd=cmd_dict)
        else:
            return render_template('cmds_form.html', cmd_path_menu = cmd_string, cmds=cmd_dict)
    return f

def cmds_api(cmd_string, cmd_dict):
    # render commands page templates
    @requires_auth
    def f():
        json_response = {
            'resource':cmd_string,
            'commands':cmd_dict,
            'terminal_prefix': mgmt.SYS_INFO['terminal_prefix']
        }
        return jsonify(json_response)
    return f


def web(mgmt_object):

    global mgmt
    mgmt = mgmt_object

    sys_info = mgmt.SYS_INFO

    help_list = mgmt.list_commands()
    help_tree = mgmt.commands_tree()

    @app.context_processor
    def inject_terminal_prefix():
        # insert terminal_prefix variable in all template as default
        return dict(terminal_prefix=sys_info['terminal_prefix'])


    # define web server routes
    @app.route("/")
    @requires_auth
    def index():
        res = mgmt.showinfo()
        return render_template('index.html', terminal_prefix=sys_info['terminal_prefix'], showinfo=res['text'])
    @app.route("/api")
    @requires_auth
    def api():
        res = mgmt.showinfo()
        json_response = {
            'resource':'/',
            'commands':help_tree,
            'terminal_prefix': mgmt.SYS_INFO['terminal_prefix'],
            'showinfo':res
        }
        return jsonify(json_response)

    @app.route("/__home")
    @requires_auth
    def home():
        cmds = []
        for line in help_tree:
            cmds.append(line)
        return render_template('home.html', cmds=cmds)

    @app.route("/__history")
    @requires_auth
    def history():
        return render_template('history.html', history=mgmt.get_HTMLhistory())

    @app.route("/__help")
    @requires_auth
    def help():
        return render_template('help.html', cmds=help_list)

    create_commands_urls(mgmt, help_tree, '')

    #if __name__ == "__main__":
    print 'Server run'
    app.run(host=mgmt.SYS_INFO['config']['ip'], port=80)
