MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src doc include

dist_data_DATA = README.md

README: README.md
	pandoc --from markdown_github --to html --standalone $< | html2text -style pretty -nobs | sed -e 's:&gt;:>:g' > $@

@DX_RULES@

if ENABLE_GCOV

gcov-reset:
	@rm -fr coverage
	@find . -name "*.gcda" -exec rm {} \;
	@@LCOV@ --directory . --zerocounters

gcov-report:
	@mkdir -p coverage
	@@LCOV@ --compat-libtool --directory . --capture --output-file coverage/libemv-check
	@@GENHTML@ -o coverage/ coverage/libemv-check

gcov:
	@make gcov-report

clean-local:
	@make gcov-reset
	@find . -name "*.gcno" -exec rm {} \;

check:
	@make gcov

endif
