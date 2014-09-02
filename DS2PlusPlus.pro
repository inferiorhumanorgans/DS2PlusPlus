TEMPLATE = subdirs

SUBDIRS += \
    libds2 \
    ds2-dump \
    tests \
    jsoncpp

# https://vilimpoc.org/blog/2014/02/21/qmake-subdirs-project-automatic-dependencies/
jsoncpp.subdir = jsoncpp
libds2.depends = jsoncpp
ds2-dump.depends = libds2
ds2-test.depends = libds2
