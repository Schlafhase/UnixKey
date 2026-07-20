project(
  'UnixKey',
  'c',
  meson_version : '>= 1.3.0',
  version : '0.1',
  default_options : ['warning_level=3'],
)

dependencies = [
  dependency('xkbcommon'),
  dependency('libevdev')
]

sources = [
  'main.c',
  'input/event_handler.c',
]

exe = executable(
  'UnixKey',
  sources,
  dependencies : dependencies,
  install : true,
)

test('basic', exe)
