py = import('python').find_installation()

py.extension_module('_lc3',
    'lc3_native.c',
    dependencies: [liblc3_dep],
    install: true
)

py.install_sources('lc3.py', 'py.typed', '_lc3.pyi')
