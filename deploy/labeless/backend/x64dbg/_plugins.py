from . import api


def _plugin_logprintf(text='', *args):
    api._plugin_logprintf(text % args)


def _plugin_logputs(text=''):
    _plugin_logprintf('%s\n' % text)
