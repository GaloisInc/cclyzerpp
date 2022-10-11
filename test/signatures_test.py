import gzip

def test_pointer_analysis_string_signatures(run):
    # This is run at -O0 because the templated functions are inlined otherwise .
    no_signatures_dir = run("strings.cpp", additional_cflags=("-O0",))
    signatures_dir = run(
        "strings.cpp",
        additional_cflags=("-O0",),
        signatures={
            # std::char_traits<char>::compare(char const*, char const*, unsigned long)
            "std::char_traits<.+>::compare": [{"pts_none": []}]
        },
    )
    with gzip.open((no_signatures_dir / "var_points_to_sizes").with_suffix(".csv.gz"), "rt") as f1:
        with gzip.open((signatures_dir / "var_points_to_sizes").with_suffix(".csv.gz"), "rt") as f2:
            # The definitions of functions with signatures should be excluded
            # by the fact generator, resulting in fewer variables.
            assert len(f1.readlines()) > len(f2.readlines())
