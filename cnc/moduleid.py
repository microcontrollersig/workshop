import uuid
import re
import xml.dom.minidom

def get_dom(filename):
    try:
        dom = xml.dom.minidom.parse(filename)
    except xml.parsers.expat.ExpatError as err:
        print(str(err), filename)
        sys.exit(-1)

    return dom.documentElement

def set_module_id(dom, name):
    newModuleID = '%.8s%s' % (re.sub(r'\s+|_', '', name), uuid.uuid4().hex)
    dom.setAttribute("moduleId", newModuleID)
    return newModuleID

if __name__ == '__main__':
    dom = get_dom("sparkfun-connectors-db25-d.fzp")
    module_id = set_module_id(dom, "moo-")
    print(module_id)