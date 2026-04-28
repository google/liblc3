py = import('python').find_installation()

pybind11_dep = dependency('pybind11')

py.extension_module('_lc3',
    'lc3_pybind.cpp',
    dependencies: [pybind11_dep, liblc3_dep],
    install: true,
    subdir: ''
)

py.install_sources('lc3.py', 'py.typed', '_lc3.pyi')
