# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    bld(rule='echo this is a test', always=True)
    module = bld.create_ns3_module('saf', ['core', 'stats', 'aodv', 'dsdv', 'internet', 'mobility', 'wifi'])
    module.source = [
        'model/saf.cc',
        'model/data.cc',
        'model/util.cc',
        'model/logging.cc',
        'model/message.cc',
        'helper/saf-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('saf')
    module_test.source = [
        'test/saf-test-suite.cc',
        ]
    # Tests encapsulating example programs should be listed here
    if (bld.env['ENABLE_EXAMPLES']):
        module_test.source.extend([
        #    'test/saf-examples-test-suite.cc',
             ])

    headers = bld(features='ns3header')
    headers.module = 'saf'
    headers.source = [
        'model/saf.h',
        'model/data.h',
        'model/util.h',
        'helper/saf-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()
