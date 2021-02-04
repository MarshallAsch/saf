def build(bld):
    obj = bld.create_ns3_program('', ['stats', 'dsdv', 'internet', 'mobility', 'wifi'])
    obj.source = ['data.cc', 'main.cc', 'message.cc', 'saf-application.cc', 'saf-helper.cc']
