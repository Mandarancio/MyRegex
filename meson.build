project('RegEx', 'c',
  version : '0.1',
  license : 'GPL3'
)


PROJECT_NAME = 'regex'

sources = [
    'main.c'
]

lib_src = [
    'regex.c'
]


executable(
    PROJECT_NAME,
    sources: [lib_src, sources],
    install: false
)

shared_library(
    PROJECT_NAME,
    sources: [lib_src],
    install: true
)
