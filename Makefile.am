MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src include

@DX_RULES@

README: README.md
	pandoc --from markdown_github --to html --standalone $< | html2text -style pretty -nobs -width 120 | sed -e 's:&gt;:>:g' > $@

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
	@rm -rf doxygen-doc

check:
	@make gcov

else

clean-local:
	@rm -rf doxygen-doc

endif
