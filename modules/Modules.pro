TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = \
    renders/rendergl/RenderGL.pro

prefix = ../../WorldBuilder/Modules
target.path = $$prefix

INSTALLS += target
