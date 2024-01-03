#!/usr/bin/env python3

import re
import xml.etree.ElementTree as ET
from collections import OrderedDict

class DescriptionDef:
    def __init__(self, text):
        self.text = text
        self.code = False

class ArgumentDef:
    def __init__(self, name, type):
        self.name = name
        self.type = type
        self.default = None  # type: Optional[str]
        self.reference = None  # type: Optional[str]
        self.modificators = None  # type: Optional[str]

class MethodDef:
    def __init__(self, name):
        self.name = name
        self.returnType = None  # type: Optional[str]
        self.returnModificators = None  # type: Optional[list]
        self.reference = None  # type: Optional[str]
        self.description = None  # type: Optional[str]
        self.example = None  # type: Optional[str]
        self.arguments = None # type: Optional[OrderedDict[str, ArgumentDef]]
        self.tags = None
        self.modificators = None

class TypeDef:
    def __init__(self, name):
        self.name = name
        self.description = None # type: Optional[str]
        self.enumValues = None # ype: Optional[List]

class ClassDef:
    def __init__(self, name):
        self.name = name
        self.inherits = None # type: Optional[str]
        self.description = None # type: Optional[str]
        self.methods = OrderedDict() # type: OrderedDict[str, List[MethodDef]]
        self.types = list() # type: List[TypeDef]

def extractTypes(element, classDef):
    content = list(element)
    methods = element.findall(".//*[@class='fn']")
    
    for fn in methods:
        result = re.search(r'(.*) (?:(.*))?', "".join(fn.itertext()))
        if result is not None:
            #Retrieve name
            group2 = result.group(2)
            nameIndex = group2.rfind("::") + 2
            typedDef = TypeDef(group2[nameIndex:])

            #Retrieve description fields
            typedDef.description = list()
            
            index = content.index(fn) + 1
            while(len(element) > index and element[index].tag != "h3"): #Extract enums
                table = element[index].find(".//*[@class='valuelist']")
                if table is not None:
                    typedDef.enumValues = list()
                    for row in table:
                        rowList = list()
                        for column in row:
                            rowList.append("".join(column.itertext()))
                        typedDef.enumValues.append(rowList)
                    
                else:
                    desc = DescriptionDef("".join(element[index].itertext()))
                    typedDef.description.append(desc)
                    
                index += 1
                
            classDef.types.append(typedDef)

def extractMembers(element, classDef):
    content = list(element)
    methods = element.findall(".//*[@class='fn']")

    for fn in methods:
        result = re.search(r'(?:(\[.*\]) )?(.*?)\((.*)\)(?: (.*))?', "".join(fn.itertext()))
        if result is not None:
            #Retrieve name
            group2 = result.group(2)
            nameIndex = group2.rfind("::") + 2
            methodDef = MethodDef(group2[nameIndex:])
            methodDef.tags = result.group(1)
            methodDef.modificators = result.group(4)
            returnIndex = group2.rfind(" ")
            if returnIndex > -1:
                typeList = group2[:returnIndex].split()
                methodDef.returnType = typeList.pop()
                methodDef.returnModificators = typeList
                reference = ""
                for c in group2[returnIndex + 1:]:
                    if c == '&' or c == '*':
                        reference += c
                    else:
                        break
                methodDef.reference = reference
            #Retrieve arguments
            if len(result.group(3)) > 0:
                methodDef.arguments = OrderedDict()
                for a in result.group(3).split(", "):
                    l = a.split(" = ")
                    argument = l[0]
                    argIndex = argument.rfind(" ")
                    argumentName = argument[argIndex + 1:]
                    reference = ""
                    for c in argumentName:
                        if c == '&' or c == '*':
                            reference += c
                        else:
                            break
                    typeIndex = argument.rfind(" ", 0, argIndex) + 1
                    argumentDef = ArgumentDef(argumentName[len(reference):], argument[typeIndex:argIndex])
                    argumentDef.reference = reference
                    if typeIndex > 0:
                        argumentDef.modificators = argument[:typeIndex - 1]

                    if len(l) > 1:
                        argumentDef.default = l[1] # Default value
                    methodDef.arguments[argumentDef.name] = argumentDef
            #Retrieve description fields
            index = content.index(fn)

            methodDef.description = list()
            index += 1
            while(len(element) > index and element[index].tag != "h3"):
                desc = DescriptionDef("".join(element[index].itertext()))
                if(element[index].tag == "pre"):
                    desc.code = True
                methodDef.description.append(desc)
                index += 1

        if methodDef.name not in classDef.methods:
            classDef.methods[methodDef.name] = []

        classDef.methods[methodDef.name].append(methodDef)

def parseFile(file):
    print(file)
    
    f = open(file, "r")
    data = f.read()
    f.close()

    data = data.replace("<meta charset=\"utf-8\">", "<meta charset=\"utf-8\"/>")
    data = data.replace("&hellip;", "...")

    root = ET.fromstring(data)
    
    #tree = ET.parse(file)
    #root = tree.getroot()

    descr = root.find(".//*[@class='descr']")
    if descr is not None:
        title = root.find(".//*[@class='title']")
        classDef = ClassDef(title.text.replace(" Class", ""))

        classDef.description = list()

        index = 1
        while(len(descr) > index and descr[index].tag != "h3"):
            desc = DescriptionDef("".join(descr[index].itertext()))
            if(descr[index].tag == "pre"):
                desc.code = True
            classDef.description.append(desc)
            index += 1

        types = root.find(".//*[@class='types']")
        if types is not None:
            extractTypes(types, classDef)

        func = root.find(".//*[@class='func']")
        if func is not None:
            extractMembers(func, classDef)

        title = root.find(".//*[@class='alignedsummary']")
        isInherit = False
        for td in list(title.iter("td")):
            data = "".join(td.itertext()).strip()
            if isInherit == True:
                classDef.inherits = data
                isInherit = False
            if data == "Inherits:":
                isInherit = True
        return classDef
    return None

def parseModule(file, files):
    print("module " + file)

    f = open(file, "r")
    data = f.read()
    f.close()

    data = data.replace("<meta charset=\"utf-8\">", "<meta charset=\"utf-8\"/>")
    data = data.replace("&hellip;", "...")
    
    root = ET.fromstring(data)

    #tree = ET.parse(file)
    #root = tree.getroot()

    result = list()
    for td in root.findall(".//*[@class='tblName']"):
        name = "".join(td.itertext()).lower()
        if files.count(name) > 0:
            result.append(name)
            files.remove(name)

    return result
