import gdb

class BasicStringPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        try:
            data = self.val['_data']
            size = int(self.val['_size'])
            capacity = int(self.val['_capacity'])
            if int(data) == 0:
                return '<null>'
            content = data.string(length=size)
            return f"'{content}' (size={size}, cap={capacity})"
        except gdb.error:
            return '<invalid string>'

    def children(self):
        try:
            data = self.val['_data']
            size = int(self.val['_size'])
            if int(data) == 0:
                return
            for i in range(size):
                yield (f'[{i}]', (data + i).dereference())
        except gdb.error:
            return

    def display_hint(self):
        return 'array'

def lookup_fennix_string(val):
    try:
        typename = str(val.type.strip_typedefs())
        fields = val.type.fields()
        field_names = [f.name for f in fields]
        if '_data' in field_names and '_size' in field_names and '_capacity' in field_names:
            return BasicStringPrinter(val)
    except:
        pass
    return None

gdb.pretty_printers.append(lookup_fennix_string)

def build_pretty_printers():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("fennix")
    pp.add_printer('std::string', '^std::string$', BasicStringPrinter)
    return pp

gdb.printing.register_pretty_printer(None, build_pretty_printers(), replace=True)
