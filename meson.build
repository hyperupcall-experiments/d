project('d', 'c',
  version : '0.1.0',
  license : 'MPL-2.0')
add_global_arguments(language : 'c')

my_inc = include_directories('include')
my_inc2 = include_directories('src')
install_headers('./src/util.h')

libsimple_proj = subproject('inih')
inih_dep = libsimple_proj.get_variable('inih_dep')
inih_inc = libsimple_proj.get_variable('inc_inih')


# subdir('include')
# subdir('src')
my_lib = static_library('mylib',
	'src/util.c',
	include_directories : [my_inc2, inih_inc])

#subdir('test')
my_dep = declare_dependency(link_with : my_lib,
  include_directories : my_inc)
#pkg_mod = import('pkgconfig')
#pkg_mod.generate(libraries : foolib,
#                 version : '1.0',
#                 name : 'libfoobar',
#                 filebase : 'foobar',
#                 description : 'A Library to barnicate your foos.')
build_args = [
]
project_source_files = [
  'bin/d.c'
]
project_dependencies = [
	my_dep
]
build_args += [
  '-DPROJECT_NAME=' + meson.project_name(),
  '-DPROJECT_VERSION=' + meson.project_version(),
]



project_dependencies += inih_dep
project_target = executable(
  meson.project_name(),
  project_source_files,
  dependencies: project_dependencies,
  install : false,
	include_directories : my_inc2,
  c_args : build_args,
)

test('basic', project_target)
