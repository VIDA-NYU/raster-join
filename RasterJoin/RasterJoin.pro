TEMPLATE = subdirs

SUBDIRS = \
            api \
            example

# where to find the sub projects - give the folders
api.subdir = api
example.subdir = example

# what subproject depends on others
example.depends = api
