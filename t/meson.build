tests = [
    { 'name': 'allged.ged' },
    { 'name': 'ansel.ged' },
    { 'name': 'anselutf8.ged' },
    { 'name': 'compat-easytree-2.ged', 'compat': true },
    { 'name': 'compat-easytree.ged', 'compat': true },
    { 'name': 'compat-famorig.ged', 'compat': true },
    { 'name': 'compat-ftree.ged', 'compat': true },
    { 'name': 'compat-lifelines.ged', 'compat': true },
    { 'name': 'compat-paf2.ged', 'compat': true },
    { 'name': 'compat-paf4.ged', 'compat': true },
    { 'name': 'compat-paf5.ged', 'compat': true },
    { 'name': 'dates.ged' },
    { 'name': 'encoding_error.ged', 'fail': true },
    { 'name': 'line_nr.ged', 'fail': true },
    { 'name': 'minimal.ged' },
    { 'name': 'uhlbomcl.ged' },
    { 'name': 'uhlcl.ged' },
    { 'name': 'ulhbomcl.ged' },
    { 'name': 'ulhc.ged' },
    { 'name': 'ulhcl.ged' },
    { 'name': 'ulhl.ged' },
    { 'name': 'ulhlc.ged' },
    { 'name': 'unknown.ged', 'fail': true }
]

foreach test: tests
    name = 'validate @0@'.format(test['name'])

    file = join_paths(meson.current_source_dir(), 'input', test['name'])
    compat = test.get('compat', false)
    fail = test.get('fail', false)

    test(name, check_exe,
        args: compat ? ['-c', file] : file,
        workdir: meson.global_build_root(),
        should_fail: fail
    )
endforeach
