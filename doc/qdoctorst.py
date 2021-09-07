#!/usr/bin/env python3

import os
import htmlparser

from string import Template
from texttable import Texttable

def composeTable(classDef, static):
    table = Texttable(512)
    table.set_cols_align(["r", "l"])

    for methods in classDef.methods.values():
        for method in methods:
            if static == True and (method.tags is None or method.tags != "[static]"):
                continue
            if static == False and (method.tags is not None and method.tags == "[static]"):
                continue
            returnMod = ""
            if method.returnModificators is not None:
                returnMod = "{} ".format(method.returnModificators)
            type = ""
            reference = ""
            if method.reference != "":
                reference = " {}".format(method.reference)
            if method.returnType is not None:
                type = "{0}:ref:`{1}<api_{1}>`{2}".format(returnMod, method.returnType, reference)

            methodMod = ""
            if method.modificators is not None:
                methodMod = " {}".format(method.modificators)

            body = ""
            args = list("")
            if method.arguments is not None:
                for argument in method.arguments.values():
                    argMod = ""
                    if argument.modificators is not None:
                        argMod = "{} ".format(argument.modificators)

                    default = ""
                    if argument.default is not None:
                        default = " = {}".format(argument.default)

                    args.append("{0}{1} {2} {3}{4}".format(argMod, argument.type, argument.reference, argument.name, default))
            body = ":ref:`{0}<api_{3}_{0}>` ({1}){2}".format(method.name, ", ".join(args), methodMod, classDef.name)

            table.add_row([type, body])

    return table.draw()

def composeMethods(classDef):
    result = ""
    if classDef.methods is not None:
        for methods in classDef.methods.values():
            for method in methods:
                result += ".. _api_{0}_{1}:\n\n".format(classDef.name, method.name)

                if method.returnType is not None:
                    result += ":ref:`{0}<api_{0}>` {1} ".format(method.returnType, method.reference)

                methodMod = ""
                if method.modificators is not None:
                    methodMod = " {}".format(method.modificators)

                args = list("")
                if method.arguments is not None:
                    for argument in method.arguments.values():
                        default = ""
                        if argument.default is not None:
                            default = " = {}".format(argument.default)
                        args.append(":ref:`{0}<api_{0}>` {1} *{2}*{3}".format(argument.type, argument.reference, argument.name, default))

                result += "**{0}::{1}** ({2}){3}\n\n".format(classDef.name, method.name, ", ".join(args), methodMod)
                for description in method.description:
                    text = description.text
                    if description.code == True:
                        result += "::\n\n"
                        text = "    " + text.replace("\n", "\n    ")
                    else:
                        if text.find("See also ") == 0:
                            clean = text.replace("See also ", "")
                            text = "**See also** " + clean

                        if method.arguments is not None:
                            for argument in method.arguments.values():
                                text = text.replace(" {}".format(argument.name), " *{}*".format(argument.name))

                        text = text.replace("Note: ", "**Note:** ")

                    result += text + "\n\n"
                result += "----\n\n"
    return result

def composeEnums(classDef):
    result = ""
    if len(classDef.types) > 0:
        result +=  ".. _api_" + classDef.name + "_enums:\nPublic Enums\n--------------\n\n"
    for type in classDef.types:
        result += ".. _api_{0}_{1}:\n".format(classDef.name, type.name)
        result += "**enum {0}::{1}**\n\n".format(classDef.name, type.name)
        
        for description in type.description:
            result += description.text + "\n\n"
        
        table = Texttable(512)
        table.set_cols_align(["r", "l", "l"])
        
        if type.enumValues is not None:
            for row in type.enumValues:
                table.add_row([row[0], row[1], row[2]])
            
            result += table.draw() + "\n\n"
    return result;

def main():
    try:
        os.mkdir("reference")
    except OSError as exc:
        pass

    f = open("page.rst", "r")
    s = Template(f.read())
    f.close()

    files = list()

    fileList = (f for f in os.listdir("html") if f.endswith(".html"))
    for curFile in fileList:
        filename = os.path.splitext(curFile)[0]
        if curFile.find("-module") == -1 and curFile.find("thunder-engine"):
            classDef = htmlparser.parseFile("html/" + curFile)
            if classDef is not None:
                files.append(filename)
                f = open("reference/" + filename + ".rst", "w")
                d = dict(className=classDef.name)

                description = ""
                for desc in classDef.description:
                    text = desc.text
                    if desc.code == True:
                        description += "::\n\n"
                        text = "    " + text.replace("\n", "\n    ")
                        
                    description += text + "\n\n"

                d["description"] = description

                inherits = classDef.inherits
                if classDef.inherits is not None:
                    inherits = ":ref:`{0}<api_{0}>`".format(inherits)

                d["inheritance"] = inherits
                d["public"] = composeTable(classDef, False)
                d["static"] = composeTable(classDef, True)
                d["methods"] = composeMethods(classDef)
                d["enums"] = composeEnums(classDef)
                f.write(s.substitute(d))
                f.close()

    f = open("index.rst", "r")
    s = Template(f.read())
    f.close()

    modules = list()

    fileList = (f for f in os.listdir("html") if f.endswith(".html"))
    for curFile in fileList:
        filename = os.path.splitext(curFile)[0]
        if curFile.find("-module") != -1:
            f = open("reference/" + filename + ".rst", "w")
            d = dict(index="\n   ".join(htmlparser.parseModule("html/" + curFile, files)))
            d["module"] = filename.replace("-", "_")
            d["header"] = filename.replace("-", " ").title()
            f.write(s.substitute(d))
            f.close()

            modules.append(filename)


    modules.extend(files)

    f = open("reference/index.rst", "w")
    d = dict(index="\n   ".join(modules))
    d["module"] = "index"
    d["header"] = "API Reference"
    f.write(s.substitute(d))
    f.close()

if __name__ == '__main__':
    main()
