plugin_sources = [
  'config_gui.c',
  'config_gui.h',
  'devperf.c',
  'devperf.h',
  'main.c',
  xfce_revision_h,
]

extra_link_args = []
if host_machine.system() == 'darwin'
  extra_link_args += [
    '-framework', 'CoreFoundation',
    '-framework', 'IOKit',
  ]
endif

plugin_install_subdir = 'xfce4' / 'panel' / 'plugins'

plugin_lib = shared_module(
  'diskperf',
  plugin_sources,
  gnu_symbol_visibility: 'hidden',
  c_args: [
    '-DG_LOG_DOMAIN="@0@"'.format('xfce4-diskperf-plugin'),
  ],
  include_directories: [
    include_directories('..'),
  ],
  dependencies: [
    glib,
    gtk,
    libxfce4panel,
    libxfce4ui,
    libxfce4util,
    libm,
  ],
  link_args: extra_link_args,
  install: true,
  install_dir: get_option('prefix') / get_option('libdir') / plugin_install_subdir,
)

i18n.merge_file(
  input: 'diskperf.desktop.in',
  output: 'diskperf.desktop',
  po_dir: '..' / 'po',
  type: 'desktop',
  install: true,
  install_dir: get_option('prefix') / get_option('datadir') / plugin_install_subdir,
)
